# Minimum CMake version required. 3.14+ gives us FetchContent for Catch2.
cmake_minimum_required(VERSION 3.14)

# Project name, version, and language.
project(DaytradingRiskManager VERSION 0.1.0 LANGUAGES CXX)

# C++ standard settings — applied to all targets in this project.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Turn up compiler warnings. Different compilers, different flags.
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- The library ----
# Compile RiskManager.cpp into a library called risk_manager_lib.
# Other targets (executable, tests) will link against this.
add_library(risk_manager_lib
    src/RiskManager.cpp
    src/Account.cpp
    src/Instrument.cpp
)

# Anyone who uses risk_manager_lib needs the include/ folder
# on their header search path. PUBLIC means: this lib needs it
# to compile, AND consumers also need it.
target_include_directories(risk_manager_lib PUBLIC include)

# ---- The example executable ----
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE risk_manager_lib)

# ---- Catch2 (fetched automatically) ----
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2     # pinned version, so builds are reproducible
)
FetchContent_MakeAvailable(Catch2)

# ---- The test executable ----
# Tests will go in tests/. We'll add a placeholder file in a moment.
add_executable(tests
    tests/test_risk_manager.cpp
    tests/test_instrument.cpp
    tests/test_account.cpp
)
target_link_libraries(tests PRIVATE risk_manager_lib Catch2::Catch2WithMain)

# Register tests with CTest so we can run them with `ctest`.
include(CTest)
include(Catch)
catch_discover_tests(tests)