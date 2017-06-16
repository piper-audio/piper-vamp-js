# piper-vamp-js/examples/docker/Dockerfile
#
# Example of the whole process of taking an existing Vamp plugin
# and turning it into a Piper Javascript module.
#
# The process is not automatic: you have to create a C++ main file and
# adjust build properties in the Makefile.
#
# Normally you would create a repo for your converted module and keep
# your newly-created files in that. (The other examples in this repo,
# examples/vamp-example-plugins and examples/vamp-test-plugin,
# illustrate that.)
#
# However in this example we will start with a Vamp plugin that we
# have not yet written any of these files for, and illustrate creating
# those -- although using batch processing tools rather than an
# interactive editor.
#
# This is provided as a Docker file in order to ensure it can be
# reproducibly run, and illustrate all the steps from a clean
# environment.
#
# There are four phases to this:
#
# 1. Install prerequisite packages, download all needed repos, and
#    build dependencies and required tools.
#
# 2. Obtain the Vamp plugin that we are actually intending to convert,
#    and compile it in the normal way (i.e. as a C++ plugin) so that
#    the generator can load and examine it.
#
# 3. Make a new working folder for our conversion, generate an
#    initial version of the C++ main entry point for the module, and
#    edit anything that the generator got wrong.
#
# 4. Create a Makefile and run the JS module build.
#
# Let's go...
#
# (Using Arch Linux base: installing Emscripten on Ubuntu is a right
# mess unless you want to recompile the whole thing)
#
FROM base/archlinux:2017.06.01
MAINTAINER Chris Cannam <cannam@all-day-breakfast.com>


# Phase 1. Install prerequisite packages...
#
RUN echo 'Server = http://archlinux.mirrors.uk2.net/$repo/os/$arch' > /etc/pacman.d/mirrorlist
RUN pacman --noconfirm -Syu && \
    pacman --noconfirm -Sy \
    gcc \
    make \
    boost \
    pkg-config \
    emscripten \
    python \
    jre8-openjdk-headless \
    nodejs \
    libsndfile \
    curl \
    git \
    mercurial
RUN rm -rf /var/cache/pacman/pkg/*
ENV PATH $PATH:/usr/lib/emscripten
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8  
#
# ... download all needed repos, including this one! ...
#
RUN git clone https://github.com/piper-audio/piper
RUN git clone https://github.com/piper-audio/piper-vamp-js
RUN git clone https://github.com/piper-audio/piper-vamp-cpp
RUN git clone https://github.com/c4dm/vamp-plugin-sdk
RUN git clone https://github.com/c4dm/vamp-test-plugin
#
# ... and build the dependencies and required tools
#
WORKDIR vamp-plugin-sdk
RUN ./configure && make && make install
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/local/lib
WORKDIR ..
WORKDIR piper-vamp-js
# (Get the first run of Emscripten, which sets it up, out of the way)
RUN em++ --version
RUN make
WORKDIR ..


# Phase 2. Obtain the Vamp plugin that we are actually intending to
# convert, and compile it in the normal way (i.e. as a C++ plugin)
# so that the generator can load and examine it.
# For this example we are going to convert the Simple Cepstral Pitch
# Tracker plugin, see
# https://code.soundsoftware.ac.uk/projects/cepstral-pitchtracker
# 
RUN git clone https://github.com/cannam/cepstral-pitchtracker
WORKDIR cepstral-pitchtracker
RUN make -f Makefile.linux64
WORKDIR ..


# Phase 3. Make a new working folder for our conversion...
#
RUN mkdir js-cepstral-pitchtracker
WORKDIR js-cepstral-pitchtracker
#
# ... generate an initial version of the C++ main entry point for
# the module...
#
RUN VAMP_PATH=../cepstral-pitchtracker \
    ../piper-vamp-js/bin/piper-vamp-stub-generator cepstral-pitchtracker \
    | tee cepstral-pitchtracker.cpp
#
# ... and edit anything that the generator got wrong. In this case,
# the only problem is that it has written [cC]epstralPitchtracker where
# the original code has [cC]epstralPitchTracker. Let's also remove the
# prompt comments the generator included.
#
RUN perl -i -p -e 's/Pitchtracker/PitchTracker/g' cepstral-pitchtracker.cpp
RUN perl -i -p -e 's,^//.*\n,,' cepstral-pitchtracker.cpp
RUN cat cepstral-pitchtracker.cpp


# Phase 4. Create a Makefile and run the JS module build. This is a
# bit ugly because we're trying to stuff the whole content of the
# Makefile into our Docker script!
#
RUN echo \
'PIPER_VAMP_JS_DIR	:= ../piper-vamp-js\n\n\
PLUGIN_DIR		:= ../cepstral-pitchtracker\n\
SRC_DIR			:= $(PLUGIN_DIR)\n\n\
MODULE_NAME		:= CepstralPitchTracker\n\
MODULE_SOURCE		:= cepstral-pitchtracker.cpp\n\n\
PLUGIN_SOURCES	        := $(SRC_DIR)/AgentFeeder.cpp $(SRC_DIR)/CepstralPitchTracker.cpp $(SRC_DIR)/NoteHypothesis.cpp $(SRC_DIR)/PeakInterpolator.cpp\n\n\
INCLUDES	        := -I$(SRC_DIR)\n\n\
include $(PIPER_VAMP_JS_DIR)/Makefile.inc' | sed 's/\\n/\n/g' > Makefile
RUN cat Makefile
RUN make em
RUN ls -l
RUN make test

