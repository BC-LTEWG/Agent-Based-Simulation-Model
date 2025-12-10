#!/bin/bash
set -euo pipefail

# Path to flamegraph scripts (edit if needed)
FLAMEGRAPH_DIR="/home/halp/pkg/flamegraph"

# Step 1: Build the project
echo "[1/4] Building project..."
./build.sh

# Step 2: Run perf to collect samples
echo "[2/4] Running perf profiling..."
perf record -F 99 -g -- ./build/SupplyDemandSignals "$@"

# Step 3: Convert perf data to folded stacks
echo "[3/4] Converting perf output to folded stack..."
perf script > out.perf
"$FLAMEGRAPH_DIR/stackcollapse-perf.pl" out.perf > folded.txt

# Step 4: Generate flamegraph SVG
echo "[4/4] Generating flamegraph..."
"$FLAMEGRAPH_DIR/flamegraph.pl" folded.txt > flamegraph.svg

echo "✅ Flamegraph generated: flamegraph.svg"

rm out.perf perf.data folded.txt