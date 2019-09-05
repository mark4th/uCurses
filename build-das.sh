#!/bin/bash

set -xueo pipefail

#CC=clang++ CFLAGS="-pedantic -Wall -Werror -std=c++17 -O2 -xc++ -c" make -f Makefile.das u
CC=clang CFLAGS="-Wall -Werror -std=c18 -O2 -xc -c" make -f Makefile.das u
