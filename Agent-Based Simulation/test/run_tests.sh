#!/bin/sh

for TEST in test_*.test
do
    ./${TEST}
done

