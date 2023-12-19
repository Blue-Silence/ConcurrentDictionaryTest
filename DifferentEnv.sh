#!/bin/bash

make
rm -rf ./TestUnderDifferentEnv
mkdir TestUnderDifferentEnv
cp a.out TestUnderDifferentEnv/
cd TestUnderDifferentEnv

for i in {1..5}; do
    echo R$i
    for trace in $(ls ../Traces | grep -E "^sym*"); do
        ./a.out RBM 128 ../Traces/$trace
        ./a.out HashM 128 ../Traces/$trace
        ./a.out ListL 128 ../Traces/$trace
    done
done