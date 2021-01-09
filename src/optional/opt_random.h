#ifndef opt_random_h
#define opt_random_h

#include "common.h"
#include "vessel.h"

#include <stdbool.h>

#if OPT_RANDOM

const char* RandomSource();
VesselForeignClassMethods RandomBindForeignClass(const char* module, const char* class_name);
VesselForeignMethodFn RandomBindForeignMethod(const char* class_name, bool is_static, const char* signature);

#endif

#endif // opt_random_h
