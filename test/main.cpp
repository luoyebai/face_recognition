// gtest
#include <gtest/gtest.h>
//
#include "detector.hpp"

int main(int argc, char const *argv[]) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
    return 0;
}
