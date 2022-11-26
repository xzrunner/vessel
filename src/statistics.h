#pragma once

#include "common.h"
#include "value.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef STATISTICS
#define STAT_TIMER_START \
	uint64_t __start_time = stat_timer_now();
#else
#define STAT_TIMER_START
#endif // STATISTICS

#ifdef STATISTICS
#define STAT_TIMER_END(obj) \
	uint64_t __end_time = stat_timer_now(); \
	obj->run_time += (__end_time - __start_time) * 0.000001; \
	__start_time = __end_time;
#else
#define STAT_TIMER_END(obj)
#endif // STATISTICS

#ifdef STATISTICS
#define STAT_UP_TIMES(obj) \
	obj->call_times++;
#else
#define STAT_UP_TIMES(obj)
#endif // STATISTICS

#ifdef STATISTICS

uint64_t stat_timer_now();

void stat_add_callee(Value obj_val);

void stat_begin();
void stat_end();

#endif // STATISTICS

#ifdef __cplusplus
}
#endif