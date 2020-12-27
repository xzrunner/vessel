#ifndef opt_random_h
#define opt_random_h

#include "common.h"
#include "vessel.h"

#include <stdbool.h>

#if OPT_RANDOM

const char* RandomSource();
ForeignClassMethods RandomBindForeignClass(const char* module, const char* className);
ForeignMethodFn RandomBindForeignMethod(const char* className, bool isStatic, const char* signature);

#endif

#endif // opt_random_h
