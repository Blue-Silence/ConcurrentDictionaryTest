#!/bin/bash

cd $1
echo $1

echo -e "Log summary\n\n" > Summary
echo -e "Type Ops-per-ms Overall-ops-per-ms\n\n" >> Summary

for log in $(ls | grep -E "^TestResult*"); do 
    cat $log | awk '{sum1+=$6; sum2+=$12 } END {print "'$log'  " sum1/NR,"  ", sum2/NR }' >> Summary
done