#!/bin/sh
cd /spool/users/boroden/current/KdRecTtau
LD_LIBRARY_PATH=/spool/users/boroden/current/lib
export LD_LIBRARY_PATH
echo Library path now is "$LD_LIBRARY_PATH"
../bin/Screening "$@"
#../tmp/Screening3 "$@"
