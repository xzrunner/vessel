#include "opt_random.h"

#if OPT_RANDOM

#include "opt_random.ves.inc"

#include "vm.h"

#include <string.h>
#include <time.h>
#include <stdio.h>

// Implements the well equidistributed long-period linear PRNG (WELL512a).
//
// https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear
typedef struct
{
    uint32_t state[16];
    uint32_t index;
} Well512;

// Code from: http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
static uint32_t advance_state(Well512* well)
{
    uint32_t a, b, c, d;
    a = well->state[well->index];
    c = well->state[(well->index + 13) & 15];
    b =  a ^ c ^ (a << 16) ^ (c << 15);
    c = well->state[(well->index + 9) & 15];
    c ^= (c >> 11);
    a = well->state[well->index] = b ^ c;
    d = a ^ ((a << 5) & 0xda442d24U);

    well->index = (well->index + 15) & 15;
    a = well->state[well->index];
    well->state[well->index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
    return well->state[well->index];
}

static void random_allocate()
{
    Well512* well = (Well512*)vessel_set_slot_new_foreign(0, 0, sizeof(Well512));
    well->index = 0;
}

static void random_seed0()
{
    Well512* well = (Well512*)vessel_get_slot_foreign(0);

    srand((uint32_t)time(NULL));
    for (int i = 0; i < 16; i++) {
        well->state[i] = rand();
    }
}

static void random_seed1()
{
    Well512* well = (Well512*)vessel_get_slot_foreign(0);

    srand((uint32_t)vessel_get_slot_double(1));
    for (int i = 0; i < 16; i++) {
        well->state[i] = rand();
    }
}

static void random_seed16()
{
    Well512* well = (Well512*)vessel_get_slot_foreign(0);

    for (int i = 0; i < 16; i++) {
        well->state[i] = (uint32_t)vessel_get_slot_double(i + 1);
    }
}

static void random_float()
{
    Well512* well = (Well512*)vessel_get_slot_foreign(0);

    // A double has 53 bits of precision in its mantissa, and we'd like to take
    // full advantage of that, so we need 53 bits of random source data.

    // First, start with 32 random bits, shifted to the left 21 bits.
    double result = (double)advance_state(well) * (1 << 21);

    // Then add another 21 random bits.
    result += (double)(advance_state(well) & ((1 << 21) - 1));

    // Now we have a number from 0 - (2^53). Divide be the range to get a double
    // from 0 to 1.0 (half-inclusive).
    result /= 9007199254740992.0;

    vessel_set_slot_double(0, result);
}

static void random_int0()
{
    Well512* well = (Well512*)vessel_get_slot_foreign(0);

    vessel_set_slot_double(0, (double)advance_state(well));
}

const char* RandomSource()
{
    return randomModuleSource;
}

VesselForeignClassMethods RandomBindForeignClass(const char* module, const char* class_name)
{
    ASSERT(strcmp(class_name, "Random") == 0, "Should be in Random class.");
    VesselForeignClassMethods methods;
    methods.allocate = random_allocate;
    methods.finalize = NULL;
    return methods;
}

VesselForeignMethodFn RandomBindForeignMethod(const char* class_name, bool is_static, const char* signature)
{
    ASSERT(strcmp(class_name, "Random") == 0, "Should be in Random class.");

    if (strcmp(signature, "<allocate>") == 0) return random_allocate;
    if (strcmp(signature, "seed_()") == 0) return random_seed0;
    if (strcmp(signature, "seed_(_)") == 0) return random_seed1;

    if (strcmp(signature, "seed_(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)") == 0)
    {
        return random_seed16;
    }

    if (strcmp(signature, "float()") == 0) return random_float;
    if (strcmp(signature, "int()") == 0) return random_int0;

    ASSERT(false, "Unknown method.");
    return NULL;
}

#endif
