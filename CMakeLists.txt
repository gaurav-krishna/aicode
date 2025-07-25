cmake_minimum_required(VERSION 3.16)
project(QuantLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/FixedCouponBond.cpp
    src/MultiMarketEnv.cpp
    src/BondPricingEngine.cpp
    src/Date.cpp
    src/CashFlow.cpp
    src/InterestRateCurve.cpp
)

# Header files
set(HEADERS
    include/FixedCouponBond.h
    include/MultiMarketEnv.h
    include/BondPricingEngine.h
    include/Date.h
    include/CashFlow.h
    include/InterestRateCurve.h
    include/Common.h
)

# Create library
add_library(QuantLib ${SOURCES} ${HEADERS})

# Example executable
add_executable(bond_example examples/bond_example.cpp)
target_link_libraries(bond_example QuantLib)

# Test executable
add_executable(tests tests/test_bond_pricing.cpp)
target_link_libraries(tests QuantLib)

# Compiler flags
target_compile_options(QuantLib PRIVATE -Wall -Wextra -O3)