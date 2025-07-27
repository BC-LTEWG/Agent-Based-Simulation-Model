# Subsystem: Supply and Demand Signals and Controls

This sub-project investigates the use of cybernetic signals and automatic responses to cause supply to converge with demand.
The key to this proposed solution is a top-level social budget for total work hours available, along
with a plan cycle and a plan approval mechanism (the automated "accountant").
Overproducing firms will not get approval to keep overproducing.
Hours will still be available for re-allocation to underproducing sectors.

## How to Run

### Prerequisites

This project requires CMake, Python with development headers, and a C++17 compatible compiler.

#### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install cmake g++ python3 python3-dev python3-pip
pip3 install matplotlib numpy
```

#### Linux (Other distributions)

```bash
# For RHEL/CentOS/Fedora:
sudo dnf install cmake gcc-c++ python3 python3-devel python3-pip
# or for older versions:
# sudo yum install cmake gcc-c++ python3 python3-devel python3-pip

pip3 install matplotlib numpy
```

#### Windows

1. Install CMake from https://cmake.org/download/
2. Install Python from https://python.org/downloads/ (ensure "Add to PATH" is checked)
3. Install Visual Studio Build Tools or Visual Studio Community
4. Open Command Prompt and run:

```cmd
pip install matplotlib numpy
```

### Building and Running

#### Linux/macOS

```bash
./build.sh
./build/SupplyDemandSignals
```

Or run both steps at once:

```bash
./test.sh
```

#### Windows

```cmd
mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build .
.\SupplyDemandSignals.exe
```
