#include "vm.h"
#include "value.h"
#include "memory.h"
#include "compiler.h"
#include "debug.h"
#include "core.h"
#include "object.h"
#include "utils.h"
#include "primitive.h"
#if OPT_RANDOM
#include "opt_random.h"
#endif // OPT_RANDOM
#if OPT_MATH
#include "opt_math.h"
#endif // OPT_MATH

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
	Value module;
	if (!table_get(&vm.modules, copy_string("Core", 4), &module)) {
		return;
	}

	push(OBJ_VAL(copy_string(name, (int)strlen(name))));
	push(OBJ_VAL(new_native(function)));
	DefineVariable(AS_MODULE(module), name, strlen(name), vm.stack[1], NULL);
	pop();
	pop();
}

void init_configuration(VesselConfiguration* config)
{
	config->load_module_fn = NULL;
	config->bind_foreign_method_fn = NULL;
	config->bind_foreign_class_fn = NULL;
	config->write_fn = NULL;
}

void ves_set_config(VesselConfiguration* cfg)
{
	if (cfg) {
		memcpy(&vm.config, cfg, sizeof(VesselConfiguration));
	}
}

void* ves_get_config()
{
	return &vm.config;
}

void ves_init_vm()
{
	reset_stack();

	vm.objects = NULL;

	vm.bytes_allocated = 0;
	vm.next_gc = 1024 * 1024;

	vm.gray_count = 0;
	vm.gray_capacity = 0;
	vm.gray_stack = NULL;

	init_table(&vm.strings);

	vm.init_str = copy_string("init", 4);
	vm.allocate_str = copy_string("<allocate>", 10);
	vm.finalize_str = copy_string("<finalize>", 10);

	init_table(&vm.modules);

	vm.api_stack = NULL;

	init_configuration(&vm.config);

	init_value_array(&vm.method_names);

	initialize_core();

	vm.last_module = NULL;

	define_native("clock", clock_native);
}

void ves_free_vm()
{
	free_table(&vm.strings);
	free_table(&vm.modules);
	free_value_array(&vm.method_names);
	free_objects();

	vm.init_str = NULL;
	vm.allocate_str = NULL;
	vm.finalize_str = NULL;
}

void push(Value value)
{
	*vm.stack_top = value;
	vm.stack_top++;

#ifdef DEBUG_PRINT_STACK
	const int stack_sz = ves_gettop();
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
		const char* names[OBJ_RANGE + 1] = {
			"bound_method",
			"class",
			"closure",
			"method",
			"function",
			"foreign",
			"instance",
			"native",
			"string",
			"upvalue",
			"module",
			"list",
			"map",
			"range",
		};
		const char* name = NULL;
		if (IS_INSTANCE(value)) {
			name = AS_INSTANCE(value)->klass->name->chars;
		} else if (IS_STRING(value)) {
			name = AS_STRING(value)->chars;
		} else if (IS_CLASS(value)) {
			name = AS_CLASS(value)->name->chars;
		}
		if (name == NULL) {
			printf("push obj %s, %d\n", names[OBJ_TYPE(value)], stack_sz);
		} else {
			printf("push obj %s %s, %d\n", names[OBJ_TYPE(value)], name, stack_sz);
		}
	}
		break;
	}
#endif // DEBUG_PRINT_STACK
}

Value pop()
{
	vm.stack_top--;
#ifdef DEBUG_PRINT_STACK
	printf("pop %d\n", ves_gettop());
#endif // DEBUG_PRINT_STACK
	return *vm.stack_top;
}

static Value peek(int distance)
{
	return vm.stack_top[-1 - distance];
}

static bool call(ObjClosure* closure, int arg_count)
{
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

			Signature signature = { vm.init_str->chars, vm.init_str->length, SIG_METHOD, arg_count };
			char name[MAX_METHOD_SIGNATURE];
			int length;
			signature_to_string(&signature, name, &length);
			if (table_get(&klass->methods, copy_string(name, length), &initializer)) {
				ASSERT(IS_METHOD(initializer), "Error method type.");
				ObjMethod* obj_method = AS_METHOD(initializer);
				ASSERT(obj_method->type == METHOD_BLOCK, "Method should be block.");
				return call(obj_method->as.closure, arg_count);
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
			//case METHOD_FUNCTION_CALL:
			//	break;
			case METHOD_FOREIGN:
			{
				ASSERT(vm.api_stack == NULL, "Cannot already be in foreign call.");
				vm.api_stack = vm.stack_top - arg_count - 1;

				method->as.foreign();

				// Discard the stack slots for the arguments and temporaries but leave one
				// for the result.
				vm.stack_top = vm.api_stack + 1;

				vm.api_stack = NULL;

				return true;
			}
				break;
			case METHOD_BLOCK:
				if (call(method->as.closure, arg_count)) {
					return true;
				} else {
					runtime_error("Run block fail.");
					return false;
				}
				break;
			//case METHOD_NONE:
			//	break;
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

	Signature signature;
	signature.name = name->chars;
	signature.length = name->length;
	signature.arity = arg_count;
	signature.type = SIG_METHOD;

	char name_str[MAX_METHOD_SIGNATURE];
	int length;
	signature_to_string(&signature, name_str, &length);

	if (!table_get(&klass->methods, copy_string(name_str, length), &method)) {
		runtime_error("Undefined property '%s'.", name->chars);
		return false;
	}

	ASSERT(IS_METHOD(method), "Error method type.");
	ObjMethod* obj_method = AS_METHOD(method);

	bool ret = false;
	switch (obj_method->type)
	{
	case METHOD_BLOCK:
		ret = call(obj_method->as.closure, arg_count);
		break;
	case METHOD_FOREIGN:
	{
		ASSERT(vm.api_stack == NULL, "Cannot already be in foreign call.");
		vm.api_stack = vm.stack_top - arg_count;

		obj_method->as.foreign();

		// Discard the stack slots for the arguments and temporaries but leave one
		// for the result.
		vm.stack_top = vm.api_stack + 1;

		vm.api_stack = NULL;
	}
		break;
	default:
		runtime_error("error method type.");
	}
	return ret;
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
	else
	{
		obj_class = get_class(receiver);
		if (obj_class == NULL) {
			runtime_error("Unknown type, no class_obj.");
			return false;
		}

		char name_str[MAX_METHOD_SIGNATURE];
		int length;

		Signature signature = { name->chars, name->length, SIG_METHOD, arg_count };
		signature_to_string(&signature, name_str, &length);

		ObjString* signed_name = copy_string(name_str, length);

		Value value;
		if (table_get(&obj_class->methods, signed_name, &value)) {
			return call_value(value, arg_count);
		}
	}
	return invoke_from_class(obj_class, name, arg_count);
}

static bool bind_method(ObjClass* klass, ObjString* name)
{
	Value method;
	if (!table_get(&klass->methods, name, &method))
	{
		bool find = false;
		for (int i = 0; i < 16; ++i)
		{
			Signature signature = { name->chars, name->length, SIG_METHOD, i };
			char name[MAX_METHOD_SIGNATURE];
			int length;
			signature_to_string(&signature, name, &length);
			if (table_get(&klass->methods, copy_string(name, length), &method)) {
				find = true;
				break;
			}
		}

		if (!find) {
			runtime_error("Undefined property '%s'.", name->chars);
			return false;
		}
	}

	ASSERT(IS_METHOD(method), "Error method type.");
	ObjMethod* obj_method = AS_METHOD(method);
	ASSERT(obj_method->type == METHOD_BLOCK, "Method should be block.");
	ObjBoundMethod* bound = new_bound_method(peek(0), obj_method->as.closure);
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

static VesselForeignMethodFn find_foreign_method(const char* moduleName, const char* class_name,
	                                       bool is_static, const char* signature)
{
	VesselForeignMethodFn method = NULL;

	if (vm.config.bind_foreign_method_fn != NULL)
	{
		method = vm.config.bind_foreign_method_fn(moduleName, class_name, is_static, signature);
	}

	// If the host didn't provide it, see if it's an optional one.
	if (method == NULL)
	{
#if OPT_RANDOM
		if (strcmp(moduleName, "random") == 0)
		{
			method = RandomBindForeignMethod(class_name, is_static, signature);
		}
#endif
#if OPT_MATH
		if (strcmp(moduleName, "math") == 0)
		{
			method = MathBindMethod(class_name, is_static, signature);
		}
#endif
	}

	return method;
}

static void define_method(ObjString* name, int method_type, ObjModule* module)
{
	Value class = peek(0);
	ASSERT(IS_CLASS(class), "Should be a class.");
	ObjClass* klass = AS_CLASS(class);

	Value method_val = peek(1);
	ObjMethod* method = new_method();
	if (IS_STRING(method_val))
	{
		const char* name = AS_CSTRING(method_val);

		method->type = METHOD_FOREIGN;
		method->as.foreign = find_foreign_method(module->name->chars,
			klass->name->chars, method_type == OP_METHOD_STATIC, name);

		if (method->as.foreign == NULL)
		{
			runtime_error("Could not find foreign method '@' for class $ in module '$'.",
				method_val, klass->name->chars, module->name->chars);
			return;
		}
	}
	else
	{
		ASSERT(IS_CLOSURE(method_val), "Error method type.");

		method->type = METHOD_BLOCK;
		method->as.closure = AS_CLOSURE(method_val);
	}

	if (method_type == OP_METHOD_STATIC) {
		klass = klass->obj.class_obj;
	}

	table_set(&klass->methods, name, OBJ_VAL(method));
	pop();
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

	ObjString* name_str = AS_STRING(name);

	Value existing;
	if (table_get(&vm.modules, name_str, &existing)) {
		return existing;
	}

	push(name);

	VesselLoadModuleResult result = {0};
	const char* source = NULL;

	// Let the host try to provide the module.
	if (vm.config.load_module_fn != NULL) {
		result = vm.config.load_module_fn(AS_CSTRING(name));
	}

	// If the host didn't provide it, see if it's a built in optional module.
	if (result.source == NULL)
	{
		result.on_complete = NULL;
#if OPT_RANDOM
		if (strncmp(name_str->chars, "random", name_str->length) == 0) {
			result.source = RandomSource();
		}
#endif
#if OPT_MATH
		if (strncmp(name_str->chars, "math", name_str->length) == 0) {
			result.source = MathSource();
		}
#endif
	}

	if (result.source == NULL)
	{
		runtime_error("Could not load module.");
		pop(); // name.
		return NIL_VAL;
	}

	ObjClosure* module_closure = compile(name_str->chars, result.source);

	// Now that we're done, give the result back in case there's cleanup to do.
	if (result.on_complete) {
		result.on_complete(AS_CSTRING(name), result);
	}

	if (module_closure == NULL)
	{
		runtime_error("Could not load module.");
		pop(); // name.
		return NIL_VAL;
	}

	pop(); // name.

	// Return the closure that executes the module.
	return OBJ_VAL(module_closure);
}

static void bind_foreign_class(ObjClass* class_obj, ObjModule* module)
{
	VesselForeignClassMethods methods;
	methods.allocate = NULL;
	methods.finalize = NULL;

	if (vm.config.bind_foreign_class_fn != NULL)
	{
		methods = vm.config.bind_foreign_class_fn(module->name->chars, class_obj->name->chars);
	}

	if (methods.allocate == NULL && methods.finalize == NULL)
	{
#if OPT_RANDOM
		if (strncmp("random", module->name->chars, module->name->length) == 0) {
			methods = RandomBindForeignClass(module->name->chars, class_obj->name->chars);
		}
#endif
	}

	if (methods.allocate != NULL)
	{
		ObjMethod* method = new_method();
		method->type = METHOD_FOREIGN;
		method->as.foreign = methods.allocate;
		table_set(&class_obj->methods, vm.allocate_str, OBJ_VAL(method));
	}

	if (methods.finalize != NULL)
	{
		ObjMethod* method = new_method();
		method->type = METHOD_FOREIGN;
		method->as.foreign = (VesselForeignMethodFn)methods.finalize;
		table_set(&class_obj->methods, vm.finalize_str, OBJ_VAL(method));
	}
}

static VesselInterpretResult run()
{
	CallFrame* frame = &vm.frames[vm.frame_count - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define FUNC (frame->closure->function)
#define READ_CONSTANT() \
    (FUNC->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return VES_INTERPRET_RUNTIME_ERROR; \
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
			dump_value(*slot, true);
			printf(" ]");
		}
		printf("\n");

		disassembleInstruction(&FUNC->chunk, (int)(frame->ip - FUNC->chunk.code));
#endif

		uint8_t instruction = READ_BYTE();
#ifdef DEBUG_PRINT_OPCODE
#define FUNCTION_NAME(name) #name
		const char* names[255] = {
#define OPCODE(name) FUNCTION_NAME(OP_##name),
#include "opcodes.h"
#undef OPCODE
		};
#undef FUNCTION_NAME
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
#ifdef DEBUG_PRINT_OPCODE
			printf("++ name %s\n", name->chars);
#endif // DEBUG_PRINT_OPCODE
			int symbol = symbol_table_find(&FUNC->module->variable_names, name->chars, name->length);
			if (symbol == -1) {
				runtime_error("Undefined variable '%s'.", name->chars);
				return VES_INTERPRET_RUNTIME_ERROR;
			}
			push(FUNC->module->variables.values[symbol]);
			break;
		}

		case OP_DEFINE_GLOBAL: {
			ObjString* name = READ_STRING();
#ifdef DEBUG_PRINT_OPCODE
			printf("++ name %s\n", name->chars);
#endif // DEBUG_PRINT_OPCODE
			int symbol = DefineVariable(FUNC->module, name->chars, name->length, peek(0), NULL);
			if (symbol == -1) {
				int symbol = symbol_table_find(&FUNC->module->variable_names, name->chars, name->length);
				if (symbol == -1) {
					runtime_error("Undefined variable '%s'.", name->chars);
					return VES_INTERPRET_RUNTIME_ERROR;
				}
				FUNC->module->variables.values[symbol] = peek(0);
			}
			pop();
			break;
		}

		case OP_SET_GLOBAL: {
			ObjString* name = READ_STRING();
#ifdef DEBUG_PRINT_OPCODE
			printf("++ name %s\n", name->chars);
#endif // DEBUG_PRINT_OPCODE
			int symbol = symbol_table_find(&FUNC->module->variable_names, name->chars, name->length);
			if (symbol == -1) {
				runtime_error("Undefined variable '%s'.", name->chars);
				return VES_INTERPRET_RUNTIME_ERROR;
			}
			FUNC->module->variables.values[symbol] = peek(0);
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
					return VES_INTERPRET_RUNTIME_ERROR;
				}
			}
			else
			{
				ObjClass* class_obj = get_class(receiver);
				if (class_obj == NULL) {
					runtime_error("Unknown type, no class_obj.");
					return VES_INTERPRET_RUNTIME_ERROR;
				}

				ObjString* name = READ_STRING();

				Value value;
				if (table_get(&class_obj->methods, name, &value))
				{
					pop();
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
								return VES_INTERPRET_RUNTIME_ERROR;
							}
							break;
						//case METHOD_FUNCTION_CALL:
						//	break;
						//case METHOD_FOREIGN:
						//	break;
						//case METHOD_BLOCK:
						//	break;
						//case METHOD_NONE:
						//	break;
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
			break;
		}

		case OP_SET_PROPERTY: {
			if (!IS_INSTANCE(peek(1))) {
				runtime_error("Only instances have fields.");
				return VES_INTERPRET_RUNTIME_ERROR;
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
				return VES_INTERPRET_RUNTIME_ERROR;
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
				return VES_INTERPRET_RUNTIME_ERROR;
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
				return VES_INTERPRET_RUNTIME_ERROR;
			}

			push(NUMBER_VAL(-AS_NUMBER(pop())));
			break;

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
				return VES_INTERPRET_RUNTIME_ERROR;
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

#ifdef DEBUG_PRINT_OPCODE
			printf("++ name %s\n", symbol->chars);
#endif // DEBUG_PRINT_OPCODE

			Value* args = vm.stack_top - arg_count;
			ObjClass* class_obj = get_class(args[0]);
			ASSERT(class_obj, "Should have class_obj.");

			Value v_method;
			if (!table_get(&class_obj->methods, symbol, &v_method)) {
				runtime_error("Method does not implement.");
				return VES_INTERPRET_RUNTIME_ERROR;
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
					return VES_INTERPRET_RUNTIME_ERROR;
				}
				break;
			//case METHOD_FUNCTION_CALL:
			//	break;
			case METHOD_FOREIGN:
			{
				ASSERT(vm.api_stack == NULL, "Cannot already be in foreign call.");
				vm.api_stack = vm.stack_top - arg_count;

				method->as.foreign();

				// Discard the stack slots for the arguments and temporaries but leave one
				// for the result.
				vm.stack_top = vm.api_stack + 1;

				vm.api_stack = NULL;
			}
				break;
			case METHOD_BLOCK:
				if (call(method->as.closure, arg_count - 1)) {
					//// The result is now in the first arg slot. Discard the other stack slots.
					//vm.stack_top -= arg_count - 1;
					frame = &vm.frames[vm.frame_count - 1];
				} else {
					runtime_error("Run block fail.");
				}
				break;
			//case METHOD_NONE:
			//	break;
			default:
				ASSERT(false, "Unknown method type.");
			}
		}
			break;

		case OP_INVOKE: {
			ObjString* method = READ_STRING();
			int arg_count = READ_BYTE();
			if (!invoke(method, arg_count)) {
				return VES_INTERPRET_RUNTIME_ERROR;
			}
			frame = &vm.frames[vm.frame_count - 1];
			break;
		}

		case OP_SUPER_INVOKE: {
			ObjString* method = READ_STRING();
			int arg_count = READ_BYTE();
			ObjClass* superclass = AS_CLASS(pop());
			if (!invoke_from_class(superclass, method, arg_count)) {
				return VES_INTERPRET_RUNTIME_ERROR;
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
				return VES_INTERPRET_OK;
			}

			vm.stack_top = frame->slots;
			push(result);

			frame = &vm.frames[vm.frame_count - 1];
			break;
		}

		case OP_CLASS:
			push(OBJ_VAL(new_class(vm.object_class, 0, READ_STRING())));
			break;

		case OP_FOREIGN_CLASS:
		{
			ObjClass* class_obj = new_class(vm.object_class, -1, READ_STRING());
			push(OBJ_VAL(class_obj));
			bind_foreign_class(class_obj, FUNC->module);
		}
			break;

		case OP_INHERIT: {
			Value superclass = peek(1);
			if (!IS_CLASS(superclass)) {
				runtime_error("Superclass must be a class.");
				return VES_INTERPRET_RUNTIME_ERROR;
			}

			ObjClass* subclass = AS_CLASS(peek(0));
			bind_superclass(subclass, AS_CLASS(superclass));
			pop(); // Subclass.
			break;
		}

		case OP_METHOD:
		case OP_METHOD_STATIC:
			define_method(READ_STRING(), instruction, FUNC->module);
			break;

		case OP_LOAD_MODULE_VAR:
			push(FUNC->module->variables.values[READ_SHORT()]);
			break;

		case OP_STORE_MODULE_VAR:
			FUNC->module->variables.values[READ_SHORT()] = peek(0);
			break;

		case OP_IMPORT_MODULE:
		{
			Value name = READ_CONSTANT();
#ifdef DEBUG_PRINT_OPCODE
			printf("++ name %s\n", AS_STRING(name)->chars);
#endif // DEBUG_PRINT_OPCODE
			push(import_module(name));
			if (IS_CLOSURE(peek(0)))
			{
				ASSERT(IS_STRING(name), "Should be string.");
				Value val;
				if (!table_get(&vm.modules, AS_STRING(name), &val)) {
					ASSERT(false, "Get module fail.");
				}

				ASSERT(IS_MODULE(val), "Get module fail.");
				vm.last_module = AS_MODULE(val);

				//STORE_FRAME();
				call(AS_CLOSURE(peek(0)), 0);
				//LOAD_FRAME();

				frame = &vm.frames[vm.frame_count - 1];
			}
			else
			{
				vm.last_module = AS_MODULE(peek(0));
			}
		}
			break;

		case OP_IMPORT_VARIABLE:
		{
			ObjString* variable = READ_STRING();
#ifdef DEBUG_PRINT_OPCODE
			printf("++ name %s\n", variable->chars);
#endif // DEBUG_PRINT_OPCODE
			ASSERT(vm.last_module != NULL, "Should have already imported module.");
			int symbol = symbol_table_find(&vm.last_module->variable_names, variable->chars, variable->length);
			if (symbol != -1) {
				push(vm.last_module->variables.values[symbol]);
			}
		}
			break;

		case OP_CONSTRUCT:
			break;

		case OP_FOREIGN_CONSTRUCT:
		{
			ASSERT(IS_CLASS(frame->slots[0]), "'this' should be a class.");

			ObjClass* class_obj = AS_CLASS(frame->slots[0]);
			ASSERT(class_obj->num_fields == -1, "Class must be a foreign class.");

			Value value;
			bool find = table_get(&class_obj->methods, vm.allocate_str, &value);
			ASSERT(find, "Not find allocator.");

			ObjMethod* method = AS_METHOD(value);
			ASSERT(method->type == METHOD_FOREIGN, "Allocator should be foreign.");

			// Pass the constructor arguments to the allocator as well.
			ASSERT(vm.api_stack == NULL, "Cannot already be in foreign call.");
			vm.api_stack = frame->slots;

			method->as.foreign();

			vm.api_stack = NULL;
		}
			break;

		case OP_END_MODULE:
			vm.last_module = FUNC->module;
			break;
		}
	}

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
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
		symbol = symbol_table_ensure(&module->variable_names, name, length);
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

void FinalizeForeign(ObjForeign* foreign)
{
	ObjClass* class_obj = foreign->obj.class_obj;

	Value value;
	if (!table_get(&class_obj->methods, vm.finalize_str, &value)) {
		return;
	}

	ObjMethod* method = AS_METHOD(value);
	if (method->type == METHOD_NONE) {
		return;
	}

	ASSERT(method->type == METHOD_FOREIGN, "Finalizer should be foreign.");

	VesselFinalizerFn finalizer = (VesselFinalizerFn)method->as.foreign;
	finalizer(foreign->data);
}

VesselInterpretResult ves_interpret(const char* module, const char* source)
{
	return ves_run(ves_compile(module, source));
}

void* ves_compile(const char* module, const char* source)
{
	return compile(module, source);
}

VesselInterpretResult ves_run(void* closure)
{
	if (closure == NULL) {
		return VES_INTERPRET_COMPILE_ERROR;
	}

	push(OBJ_VAL(closure));
	call_value(OBJ_VAL(closure), 0);

	VesselInterpretResult ret = run();

	if (ves_gettop() != 0) {
		runtime_error("Stack not empty.");
	}

	return ret;
}

static Value get_stack_value(int index)
{
	if (index >= 0)
	{
		int num = 0;
		if (vm.api_stack != NULL) {
			num = (int)(vm.stack_top - vm.api_stack);
		}
		ASSERT(index < num, "Not that many slots.");
		return vm.api_stack[index];
	}
	else
	{
		const int num = ves_gettop();
		ASSERT(-index <= num, "Not that many slots.");
		return vm.stack_top[index];
	}
}

int ves_gettop()
{
	return vm.stack_top - &vm.stack[0];
}

VesselType ves_type(int index)
{
	Value val = get_stack_value(index);

	if (IS_BOOL(val)) return VES_TYPE_BOOL;
	if (IS_NUMBER(val)) return VES_TYPE_NUM;
	if (IS_FOREIGN(val)) return VES_TYPE_FOREIGN;
	if (IS_LIST(val)) return VES_TYPE_LIST;
	if (IS_MAP(val)) return VES_TYPE_MAP;
	if (IS_NIL(val)) return VES_TYPE_NULL;
	if (IS_STRING(val)) return VES_TYPE_STRING;
	if (IS_INSTANCE(val)) return VES_TYPE_INSTANCE;

	return VES_TYPE_UNKNOWN;
}

int ves_len(int index)
{
	Value val = get_stack_value(index);
	if (IS_LIST(val)) {
		return AS_LIST(val)->elements.count;
	} else if (IS_MAP(val)) {
		return AS_MAP(val)->entries.count;
	} else {
		runtime_error("Unsupported type.");
		return 0;
	}
}

void ves_geti(int index, int i)
{
	Value val = get_stack_value(index);
	ASSERT(IS_LIST(val), "Slot must hold a list.");

	ValueArray* elements = &AS_LIST(val)->elements;

	uint32_t used_index = validate_index_value(elements->count, (double)i, "Index");
	ASSERT(used_index != UINT32_MAX, "Index out of bounds.");

	push(elements->values[used_index]);
}

double ves_tonumber(int index)
{
	Value val = get_stack_value(index);
	ASSERT(IS_NUMBER(val), "Slot must hold a number.");

	return AS_NUMBER(val);

}

bool ves_toboolean(int index)
{
	Value val = get_stack_value(index);
	ASSERT(IS_BOOL(val), "Slot must hold a bool.");

	return AS_BOOL(val);
}

const char* ves_tostring(int index)
{
	Value val = get_stack_value(index);
	ASSERT(IS_STRING(val), "Slot must hold a string.");

	return AS_STRING(val)->chars;
}

void* ves_toforeign(int index)
{
	Value val = get_stack_value(index);
	ASSERT(IS_FOREIGN(val), "Slot must hold a foreign instance.");

	return AS_FOREIGN(val)->data;
}

void ves_pushnumber(double n)
{
	push(NUMBER_VAL(n));
}

void ves_pushboolean(int b)
{
	push(BOOL_VAL(b != 0));
}

void ves_pushstring(const char* s)
{
	push(OBJ_VAL(copy_string(s, strlen(s))));
}

void ves_pushlstring(const char* s, size_t len)
{
	push(OBJ_VAL(copy_string(s, len)));
}

void ves_pop(int n)
{
	for (int i = 0; i < n; ++i) {
		pop();
	}
}

int ves_getfield(int index, const char* k)
{
	Value val = get_stack_value(index);
	ASSERT(IS_MAP(val), "Slot must hold a map.");

	ObjMap* map = AS_MAP(val);
	Value value = NIL_VAL;
	table_get(&map->entries, copy_string(k, strlen(k)), &value);
	push(value);

	return ves_type(-1);
}

int ves_getglobal(const char* name)
{
	int ret = VES_TYPE_NULL;

	int symbol = symbol_table_find(&vm.last_module->variable_names, name, strlen(name));
	if (symbol != -1) {
		Value val = vm.last_module->variables.values[symbol];
		push(val);
		ret = ves_type(-1);
	} else {
		push(NIL_VAL);
	}

	return ret;
}

VesselInterpretResult ves_call(int nargs, int nresults)
{
	ASSERT(IS_STRING(peek(0)), "Method name should be string.");
	ObjString* s_method = AS_STRING(peek(0));
	pop();

	Value* args = vm.stack_top - nargs - 1;
	ObjClass* class_obj = get_class(args[0]);
	ASSERT(class_obj, "Should have class_obj.");

	Value v_method;
	if (!table_get(&class_obj->methods, s_method, &v_method)) {
		runtime_error("Method does not implement.");
		return VES_INTERPRET_RUNTIME_ERROR;
	}

	ObjMethod* method = AS_METHOD(v_method);
	switch (method->type)
	{
	case METHOD_BLOCK:
		if (!call(method->as.closure, nargs)) {
			runtime_error("Run block fail.");
		}
		break;
	default:
		runtime_error("Unknown method type.");
	}

	Value* stack_top = vm.stack_top - nargs;
	VesselInterpretResult ret = run();
	vm.stack_top = stack_top;

	return ret;
}

static void set_slot(int slot, Value value)
{
	int num = 0;
	if (vm.api_stack != NULL) {
		num = (int)(vm.stack_top - vm.api_stack);
	}
	ASSERT(slot < num, "Not that many slots.");

	vm.api_stack[slot] = value;
}

void ves_set_number(int slot, double value)
{
	set_slot(slot, NUMBER_VAL(value));
}

void* ves_set_newforeign(int slot, int class_slot, size_t size)
{
	ASSERT(IS_CLASS(vm.api_stack[class_slot]), "Slot must hold a class.");

	ObjClass* class_obj = AS_CLASS(vm.api_stack[class_slot]);
	ASSERT(class_obj->num_fields == -1, "Class must be a foreign class.");

	ObjForeign* foreign = new_foreign(size, class_obj);
	foreign->obj.class_obj = class_obj;
	vm.api_stack[slot] = OBJ_VAL(foreign);

	return (void*)foreign->data;
}