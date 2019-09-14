#!/bin/bash

set -xueo pipefail

JOBS=$(nproc --all)

make -j ${JOBS} -f Makefile u
#strip -R .comment u
