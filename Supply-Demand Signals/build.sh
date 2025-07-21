#!/bin/bash

mkdir -p build

cd build

cmake ..

cmake --build .

echo "Build completed. Run the executable with: ./build/SupplyDemandSignals"
