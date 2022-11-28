#include "test.h"

#include <boost/json/src.hpp>
#include <iostream>

#include "gtfsTypes.h"
#include "people.h"
#include "routingCacher.h"

namespace test {
void tests() {
    gtfs::test();
    routingCacher::test();
    People::test();
}

void runAllTests() {
    std::cout << "[TEST] Running all tests..." << std::endl;
    tests();
    std::cout << "[TEST] All tests finished, exiting..." << std::endl;
}

void runThenExit() {
    runAllTests();
    exit(0);
}
}  // namespace test

int main() { test::runThenExit(); }
