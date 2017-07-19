
# This GNU Makefile fragment is intended to be included from the
# Makefile for a Piper adapter, for use when recompiling an existing
# Vamp plugin (in C++) into a Javascript module.
#
# This fragment defines all of the actual build targets for the
# module. Your including Makefile should define some or all of the
# following variables before including it:
#
# MODULE_SOURCE - The name of the .cpp file for the module main entry
# point, e.g. vamp-example-plugins.cpp. You must write this file; it
# is not the same as the plugin library entry point used for the
# original Vamp plugin build. You can use the generator program (in
# piper-vamp-js/generator) to generate a preliminary version based on
# metadata reported by your actual plugin library, but it will usually
# still need some editing.
#
# MODULE_NAME - The name of the target feature extraction module.
# Typically a camel-cased representation of the Vamp plugin library
# name, like VampExamplePlugins.
#
# PLUGIN_SOURCES - All C++ source files for the plugin library.
#
# PLUGIN_C_SOURCES - Any C source files for the plugin library.
#
# DEFINES - Any additional -D flags for the C/C++ compiler.
#
# INCLUDES - Any additional -I flags for the C/C++ compiler.
#
# EMFLAGS - Any additional Emscripten-specific flags to pass to the
# compiler.

default:        help

help:
		@echo
		@echo "Supported make targets:"
		@echo
		@echo "$$ make em"
		@echo "  - build Javascript module using Emscripten"
		@echo "$$ make test"
		@echo "  - build and run simple load test of Javascript module using node.js"
		@echo "$$ make linux"
		@echo "  - build native-code module on Linux (currently this is mostly for testing)"
		@echo "$$ make clean"
		@echo "  - remove intermediate build files, but not targets"
		@echo "$$ make distclean"
		@echo "  - remove all targets and intermediate build files"
		@echo
		@echo "Note: when switching from one target to another, make clean first"
		@echo

MY_DIR		:= $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

SRC_DIR		:= $(MY_DIR)/src
SDK_DIR		:= $(MY_DIR)/../vamp-plugin-sdk
PIPERCPP_DIR    := $(MY_DIR)/../piper-vamp-cpp

ADAPTER_HEADERS	:= \
		$(SRC_DIR)/PiperAdapter.h \
		$(SRC_DIR)/PiperPluginLibrary.h 
ADAPTER_SOURCES	:= \
		$(SRC_DIR)/PiperPluginLibrary.cpp

SDK_SOURCES	:= \
		$(SDK_DIR)/src/vamp-hostsdk/PluginBufferingAdapter.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/PluginChannelAdapter.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/PluginHostAdapter.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/PluginInputDomainAdapter.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/PluginLoader.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/PluginSummarisingAdapter.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/PluginWrapper.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/RealTime.cpp \
		$(SDK_DIR)/src/vamp-hostsdk/Files.cpp \
		$(SDK_DIR)/src/vamp-sdk/PluginAdapter.cpp \
		$(SDK_DIR)/src/vamp-sdk/RealTime.cpp \
		$(SDK_DIR)/src/vamp-sdk/FFT.cpp

OTHER_SOURCES	:= \
		$(PIPERCPP_DIR)/ext/json11/json11.cpp

NODE            ?= node

EM_MODULE_T_EXT	:= .tmp.js
EM_MODULE_U_EXT	:= .umd.js
EM_MODULE_T	:= $(MODULE_NAME)$(EM_MODULE_T_EXT)
EM_MODULE_U	:= $(MODULE_NAME)$(EM_MODULE_U_EXT)
EM_MODULE_SYMBOL	:= $(MODULE_NAME)Module

WASM_MODULE_T_EXT := _w.js
WASM_MODULE_U_EXT := _w.umd.js
WASM_MODULE_T   := $(MODULE_NAME)$(WASM_MODULE_T_EXT)
WASM_MODULE_U   := $(MODULE_NAME)$(WASM_MODULE_U_EXT)

SO_MODULE_EXT	:= .so
SO_MODULE	:= $(MODULE_NAME)$(SO_MODULE_EXT)

EMFLAGS		:= \
		--memory-init-file 0 \
		-s MODULARIZE=1 \
		-s NO_FILESYSTEM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
	    	-s "EXPORT_NAME='$(EM_MODULE_SYMBOL)'" \
	    	-s "EXPORTED_FUNCTIONS=['_piperRequestJson','_piperProcessRaw','_piperFreeJson']" \
		$(EMFLAGS)

WASMFLAGS       := \
                $(EMFLAGS) \
                -s WASM=1

#                -s "BINARYEN_METHOD='native-wasm'"

CXX_SOURCES	:= $(MODULE_SOURCE) $(ADAPTER_SOURCES) $(PLUGIN_SOURCES) $(SDK_SOURCES) $(OTHER_SOURCES)
C_SOURCES	:= $(PLUGIN_C_SOURCES) $(C_SOURCES)

#OPTFLAGS	?= -g3
OPTFLAGS	?= -O

DEFINES		:= $(DEFINES)
INCLUDES	:= -I$(SRC_DIR) -I$(SDK_DIR) -I$(PIPERCPP_DIR) -I$(PIPERCPP_DIR)/ext $(INCLUDES)

em:	$(EM_MODULE_U)
em:	CXX		:= em++
em:	CC		:= emcc
em:	CXXFLAGS	:= -std=c++11 -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(EMFLAGS) $(INCLUDES)
em:	CFLAGS		:= -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(EMFLAGS) $(INCLUDES)
em:	LDFLAGS		:= $(EMFLAGS)

wasm:   $(WASM_MODULE_U)
wasm:   CXX		:= em++
wasm:	CC		:= emcc
wasm:	CXXFLAGS	:= -std=c++11 -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(WASMFLAGS) $(INCLUDES)
wasm:	CFLAGS		:= -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(WASMFLAGS) $(INCLUDES)
wasm:	LDFLAGS		:= $(WASMFLAGS)

linux:	$(SO_MODULE)
linux:	CXXFLAGS	:= -std=c++11 -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(INCLUDES)
linux:	CFLAGS		:= -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(INCLUDES)
linux:	LDFLAGS		:= -shared -Wl,-Bsymbolic -Wl,-soname=$(SO_MODULE) -Wl,-z,defs -Wl,--version-script=$(SRC_DIR)/piper.map -ldl

OBJDIR          := o

CXX_OBJECTS	:= $(CXX_SOURCES:.cpp=.o)
C_OBJECTS	:= $(C_SOURCES:.c=.o)
ALL_OBJECTS	:= $(CXX_OBJECTS) $(C_OBJECTS)
OBJECTS         := $(addprefix $(OBJDIR)/,$(abspath $(ALL_OBJECTS)))

o/%.o:            %.cpp
		mkdir -p $(dir $@)
		$(CXX) -c $(INCLUDES) $(CXXFLAGS) -o $@ $<

o/%.o:            %.c
		mkdir -p $(dir $@)
		$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

$(EM_MODULE_U):	$(OBJECTS)
		$(CXX) $(OPTFLAGS) -o $(EM_MODULE_T) $^ $(LDFLAGS) && \
		( echo "(function(root,factory){if(typeof define==='function'&&define.amd){define([],factory);}else if(typeof module==='object'&&module.exports){module.exports=factory();}else{root['$(EM_MODULE_SYMBOL)']=factory();}}(this,function(){" > $(EM_MODULE_U) ) && \
		cat $(EM_MODULE_T) >> $(EM_MODULE_U) && \
                rm $(EM_MODULE_T) && \
		( echo ; echo "return $(EM_MODULE_SYMBOL);}));" >> $(EM_MODULE_U) )

$(WASM_MODULE_U): $(OBJECTS)
		$(CXX) $(OPTFLAGS) -o $(WASM_MODULE_T) $^ $(LDFLAGS) && \
		( echo "(function(root,factory){if(typeof define==='function'&&define.amd){define([],factory);}else if(typeof module==='object'&&module.exports){module.exports=factory();}else{root['$(EM_MODULE_SYMBOL)']=factory();}}(this,function(){" > $(WASM_MODULE_U) ) && \
		cat $(WASM_MODULE_T) >> $(WASM_MODULE_U) && \
                rm $(WASM_MODULE_T) && \
		( echo ; echo "return $(EM_MODULE_SYMBOL);}));" >> $(WASM_MODULE_U) )

$(SO_MODULE):	$(OBJECTS)
		$(CXX) -o $@ $^ $(LDFLAGS)

test:		em
		$(NODE) $(MY_DIR)/test/node-load-test.js $(shell pwd)/$(EM_MODULE_U)

test-wasm:		wasm
		$(NODE) $(MY_DIR)/test/node-load-test.js $(shell pwd)/$(WASM_MODULE_U)

clean:
		rm -f $(OBJECTS)

distclean:	clean
		rm -f $(EM_MODULE_T) $(EM_MODULE_U) $(WASM_MODULE_T) $(WASM_MODULE_U) $(SO_MODULE)

