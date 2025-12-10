#!/bin/bash

./build.sh || exit 1
./build/SupplyDemandSignals "$@"
