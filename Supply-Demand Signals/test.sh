#!/bin/bash

# Set colors for better readability
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Building the Supply-Demand Signals Project ===${NC}"

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure the project with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. || { echo -e "${RED}CMake configuration failed!${NC}"; exit 1; }

# Build the project
echo -e "${YELLOW}Building...${NC}"
cmake --build . || { echo -e "${RED}Build failed!${NC}"; exit 1; }

echo -e "${GREEN}Build completed successfully!${NC}"

# Run the executable
echo -e "${YELLOW}=== Running the application ===${NC}"
./SupplyDemandSignals

# Check exit status
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Application executed successfully!${NC}"
else
    echo -e "${RED}Application exited with an error code: $?${NC}"
fi
