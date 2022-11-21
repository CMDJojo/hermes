#include "test.h"

#include <iostream>

#include "gtfsTypes.h"

namespace test {
void tests() { gtfs::test(); }

void runAllTests() {
    std::cout << "[TEST] Running all tests..." << std::endl;
    gtfs::test();
    std::cout << "[TEST] All tests finished, exiting..." << std::endl;
}

void runThenExit() {
    runAllTests();
    exit(0);
}
}  // namespace test
