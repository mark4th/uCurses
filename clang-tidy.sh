#!/bin/bash

set -euo pipefail

clang-format -i src/*.c h/*.h

for i in src/*.c h/*.h; do
    if grep -q ' $' $i ; then
        echo $i had trailing whitespace
        sed 's/ *$//' -i $i
    fi
done
