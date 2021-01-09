#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

#include <vessel.h>

int main(int argc, char* argv[])
{
    vessel_init_vm();

    int result = Catch::Session().run(argc, argv);

    // global clean-up...

     return result;
}  