#include "vm.h"
#include "value.h"
#include "memory.h"
#include "compiler.h"
#include "debug.h"
#include "core.h"
#include "object.h"
#include "vessel.h"
#if OPT_RANDOM
#include "opt_random.h"
#endif

#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#include <stdio.h>

VM vm;

static Value clock_native(int arg_count, Value* args)
{
	return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void reset_stack()
{
	vm.stack_top = vm.stack;
	vm.frame_count = 0;
	vm.open_upvalues = NULL;
}

static void runtime_error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	for (int i = vm.frame_count - 1; i >= 0; i--)
	{
		CallFrame* frame = &vm.frames[i];

		ObjFunction* function = frame->closure->function;
		size_t instruction = frame->ip - function->chunk.code - 1;
		fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
		if (function->name == NULL) {
			fprintf(stderr, "script\n");
		} else {
			fprintf(stderr, "%s()\n", function->name->chars);
		}
	}

	reset_stack();
}

static void define_native(const char* name, NativeFn function)
{
	push(OBJ_VAL(copy_string(name, (int)strlen(name))));
	push(OBJ_VAL(new_native(function)));
	table_set(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
	pop();
	pop();
}

void init_configuration(Configuration* config)
{
	config->load_module_fn = NULL;
}

void init_vm()
{
	reset_stack();

	vm.objects = NULL;

	vm.bytes_allocated = 0;
	vm.next_gc = 1024 * 1024;

	vm.gray_count = 0;
	vm.gray_capacity = 0;
	vm.gray_stack = NULL;

	init_table(&vm.globals);
	init_table(&vm.strings);

	vm.init_string = NULL;
	vm.init_string = copy_string("init", 4);

	init_table(&vm.modules);

	vm.api_stack = NULL;

	init_configuration(&vm.config);

	init_value_array(&vm.method_names);

	initialize_core();

	define_native("clock", clock_native);
}

void free_vm()
{
	free_table(&vm.globals);
	free_table(&vm.strings);
	vm.init_string = NULL;
	free_table(&vm.modules);
	free_value_array(&vm.method_names);
	free_objects();
}

void push(Value value)
{
	*vm.stack_top = value;
	vm.stack_top++;

#ifdef DEBUG_PRINT_STACK
	const int stack_sz = vm.stack_top - &vm.stack[0];
	switch (value.type)
	{
	case VAL_BOOL:
		printf("push bool, %d\n", stack_sz);
		break;
	case VAL_NIL:
		printf("push nil, %d\n", stack_sz);
		break;
	case VAL_NUMBER:
		printf("push num %g, %d\n", AS_NUMBER(value), stack_sz);
		break;
	case VAL_OBJ:
	{
		const char* names[OBJ_MAP + 1] = {
			"bound_method",
			"class",
			"closure",
			"method",
			"function",
			"instance",
			"native",
			"string",
			"upvalue",
			"module",
			"list",
			"map",
		};
		printf("push obj %s, %d\n", names[OBJ_TYPE(value)], stack_sz);
	}
		break;
	}
#endif // DEBUG_PRINT_STACK
}

Value pop()
{
	vm.stack_top--;
#ifdef DEBUG_PRINT_STACK
	printf("pop %d\n", vm.stack_top - &vm.stack[0]);
#endif // DEBUG_PRINT_STACK
	return *vm.stack_top;
}

static Value peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

static bool call(ObjClosure* closure, int arg_count)
{
	if (arg_count != closure->function->arity) {
		runtime_error("Expected %d arguments but got %d.", closure->function->arity, arg_count);
		return false;
	}

	if (vm.frame_count == FRAMES_MAX) {
		runtime_error("Stack overflow.");
		return false;
	}

	CallFrame* frame = &vm.frames[vm.frame_count++];

	frame->closure = closure;
	frame->ip = closure->function->chunk.code;

	frame->slots = vm.stack_top - arg_count - 1;
	return true;
}

static bool call_value(Value callee, int arg_count)
{
	if (IS_OBJ(callee))
	{
		switch (OBJ_TYPE(callee))
		{
		case OBJ_BOUND_METHOD:
		{
			ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
			vm.stack_top[-arg_count - 1] = bound->receiver;
			return call(bound->method, arg_count);
		}

		case OBJ_CLASS:
		{
			ObjClass* klass = AS_CLASS(callee);
			vm.stack_top[-arg_count - 1] = OBJ_VAL(new_instance(klass));
			Value initializer;
			if (table_get(&klass->methods, vm.init_string, &initializer)) {
				return call(AS_CLOSURE(initializer), arg_count);
			} else if (arg_count != 0) {
				runtime_error("Expected 0 arguments but got %d.", arg_count);
				return false;
			}
			return true;
		}

		case OBJ_METHOD:
		{
			ObjMethod* method = AS_METHOD(callee);
			Value* args = vm.stack_top - arg_count - 1;
			switch (method->type)
			{
			case METHOD_PRIMITIVE:
				if (method->as.primitive(args)) {
					vm.stack_top -= arg_count;
					return true;
				} else {
					runtime_error("Run primitive fail.");
					return false;
				}
				break;
			case METHOD_FUNCTION_CALL:
				break;
			case METHOD_FOREIGN:
				break;
			case METHOD_BLOCK:
				break;
			case METHOD_NONE:
				break;
			default:
				ASSERT(false, "Unknown method type.");
			}
			return false;
		}

		case OBJ_CLOSURE:
			return call(AS_CLOSURE(callee), arg_count);

		case OBJ_NATIVE:
		{
			NativeFn native = AS_NATIVE(callee);
			Value result = native(arg_count, vm.stack_top - arg_count);
			vm.stack_top -= arg_count + 1;
			push(result);
			return true;
		}

		default:
			break;
		}
	}

	runtime_error("Can only call functions and classes.");
	return false;
}

static bool invoke_from_class(ObjClass* klass, ObjString* name, int arg_count)
{
	Value method;
	if (!table_get(&klass->methods, name, &method)) {
		runtime_error("Undefined property '%s'.", name->chars);
		return false;
	}

	return call(AS_CLOSURE(method), arg_count);
}

static bool invoke(ObjString* name, int arg_count)
{
	Value receiver = peek(arg_count);

	ObjClass* obj_class = NULL;
	if (IS_INSTANCE(receiver))
	{
		ObjInstance* instance = AS_INSTANCE(receiver);

		Value value;
		if (table_get(&instance->fields, name, &value)) {
			vm.stack_top[-arg_count - 1] = value;
			return call_value(value, arg_count);
		}

		obj_class = instance->klass;
	}
	else if (IS_LIST(receiver))
	{
		char name_str[MAX_METHOD_SIGNATURE];
		int length;

		Signature signature = { name->chars, name->length, SIG_METHOD, arg_count };
		signature_to_string(&signature, name_str, &length);

		ObjString* signed_name = copy_string(name_str, length);

		ObjList* list = AS_LIST(receiver);

		Value value;
		if (table_get(&vm.list_class->methods, signed_name, &value)) {
			return call_value(value, arg_count);
		}

		obj_class = vm.list_class;
	}
	else if (IS_MAP(receiver))
	{
		char name_str[MAX_METHOD_SIGNATURE];
		int length;

		Signature signature = { name->chars, name->length, SIG_METHOD, arg_count };
		signature_to_string(&signature, name_str, &length);

		ObjString* signed_name = copy_string(name_str, length);

		ObjMap* map = AS_MAP(receiver);

		Value value;
		if (table_get(&vm.map_class->methods, signed_name, &value)) {
			return call_value(value, arg_count);
		}

		obj_class = vm.map_class;
	}
	else
	{
		runtime_error("Only instances and list have methods.");
		return false;
	}
	return invoke_from_class(obj_class, name, arg_count);
}

static bool bind_method(ObjClass* klass, ObjString* name)
{
	Value method;
	if (!table_get(&klass->methods, name, &method)) {
		runtime_error("Undefined property '%s'.", name->chars);
		return false;
	}

	ObjBoundMethod* bound = new_bound_method(peek(0), AS_CLOSURE(method));
	pop();
	push(OBJ_VAL(bound));
	return true;
}

static ObjUpvalue* capture_upvalue(Value* local)
{
	ObjUpvalue* prev_upvalue = NULL;
	ObjUpvalue* upvalue = vm.open_upvalues;

	while (upvalue != NULL && upvalue->location > local) {
		prev_upvalue = upvalue;
		upvalue = upvalue->next;
	}

	if (upvalue != NULL && upvalue->location == local) {
		return upvalue;
	}

	ObjUpvalue* created_upvalue = new_upvalue(local);
	created_upvalue->next = upvalue;

	if (prev_upvalue == NULL) {
		vm.open_upvalues = created_upvalue;
	} else {
		prev_upvalue->next = created_upvalue;
	}

	return created_upvalue;
}

static void close_upvalues(Value* last)
{
	while (vm.open_upvalues != NULL
		&& vm.open_upvalues->location >= last)
	{
		ObjUpvalue* upvalue = vm.open_upvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;
		vm.open_upvalues = upvalue->next;
	}
}

static void define_method(ObjString* name)
{
	Value method = peek(0);
	ObjClass* klass = AS_CLASS(peek(1));
	table_set(&klass->methods, name, method);
	pop();
}

static bool is_falsey(Value value)
{
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate()
{
	ObjString* b = AS_STRING(peek(0));
	ObjString* a = AS_STRING(peek(1));

	int length = a->length + b->length;
	char* chars = ALLOCATE(char, length + 1);
	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	ObjString* result = take_string(chars, length);
	pop();
	pop();
	push(OBJ_VAL(result));
}

static Value import_module(Value name)
{
	if (!IS_STRING(name)) {
		runtime_error("Module name is not string.");
		return NIL_VAL;
	}

	Value existing;
	if (table_get(&vm.modules, AS_STRING(name), &existing)) {
		return existing;
	}

	push(name);

	LoadModuleResult result = {0};
	const char* source = NULL;

	// Let the host try to provide the module.
	if (vm.config.load_module_fn != NULL) {
		result = vm.config.load_module_fn(AS_CSTRING(name));
	}

	// If the host didn't provide it, see if it's a built in optional module.
	if (result.source == NULL)
	{
		result.on_complete = NULL;
		ObjString* name_string = AS_STRING(name);
#if OPT_RANDOM
		if (strncmp(name_string->chars, "random", name_string->length) == 0) {
			result.source = RandomSource();
		}
#endif
	}

	if (result.source == NULL)
	{
		runtime_error("Could not load module.");
		pop(); // name.
		return NIL_VAL;
	}

	ObjFunction* module_func = compile(AS_STRING(name)->chars, result.source);

	// Now that we're done, give the result back in case there's cleanup to do.
	if (result.on_complete) {
		result.on_complete(AS_CSTRING(name), result);
	}

	if (module_func == NULL)
	{
		runtime_error("Could not load module.");
		pop(); // name.
		return NIL_VAL;
	}

	pop(); // name.

	// Return the closure that executes the module.
	return OBJ_VAL(module_func);
}

static InterpretResult run()
{
	CallFrame* frame = &vm.frames[vm.frame_count - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
    } while (false)

	for (;;)
	{
#ifdef DEBUG_TRACE_EXECUTION
		printf("          ");
		for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
			printf("[ ");
			ves_dump_value(*slot);
			printf(" ]");
		}
		printf("\n");

		disassembleInstruction(&frame->closure->function->chunk,
			(int)(frame->ip - frame->closure->function->chunk.code));
#endif

		uint8_t instruction = READ_BYTE();
#ifdef DEBUG_PRINT_OPCODE
		const char* names[OP_IMPORT_VARIABLE + 1] = {
			"OP_CONSTANT",
			"OP_NIL",
			"OP_TRUE",
			"OP_FALSE",
			"OP_POP",
			"OP_GET_LOCAL",
			"OP_SET_LOCAL",
			"OP_GET_GLOBAL",
			"OP_DEFINE_GLOBAL",
			"OP_SET_GLOBAL",
			"OP_GET_UPVALUE",
			"OP_SET_UPVALUE",
			"OP_GET_PROPERTY",
			"OP_SET_PROPERTY",
			"OP_GET_SUPER",
			"OP_EQUAL",
			"OP_GREATER",
			"OP_LESS",
			"OP_ADD",
			"OP_SUBTRACT",
			"OP_MULTIPLY",
			"OP_DIVIDE",
			"OP_NOT",
			"OP_NEGATE",
			"OP_PRINT",
			"OP_JUMP",
			"OP_JUMP_IF_FALSE",
			"OP_LOOP",
			"OP_CALL",
			"OP_INVOKE",
			"OP_SUPER_INVOKE",
			"OP_CLOSURE",
			"OP_CLOSE_UPVALUE",
			"OP_RETURN",
			"OP_CLASS",
			"OP_INHERIT",
			"OP_METHOD",
			"OP_LOAD_MODULE_VAR",
			"OP_CALL_0",
			"OP_CALL_1",
			"OP_CALL_2",
			"OP_CALL_3",
			"OP_CALL_4",
			"OP_CALL_5",
			"OP_CALL_6",
			"OP_CALL_7",
			"OP_CALL_8",
			"OP_CALL_9",
			"OP_CALL_10",
			"OP_CALL_11",
			"OP_CALL_12",
			"OP_CALL_13",
			"OP_CALL_14",
			"OP_CALL_15",
			"OP_CALL_16",
			"OP_IMPORT_MODULE",
			"OP_IMPORT_VARIABLE",
		};
		printf("%s\n", names[instruction]);
#endif // DEBUG_PRINT_OPCODE
		switch (instruction)
		{
		case OP_CONSTANT: {
			Value constant = READ_CONSTANT();
			push(constant);
			break;
		}
		case OP_NIL: push(NIL_VAL); break;
		case OP_TRUE: push(BOOL_VAL(true)); break;
		case OP_FALSE: push(BOOL_VAL(false)); break;
		case OP_POP: pop(); break;

		case OP_GET_LOCAL: {
			uint8_t slot = READ_BYTE();
			push(frame->slots[slot]);
			break;
		}

		case OP_SET_LOCAL: {
			uint8_t slot = READ_BYTE();
			frame->slots[slot] = peek(0);
			break;
		}

		case OP_GET_GLOBAL: {
			ObjString* name = READ_STRING();
			Value value;
			if (!table_get(&vm.globals, name, &value)) {
				runtime_error("Undefined variable '%s'.", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			push(value);
			break;
		}

		case OP_DEFINE_GLOBAL: {
			ObjString* name = READ_STRING();
			table_set(&vm.globals, name, peek(0));
			pop();
			break;
		}

		case OP_SET_GLOBAL: {
			ObjString* name = READ_STRING();
			if (table_set(&vm.globals, name, peek(0))) {
				table_delete(&vm.globals, name); // [delete]
				runtime_error("Undefined variable '%s'.", name->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}

		case OP_GET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			push(*frame->closure->upvalues[slot]->location);
			break;
		}

		case OP_SET_UPVALUE: {
			uint8_t slot = READ_BYTE();
			*frame->closure->upvalues[slot]->location = peek(0);
			break;
		}

		case OP_GET_PROPERTY: {
			Value receiver = peek(0);
			if (IS_INSTANCE(receiver))
			{
				ObjInstance* instance = AS_INSTANCE(receiver);
				ObjString* name = READ_STRING();

				Value value;
				if (table_get(&instance->fields, name, &value)) {
					pop(); // Instance.
					push(value);
					break;
				}
				if (!bind_method(instance->klass, name)) {
					return INTERPRET_RUNTIME_ERROR;
				}
			}
			else if (IS_LIST(receiver))
			{
				ObjList* list = AS_LIST(receiver);
				ObjString* name = READ_STRING();

				Value value;
				if (table_get(&vm.list_class->methods, name, &value))
				{
					pop(); // List
					if (IS_METHOD(value))
					{
						ObjMethod* method = AS_METHOD(value);
						switch (method->type)
						{
						case METHOD_PRIMITIVE:
							if (method->as.primitive(vm.stack_top)) {
								vm.stack_top += 1;
							} else {
								runtime_error("Run primitive fail.");
								return INTERPRET_RUNTIME_ERROR;
							}
							break;
						case METHOD_FUNCTION_CALL:
							break;
						case METHOD_FOREIGN:
							break;
						case METHOD_BLOCK:
							break;
						case METHOD_NONE:
							break;
						default:
							ASSERT(false, "Unknown method type.");
						}
					}
					else
					{
						push(value);
					}
					break;
				}
			}
			else if (IS_MAP(receiver))
			{
				ObjMap* map = AS_MAP(receiver);
				ObjString* name = READ_STRING();

				Value value;
				if (table_get(&vm.map_class->methods, name, &value))
				{
					pop(); // Map
					if (IS_METHOD(value))
					{
						ObjMethod* method = AS_METHOD(value);
						switch (method->type)
						{
						case METHOD_PRIMITIVE:
							if (method->as.primitive(vm.stack_top)) {
								vm.stack_top += 1;
							} else {
								runtime_error("Run primitive fail.");
								return INTERPRET_RUNTIME_ERROR;
							}
							break;
						case METHOD_FUNCTION_CALL:
							break;
						case METHOD_FOREIGN:
							break;
						case METHOD_BLOCK:
							break;
						case METHOD_NONE:
							break;
						default:
							ASSERT(false, "Unknown method type.");
						}
					}
					else
					{
						push(value);
					}
					break;
				}
			}
			else
			{
				runtime_error("Only instances and list and map have properties.");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}

		case OP_SET_PROPERTY: {
			if (!IS_INSTANCE(peek(1))) {
				runtime_error("Only instances have fields.");
				return INTERPRET_RUNTIME_ERROR;
			}

			ObjInstance* instance = AS_INSTANCE(peek(1));
			table_set(&instance->fields, READ_STRING(), peek(0));

			Value value = pop();
			pop();
			push(value);
			break;
		}

		case OP_GET_SUPER: {
			ObjString* name = READ_STRING();
			ObjClass* superclass = AS_CLASS(pop());
			if (!bind_method(superclass, name)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}

		case OP_EQUAL: {
			Value b = pop();
			Value a = pop();
			push(BOOL_VAL(values_equal(a, b)));
			break;
		}

		case OP_GREATER:  BINARY_OP(BOOL_VAL, > ); break;
		case OP_LESS:     BINARY_OP(BOOL_VAL, < ); break;

		case OP_ADD: {
			if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
				concatenate();
			} else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
				double b = AS_NUMBER(pop());
				double a = AS_NUMBER(pop());
				push(NUMBER_VAL(a + b));
			} else {
				runtime_error("Operands must be two numbers or two strings.");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
		case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
		case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, / ); break;
		case OP_NOT:
			push(BOOL_VAL(is_falsey(pop())));
			break;
		case OP_NEGATE:
			if (!IS_NUMBER(peek(0))) {
				runtime_error("Operand must be a number.");
				return INTERPRET_RUNTIME_ERROR;
			}

			push(NUMBER_VAL(-AS_NUMBER(pop())));
			break;

		case OP_PRINT: {
			ves_dump_value(pop());
			ves_str_buf_newline();
			break;
		}

		case OP_JUMP: {
			uint16_t offset = READ_SHORT();
			frame->ip += offset;
			break;
		}

		case OP_JUMP_IF_FALSE: {
			uint16_t offset = READ_SHORT();
			if (is_falsey(peek(0))) frame->ip += offset;
			break;
		}

		case OP_LOOP: {
			uint16_t offset = READ_SHORT();
			frame->ip -= offset;
			break;
		}

		case OP_CALL: {
			int arg_count = READ_BYTE();
			if (!call_value(peek(arg_count), arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}

		case OP_CALL_0:
		case OP_CALL_1:
		case OP_CALL_2:
		case OP_CALL_3:
		case OP_CALL_4:
		case OP_CALL_5:
		case OP_CALL_6:
		case OP_CALL_7:
		case OP_CALL_8:
		case OP_CALL_9:
		case OP_CALL_10:
		case OP_CALL_11:
		case OP_CALL_12:
		case OP_CALL_13:
		case OP_CALL_14:
		case OP_CALL_15:
		case OP_CALL_16:
		{
			// Add one for the implicit receiver argument.
			int arg_count = instruction - OP_CALL_0 + 1;
			ObjString* symbol = AS_STRING(vm.method_names.values[READ_SHORT()]);

			Value* args = vm.stack_top - arg_count;
			ASSERT(IS_OBJ(args[0]), "Should be obj.");
			ObjClass* class_obj = AS_OBJ(args[0])->class_obj;
			ASSERT(class_obj, "Should have class_obj.");

			Value v_method;
			if (!table_get(&class_obj->methods, symbol, &v_method)) {
				runtime_error("Method does not implement.");
				return INTERPRET_RUNTIME_ERROR;
			}

			ObjMethod* method = AS_METHOD(v_method);
			switch (method->type)
			{
			case METHOD_PRIMITIVE:
				if (method->as.primitive(args)) {
					// The result is now in the first arg slot. Discard the other stack slots.
					vm.stack_top -= arg_count - 1;
				} else {
					runtime_error("Run primitive fail.");
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			case METHOD_FUNCTION_CALL:
				break;
			case METHOD_FOREIGN:
				break;
			case METHOD_BLOCK:
				break;
			case METHOD_NONE:
				break;
			default:
				ASSERT(false, "Unknown method type.");
			}
		}
			break;

		case OP_INVOKE: {
			ObjString* method = READ_STRING();
			int arg_count = READ_BYTE();
			if (!invoke(method, arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}

		case OP_SUPER_INVOKE: {
			ObjString* method = READ_STRING();
			int arg_count = READ_BYTE();
			ObjClass* superclass = AS_CLASS(pop());
			if (!invoke_from_class(superclass, method, arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}

		case OP_CLOSURE: {
			ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
			ObjClosure* closure = new_closure(function);
			push(OBJ_VAL(closure));
			for (int i = 0; i < closure->upvalue_count; i++) {
				uint8_t is_local = READ_BYTE();
				uint8_t index = READ_BYTE();
				if (is_local) {
					closure->upvalues[i] = capture_upvalue(frame->slots + index);
				} else {
					closure->upvalues[i] = frame->closure->upvalues[index];
				}
			}
			break;
		}

		case OP_CLOSE_UPVALUE:
			close_upvalues(vm.stack_top - 1);
			pop();
			break;

		case OP_RETURN: {
			Value result = pop();

			close_upvalues(frame->slots);

			vm.frame_count--;
			if (vm.frame_count == 0) {
				pop();
				return INTERPRET_OK;
			}

			vm.stack_top = frame->slots;
			push(result);

			frame = &vm.frames[vm.frame_count - 1];
			break;
		}

		case OP_CLASS:
			push(OBJ_VAL(new_class(READ_STRING())));
			break;

		case OP_INHERIT: {
			Value superclass = peek(1);
			if (!IS_CLASS(superclass)) {
				runtime_error("Superclass must be a class.");
				return INTERPRET_RUNTIME_ERROR;
			}

			ObjClass* subclass = AS_CLASS(peek(0));
			table_add_all(&AS_CLASS(superclass)->methods, &subclass->methods);
			pop(); // Subclass.
			break;
		}

		case OP_METHOD:
			define_method(READ_STRING());
			break;

		case OP_LOAD_MODULE_VAR:
			push(frame->closure->function->module->variables.values[READ_SHORT()]);
			break;

		case OP_IMPORT_MODULE:
			push(import_module(READ_CONSTANT()));

			//if (wrenHasError(fiber)) RUNTIME_ERROR();

			// If we get a closure, call it to execute the module body.
			if (IS_FUNCTION(peek(0)))
			{
				//STORE_FRAME();
				ObjFunction* func = AS_FUNCTION(peek(0));

				//wrenCallFunction(vm, fiber, closure, 1);
				//LOAD_FRAME();
			}
			else
			{
				//// The module has already been loaded. Remember it so we can import
				//// variables from it if needed.
				//vm->lastModule = AS_MODULE(PEEK());
			}
			break;
		}
	}

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

void FinalizeForeign(ObjForeign* foreign)
{
	ObjClass* class_obj = foreign->obj.class_obj;

	Value value;
	if (!table_get(&class_obj->methods, copy_string("<finalize>", 10), &value)) {
		return;
	}

	ObjMethod* method = AS_METHOD(value);
	if (method->type == METHOD_NONE) {
		return;
	}

	ASSERT(method->type == METHOD_FOREIGN, "Finalizer should be foreign.");

	FinalizerFn finalizer = (FinalizerFn)method->as.foreign;
	finalizer(foreign->data);
}

InterpretResult interpret(const char* module, const char* source)
{
	ObjFunction* function = compile(module, source);
	if (function == NULL) {
		return INTERPRET_COMPILE_ERROR;
	}

	push(OBJ_VAL(function));

	ObjClosure* closure = new_closure(function);
	pop();
	push(OBJ_VAL(closure));
	call_value(OBJ_VAL(closure), 0);

	InterpretResult ret = run();

	if (vm.stack_top - &vm.stack[0] != 0) {
		runtime_error("Stack not empty.");
	}

	return ret;
}

static void validate_api_slot(int slot)
{
	ASSERT(slot >= 0, "Slot cannot be negative.");
	ASSERT(slot < GetSlotCount(vm), "Not that many slots.");
}

int GetSlotCount()
{
	if (vm.api_stack == NULL) {
		return 0;
	}

	return (int)(vm.stack_top - vm.api_stack);
}

double GetSlotDouble(int slot)
{
	validate_api_slot(slot);
	ASSERT(IS_NUMBER(vm.api_stack[slot]), "Slot must hold a number.");

	return AS_NUMBER(vm.api_stack[slot]);
}

void* GetSlotForeign(int slot)
{
	validate_api_slot(slot);
	ASSERT(IS_FOREIGN(vm.api_stack[slot]), "Slot must hold a foreign instance.");

	return AS_FOREIGN(vm.api_stack[slot])->data;
}

// Stores [value] in [slot] in the foreign call stack.
static void set_slot(int slot, Value value)
{
	validate_api_slot(slot);
	vm.api_stack[slot] = value;
}

void SetSlotDouble(int slot, double value)
{
	set_slot(slot, NUMBER_VAL(value));
}

void* SetSlotNewForeign(int slot, int classSlot, size_t size)
{
	validate_api_slot(slot);
	validate_api_slot(classSlot);
	ASSERT(IS_CLASS(vm.api_stack[classSlot]), "Slot must hold a class.");

	ObjClass* class_obj = AS_CLASS(vm.api_stack[classSlot]);
	ASSERT(class_obj->num_fields == -1, "Class must be a foreign class.");

	ObjForeign* foreign = new_foreign(size);
	foreign->obj.class_obj = class_obj;
	vm.api_stack[slot] = OBJ_VAL(foreign);

	return (void*)foreign->data;
}

int DefineVariable(ObjModule* module, const char* name, size_t length, Value value, int* line)
{
	if (module->variables.count == MAX_MODULE_VARS) {
		return -2;
	}

	if (IS_OBJ(value)) {
		push(value);
	}

	int symbol = symbol_table_find(&module->variable_names, name, length);
	if (symbol == -1)
	{
		symbol = symbol_table_add(&module->variable_names, name, length);
		write_value_array(&module->variables, value);
	}
	else
	{
		symbol = -1;
	}

	if (IS_OBJ(value)) {
		pop();
	}

	return symbol;
}