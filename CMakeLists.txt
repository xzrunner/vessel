cmake_minimum_required(VERSION 3.16)

project(vessel)

option(VESSEL_BUILD_TESTS "Set to ON to build the test suite." OFF)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "src/buffer.c"
    "src/buffer.h"
    "src/chunk.c"
    "src/chunk.h"
    "src/common.h"
    "src/compiler.c"
    "src/compiler.h"
    "src/core.c"
    "src/core.h"
    "src/core.ves.inc"
    "src/debug.c"
    "src/debug.h"
    "src/memory.c"
    "src/memory.h"
    "src/object.c"
    "src/object.h"
    "src/opcodes.h"
    "src/primitive.c"
    "src/primitive.h"
    "src/scanner.c"
    "src/scanner.h"
    "src/statistics.cpp"
    "src/statistics.h"
    "src/table.c"
    "src/table.h"
    "src/utils.c"
    "src/utils.h"
    "src/value.c"
    "src/value.h"
    "src/vm.c"
    "src/vm.h"
)
source_group("" FILES ${no_group_source_files})

set(include
    "src/include/vessel.h"
)
source_group("include" FILES ${include})

set(optional
    "src/optional/opt_io.c"
    "src/optional/opt_io.h"
    "src/optional/opt_io.ves.inc"
    "src/optional/opt_math.c"
    "src/optional/opt_math.h"
    "src/optional/opt_math.ves.inc"
    "src/optional/opt_random.c"
    "src/optional/opt_random.h"
    "src/optional/opt_random.ves.inc"
)
source_group("optional" FILES ${optional})

set(ALL_FILES
    ${no_group_source_files}
    ${include}
    ${optional}
)

add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC src/include)
target_include_directories(${PROJECT_NAME} PRIVATE src src/optional)

if(VESSEL_BUILD_TESTS)
    set(no_group_source_files
        "test/main.cpp"
        "test/utility.cpp"
        "test/utility.h"
    )
    source_group("" FILES ${no_group_source_files})

    set(tests
        "test/assignment.cpp"
        "test/block.cpp"
        "test/bool.cpp"
        "test/class.cpp"
        "test/closure.cpp"
        "test/comments.cpp"
        "test/constructor.cpp"
        "test/expressions.cpp"
        "test/field.cpp"
        "test/for.cpp"
        "test/function.cpp"
        "test/if.cpp"
        "test/inheritance.cpp"
        "test/list.cpp"
        "test/logical_operator.cpp"
        "test/map.cpp"
        "test/math.cpp"
        "test/method.cpp"
        "test/nil.cpp"
        "test/number.cpp"
        "test/operator.cpp"
        "test/random.cpp"
        "test/range.cpp"
        "test/return.cpp"
        "test/string.cpp"
        "test/super.cpp"
        "test/this.cpp"
        "test/variable.cpp"
        "test/while.cpp"
        "test/z_test.cpp"
    )
    source_group("tests" FILES ${tests})

    set(TEST_FILES
        ${no_group_source_files}
        ${tests}
    )

    if(NOT TARGET Catch2)
        add_subdirectory(third_party/Catch2)
    endif()

    add_executable(vessel-test ${TEST_FILES})
    target_include_directories(vessel-test PRIVATE src/include third_party/catch2/src)
    target_link_libraries(vessel-test vessel Catch2)
endif()
