#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}=== Building the Supply-Demand Signals Project ===${NC}"

mkdir -p build

cd build

echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. || { echo -e "${RED}CMake configuration failed!${NC}"; exit 1; }

echo -e "${YELLOW}Building...${NC}"
cmake --build . || { echo -e "${RED}Build failed!${NC}"; exit 1; }

echo -e "${GREEN}Build completed successfully!${NC}"

echo -e "${YELLOW}=== Running the application ===${NC}"
./SupplyDemandSignals

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Application executed successfully!${NC}"
else
    echo -e "${RED}Application exited with an error code: $?${NC}"
fi
