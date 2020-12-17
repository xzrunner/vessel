#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

#include <vm.h>

int main(int argc, char* argv[])
{
    init_vm();

    int result = Catch::Session().run(argc, argv);

    // global clean-up...

     return result;
}  