// Catch2 v3 requires exactly one translation unit to define the main.
// All spdlog loggers are also initialised here so every test file shares them.
#include <catch2/catch_session.hpp>
#include "dsa_framework/framework.h"   // triggers DSALog init on first use

int main(int argc, char* argv[]) {
    // Force logger creation before any test runs
    DSALog::info("DSA test suite starting");
    int result = Catch::Session().run(argc, argv);
    DSALog::info("DSA test suite finished - exit code {}", result);
    return result;
}