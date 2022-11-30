#!/bin/bash

NPROCS=( 1 4 9 16 25 36 49 64 81 100 121 144 )
L=( 96 192 288 384 480 576 672 768 864 960 1056 1152 )

for i in {0..12}
do
    printf "${NPROCS[i]} ${L[i]}\n"
done
