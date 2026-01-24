#!/bin/sh

for TEST in test_*.test
do
    echo "* RUNNING ${TEST} ***********"
    ./${TEST}
done

