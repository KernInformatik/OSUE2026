#!/bin/bash

for prog in err1 err2 err3 err4 err5; do
    echo "=== Running $prog ==="
    ./"$prog"
    echo "Exit code: $?"
    echo
done