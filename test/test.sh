#!/bin/bash

set -eu

mydir="$(dirname $0)"

cd "$mydir"

echo
echo "Running C++ test..."
echo

( cd ../examples/vamp-example-plugins && make clean linux )

make quick-test

./quick-test > /tmp/$$

if ! cmp /tmp/$$ ./expected.txt ; then
    echo
    echo "FAILED: output not as expected: diff follows:"
    cp /tmp/$$ ./obtained.txt
    diff -u ./obtained.txt ./expected.txt
    exit 1
fi

echo
echo "Running Emscripten tests..."
echo

NODE=nodejs
if ! nodejs -v >/dev/null 2>&1 ; then
    NODE=node
    if ! node -v >/dev/null 2>&1 ; then
        NODE=""
    fi
fi
export NODE

( cd ../examples/vamp-example-plugins && make clean test )
( cd ../examples/vamp-test-plugin && make clean test )

echo
echo "Done"
echo
