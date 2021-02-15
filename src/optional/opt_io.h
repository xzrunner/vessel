#ifndef opt_io_h
#define opt_io_h

#include "common.h"
#include "vessel.h"

#include <stdbool.h>

#if OPT_IO

const char* IOSource();
VesselForeignClassMethods IOBindForeignClass(const char* module, const char* class_name);
VesselForeignMethodFn IOBindForeignMethod(const char* class_name, bool is_static, const char* signature);

#endif

#endif // opt_io_h
