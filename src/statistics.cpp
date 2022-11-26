#include "statistics.h"
#include "object.h"

#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>

#ifdef STATISTICS

namespace
{

template<typename T>
bool cmp_call_times(const T* a, const T* b)
{
    return a->call_times > b->call_times;
}

template<typename T>
bool cmp_run_time(const T* a, const T* b)
{
	return a->run_time > b->run_time;
}

std::vector<Value> func_callees, method_callees;

}

namespace vessel
{

extern "C"
void stat_add_callee(Value obj_val)
{
	switch (OBJ_TYPE(obj_val))
	{
	case OBJ_FUNCTION:
		func_callees.push_back(obj_val);
		break;
	case OBJ_METHOD:
		method_callees.push_back(obj_val);
		break;
	}
}

extern "C"
uint64_t stat_timer_now()
{
	return std::chrono::duration_cast<std::chrono::microseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
}

extern "C"
void stat_begin()
{

}

extern "C"
void stat_end()
{
	std::sort(func_callees.begin(), func_callees.end(), [](const Value& a, const Value& b)->bool
	{
		return cmp_run_time(AS_FUNCTION(a), AS_FUNCTION(b));
	});
	std::sort(method_callees.begin(), method_callees.end(), [](const Value& a, const Value& b)->bool
	{
		return cmp_run_time(AS_METHOD(a), AS_METHOD(b));
	});

	for (int i = 0; i < 30; ++i)
	{
		auto f = AS_FUNCTION(func_callees[i]);
		if (f->obj.type < 0) {
			continue;
		}
		auto name = f->name ? f->name->chars : "";
		auto m_name = f->module->name ? f->module->name->chars : "";
		printf("func [%d] [%lld, %f] %s, %s\n", i, f->call_times, f->run_time, name, m_name);
	}

	for (int i = 0; i < 30; ++i)
	{
		auto m = AS_METHOD(method_callees[i]);

		char* name = nullptr;
		char* m_name = "";
		switch (m->type)
		{
		case METHOD_PRIMITIVE:
			name = "primitive";
			break;
		case METHOD_FOREIGN:
			name = "foreign";
			break;
		case METHOD_BLOCK:
			name = m->as.closure->function->name->chars;
			m_name = m->as.closure->function->module->name->chars;
			break;
		default:
			std::cerr << "Unknown type: " << m->type;
		}

		printf("method [%d] [%lld, %f] %s, %s\n", i, m->call_times, m->run_time, name, m_name);
	}
}

}

#endif // STATISTICS