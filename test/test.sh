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
    diff -u /tmp/$$ ./expected.txt
    exit 1
fi

echo
echo "Running Emscripten tests..."
echo

( cd ../examples/vamp-example-plugins && make clean test )
( cd ../examples/vamp-test-plugin && make clean test )

echo
echo "Done"
echo
