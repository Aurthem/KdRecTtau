#!/bin/sh
cd ~/current/KdRecTtau
LD_LIBRARY_PATH=~/current/lib
export LD_LIBRARY_PATH
echo Library path now is "$LD_LIBRARY_PATH"
../bin/Screening "$@"
