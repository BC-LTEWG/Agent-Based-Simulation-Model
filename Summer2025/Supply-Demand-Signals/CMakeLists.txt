cmake_minimum_required(VERSION 3.14)
project(SupplyDemandSignals)

# Try to find installed matplot++ first
find_package(Matplot++ QUIET)

if(NOT Matplot++_FOUND)
    message(STATUS "Matplot++ not found system-wide, using external version")
    add_subdirectory(extern/matplotplusplus)
else()
    message(STATUS "Using system-installed Matplot++")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-Wall -Wno-deprecated-declarations)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -g")

set(SOURCES
    src/main.cpp
    src/Accountant.cpp
    src/Distributor.cpp
    src/Firm.cpp
    src/Good.cpp
    src/Plan.cpp
    src/PlotHandler.cpp
    src/Project.cpp
    src/randomizer.cpp
    src/Society.cpp
    src/Worker.cpp
)

add_executable(SupplyDemandSignals ${SOURCES})

# Link against matplot++ - use different target names depending on source
if(Matplot++_FOUND)
    target_link_libraries(SupplyDemandSignals PUBLIC Matplot++::matplot)
else()
    target_link_libraries(SupplyDemandSignals PUBLIC matplot)
endif()

target_include_directories(SupplyDemandSignals PRIVATE ${CMAKE_SOURCE_DIR}/include)
