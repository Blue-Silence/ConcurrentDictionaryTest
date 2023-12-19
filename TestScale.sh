#!/bin/bash

make
rm -rf ./TestScale
mkdir TestScale
cp a.out TestScale/
cd TestScale

for i in {1..10}; do
    echo R$i
    for t in {1,2,4,8,16,32,64,128,256,512}; do
        echo "Thread number:"$t
        for trace in $(ls ../Traces | grep -E "^symAccess-(1-1-0|10-5-1|1-1-1)"); do
            ./a.out RBM $t ../Traces/$trace
            ./a.out HashM $t ../Traces/$trace
            ./a.out ListL $t ../Traces/$trace
        done
    done
done