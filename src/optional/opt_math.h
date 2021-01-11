#ifndef opt_math_h
#define opt_math_h

#include "common.h"
#include "vessel.h"

#include <stdbool.h>

#if OPT_MATH

const char* MathSource();
VesselForeignMethodFn MathBindMethod(const char* class_name, bool is_static, const char* signature);

#endif

#endif // opt_math_h
