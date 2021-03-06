
# Piper Vamp Javascript and JSON Adapter

Compile your C++ Vamp plugin code into a Javascript module that
exports the Piper JSON protocol, for use with Javascript code that
uses Piper audio feature extractors.

If you have an existing Vamp plugin with source code, you can
generally use the code here to convert it (with a little tweaking) to
a browser/runtime-independent Javascript module that can then be
distributed and used in browser applications via the Piper
protocol. Limitations on performance and memory usage may apply.

## What this repository contains

 * Adapter code to make a Vamp plugin library into a module that
   exports request-handling functions for the Piper protocol's JSON
   serialisation. This code is in C++ and can be compiled to a native
   shared library or to Javascript with Emscripten.

 * Examples of usage with Vamp plugin code, found in the
   examples/vamp-example-plugins and examples/vamp-test-plugin
   directories.

 * A utility (piper-vamp-stub-generator) that loads up an existing
   Vamp plugin and queries it along with its accompanying metadata,
   writing out a first draft of the converted module's main function
   code based on what it finds in the plugin.

 * A complete worked example of using that utility and building the
   resulting module, in a Docker script that can be read as
   documentation or run within Docker on Linux, found in
   examples/docker/Dockerfile.

## Authors and licensing

Written by Chris Cannam and Lucas Thompson at the Centre for Digital
Music, Queen Mary, University of London.

Copyright (c) 2015-2017 Queen Mary, University of London, provided
under a BSD-style licence. See the file COPYING for details.

