cmake_minimum_required(VERSION 3.10)
project(SupplyDemandSignals)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")


include_directories(include)

add_compile_options(-Wall)

add_executable(SupplyDemandSignals 
    src/main.cpp
    src/Firm.cpp
    src/Distributor.cpp
    src/Society.cpp
    src/Worker.cpp
    src/Accountant.cpp
    src/Project.cpp
)
