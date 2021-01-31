#include "opt_math.h"

#if OPT_MATH

#include "opt_math.ves.inc"

#include <math.h>
#include <string.h>

#define DEF_MATH_FUNC(name, fn)           \
static void math_##name()                 \
{                                         \
	double num = ves_tonumber(1);         \
	ves_set_number(0, fn(num));           \
}

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)
#define	CALL_MATH_FUNC(name)              \
if (strcmp(signature, STRINGIZE(PPCAT(name, (_)))) == 0) return math_##name;

DEF_MATH_FUNC(abs, fabs)
DEF_MATH_FUNC(acos, acos)
DEF_MATH_FUNC(asin, asin)
DEF_MATH_FUNC(atan, atan)
DEF_MATH_FUNC(ceil, ceil)
DEF_MATH_FUNC(cos, cos)
DEF_MATH_FUNC(floor, floor)
DEF_MATH_FUNC(negate, -)
DEF_MATH_FUNC(round, round)
DEF_MATH_FUNC(sin, sin)
DEF_MATH_FUNC(sqrt, sqrt)
DEF_MATH_FUNC(tan, tan)
DEF_MATH_FUNC(log, log)
DEF_MATH_FUNC(log2, log2)
DEF_MATH_FUNC(exp, exp)

static void math_pi()
{
	ves_set_number(0, 3.14159265358979323846264338327950288);
}

static void math_min()
{
	double num = min(ves_tonumber(1), ves_tonumber(2));
	ves_set_number(0, num);
}

static void math_max()
{
	double num = max(ves_tonumber(1), ves_tonumber(2));
	ves_set_number(0, num);
}

const char* MathSource()
{
	return mathModuleSource;
}

VesselForeignMethodFn MathBindMethod(const char* class_name, bool is_static, const char* signature)
{
    ASSERT(strcmp(class_name, "Math") == 0, "Should be in Random class.");

	CALL_MATH_FUNC(abs)
	CALL_MATH_FUNC(acos)
	CALL_MATH_FUNC(asin)
	CALL_MATH_FUNC(atan)
	CALL_MATH_FUNC(ceil)
	CALL_MATH_FUNC(cos)
	CALL_MATH_FUNC(floor)
	CALL_MATH_FUNC(negate)
	CALL_MATH_FUNC(round)
	CALL_MATH_FUNC(sin)
	CALL_MATH_FUNC(sqrt)
	CALL_MATH_FUNC(tan)
	CALL_MATH_FUNC(log)
	CALL_MATH_FUNC(log2)
	CALL_MATH_FUNC(exp)

	if (strcmp(signature, "pi()") == 0) return math_pi;
	if (strcmp(signature, "min(_,_)") == 0) return math_min;
	if (strcmp(signature, "max(_,_)") == 0) return math_max;

    ASSERT(false, "Unknown method.");
    return NULL;
}

#endif