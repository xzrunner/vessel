#include <vm.h>

int main(int argc, char* argv[])
{
    init_vm();

    auto ret = interpret(R"(

{
  var a = "before";
  print a; // expect: before

  a = "after";
  print a; // expect: after

  print a = "arg"; // expect: arg
  print a; // expect: arg
}


)");

    int zz = 0;

    free_vm();

    return 0;
}