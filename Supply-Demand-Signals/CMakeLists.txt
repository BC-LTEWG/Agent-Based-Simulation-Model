cmake_minimum_required(VERSION 3.10)
project(SupplyDemandSignals)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-Wall -Wno-deprecated-declarations)


# Uncomment the following line to enable Address Sanitizer
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

set(SOURCES
    src/main.cpp
    src/Firm.cpp
    src/Distributor.cpp
    src/Society.cpp
    src/Worker.cpp
    src/Accountant.cpp
    src/Project.cpp
)

include_directories(
    include
    extern/matplotlib-cpp
    ${Python3_INCLUDE_DIRS}
)

execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
  OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
include_directories(${NUMPY_INCLUDE_DIR})

add_executable(SupplyDemandSignals ${SOURCES})

target_link_libraries(SupplyDemandSignals PRIVATE ${Python3_LIBRARIES})
