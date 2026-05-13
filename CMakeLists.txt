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
    src/TradeIntent.cpp
    src/Order.cpp
    src/ChecklistItem.cpp        
    src/ChecklistTemplate.cpp    
    src/ChecklistGate.cpp
    src/TradePipeline.cpp
    src/InMemoryTradeJournal.cpp
    src/SqliteTradeJournal.cpp
    src/MockBrokerAdapter.cpp
    src/InMemoryAccountStore.cpp
    src/SqliteAccountStore.cpp
    src/InMemoryChecklistTemplateStore.cpp
    src/SqliteChecklistTemplateStore.cpp
)

# Anyone who uses risk_manager_lib needs the include/ folder
# on their header search path. PUBLIC means: this lib needs it
# to compile, AND consumers also need it.
target_include_directories(risk_manager_lib PUBLIC include)
target_link_libraries(risk_manager_lib PUBLIC SQLiteCpp)

# ---- The example executable ----
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE risk_manager_lib)
# ---- The HTTP server executable ----
add_executable(http_server src/main_http.cpp)
target_link_libraries(http_server
    PRIVATE
        risk_manager_lib
        httplib::httplib
        nlohmann_json::nlohmann_json
)

# ---- Catch2 (fetched automatically) ----
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2     # pinned version, so builds are reproducible
)
FetchContent_MakeAvailable(Catch2)
FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG        3.3.2
)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.0
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(SQLiteCpp)
if(TARGET SQLiteCpp)
    target_compile_options(SQLiteCpp PUBLIC -include cstdint)
endif()
# ---- The test executable ----
# Tests will go in tests/. We'll add a placeholder file in a moment.
add_executable(tests
    tests/test_risk_manager.cpp
    tests/test_instrument.cpp
    tests/test_account.cpp
    tests/test_trade_intent.cpp
    tests/test_order.cpp
    tests/test_checklist_item.cpp      
    tests/test_checklist_template.cpp    
    tests/test_checklist_gate.cpp
    tests/test_trade_pipeline.cpp
    tests/test_in_memory_journal.cpp
    tests/test_sqlite_journal.cpp
    tests/test_mock_broker_adapter.cpp
    tests/test_in_memory_account_store.cpp
    tests/test_sqlite_account_store.cpp
    tests/test_in_memory_checklist_template_store.cpp
    tests/test_sqlite_checklist_template_store.cpp    
)
target_link_libraries(tests PRIVATE risk_manager_lib Catch2::Catch2WithMain)

# Register tests with CTest so we can run them with `ctest`.
include(CTest)
include(Catch)
catch_discover_tests(tests)