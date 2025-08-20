cmake_minimum_required(VERSION 3.16)
project(ProductivityIncentives)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Matplot++
find_package(Matplot++ REQUIRED)

# Add executable
add_executable(simulation
    src/main.cpp
    src/Firm.cpp
    src/Worker.cpp
    src/PriceController.cpp
    src/Simulation.cpp
)

# Include directories
target_include_directories(simulation PRIVATE include)

# Link libraries
target_link_libraries(simulation Matplot++::matplot)

# Set compiler warnings
target_compile_options(simulation PRIVATE -Wall -Wextra -O2) 