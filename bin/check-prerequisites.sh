#!/bin/bash

set -eu

mydir=$(dirname "$0")

case "$mydir" in
    /*) ;;
    *) mydir=$(pwd)/"$mydir";;
esac

parent_dir=$(echo "$mydir/../..")

sibling_dir() {
    local name="$1"
    local dir=$parent_dir/"$name"
    local simplified=""
    while true; do
        simplified=$(echo "$dir" | sed 's,/[a-z][a-z-]*/../,/,')
        simplified=$(echo "$simplified" | sed 's,/./,/,')
        if [ "$simplified" = "$dir" ]; then break
        else dir="$simplified"
        fi
    done
    echo "$dir"
}

explain_and_exit() {
    cat 1>&2 <<EOF

    To build the examples and the generator program found here, you
    will need the following sibling directories. (That is, other
    repositories of code checked out in the same place locally as the
    piper-vamp-js repository is.)

    * vamp-plugin-sdk   - The Vamp plugin SDK and example plugins
    * piper             - The Piper protocol schema
    * piper-vamp-cpp    - C++ classes for Piper/Vamp adaptation
    * vamp-test-plugin  - Vamp Test Plugin, used as one of our examples

    You will also need the Emscripten compiler (em++) to build the
    example modules, the node.js environment to test loading them,
    and any further C/C++ libraries needed for the generator program
    such as the Sord RDF store.

EOF
    exit 2
}

for sibling in vamp-plugin-sdk piper piper-vamp-cpp vamp-test-plugin ; do
    dir=$(sibling_dir "$sibling")
    if [ ! -d "$parent_dir/$sibling" ]; then
        echo 1>&2
        echo "*** Failed to find sibling directory $sibling" 1>&2
        echo "*** (expected in full path: $dir)" 1>&2
        explain_and_exit
    fi
done

for program in em++ node ; do
    if ! "$program" -v >/dev/null 2>&1 ; then
        "$program" -v
        echo 1>&2
        echo "*** Failed to find or run required program $program" 1>&2
        explain_and_exit
    fi
done


