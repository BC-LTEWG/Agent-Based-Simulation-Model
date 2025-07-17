cmake_minimum_required(VERSION 3.10)
project(SupplyDemandSignals)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Include directories
include_directories(include)

# Add executable target
add_executable(SupplyDemandSignals 
    src/main.cpp
    src/Firm.cpp
    src/Distributor.cpp
    src/Society.cpp
    src/Worker.cpp
    src/Accountant.cpp
    src/Plan.cpp
    src/Project.cpp
)
