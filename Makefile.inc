
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
PIPERCPP_DIR    := $(MY_DIR)/../piper-cpp

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
		$(PIPERCPP_DIR)/json11/json11.cpp

EM_MODULE_EXT	:= .js
EM_MODULE	:= $(MODULE_NAME)$(EM_MODULE_EXT)
EM_MODULE_SYMBOL	:= $(MODULE_NAME)Module

SO_MODULE_EXT	:= .so
SO_MODULE	:= $(MODULE_NAME)$(SO_MODULE_EXT)

EMFLAGS		:= \
		--memory-init-file 0 \
		-s MODULARIZE=1 \
		-s NO_FILESYSTEM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
	    	-s EXPORT_NAME="'$(EM_MODULE_SYMBOL)'" \
	    	-s EXPORTED_FUNCTIONS="['_piperRequestJson','_piperProcessRaw','_piperFreeJson']" \
		$(EMFLAGS)

CXX_SOURCES	:= $(MODULE_SOURCE) $(ADAPTER_SOURCES) $(PLUGIN_SOURCES) $(SDK_SOURCES) $(OTHER_SOURCES)
C_SOURCES	:= $(PLUGIN_C_SOURCES) $(C_SOURCES)

#OPTFLAGS	?= -g3
OPTFLAGS	?= -O

DEFINES		:= $(DEFINES)
INCLUDES	:= -I$(SRC_DIR) -I$(SDK_DIR) -I$(PIPERCPP_DIR) $(INCLUDES)

em:	$(EM_MODULE)
em:	CXX		:= em++
em:	CC		:= emcc
em:	CXXFLAGS	:= -std=c++11 -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(EMFLAGS) $(INCLUDES)
em:	CFLAGS		:= -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(EMFLAGS) $(INCLUDES)
em:	LDFLAGS		:= $(EMFLAGS)

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

$(EM_MODULE):	$(OBJECTS)
		$(CXX) $(OPTFLAGS) -o $@ $^ $(LDFLAGS) && \
		( echo "if (typeof process === 'object') module.exports=$(EM_MODULE_SYMBOL);" >> $(EM_MODULE) )

$(SO_MODULE):	$(OBJECTS)
		$(CXX) -o $@ $^ $(LDFLAGS)

test:		em
		node $(MY_DIR)/test/node-load-test.js $(shell pwd)/$(EM_MODULE)

clean:
		rm -f $(OBJECTS)

distclean:	clean
		rm -f $(EM_MODULE) $(SO_MODULE)
