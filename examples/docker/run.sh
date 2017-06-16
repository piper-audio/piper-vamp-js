#!/bin/bash

set -eu

dockerfile=Dockerfile

if [ ! -f "$dockerfile" ]; then
    echo "Run this script from the directory with the Dockerfile in it"
fi

dockertag="piper-audio/piper-vamp-js-example"

sudo docker build -t "$dockertag" -f "$dockerfile" "."

#outdir="$dockerdir/output"
#mkdir -p "$outdir"
#
#container=$(sudo docker create "$dockertag")
#sudo docker cp "$container":output.tar "$outdir"
#sudo docker rm "$container"
#
#( cd "$outdir" ; tar xf output.tar && rm -f output.tar )
#
#echo
#echo "Done, output directory contains:"
#ls -ltr "$outdir"
