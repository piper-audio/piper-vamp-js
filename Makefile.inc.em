
ADAPTER_HEADERS	:= PiperAdapter.h PiperPluginLibrary.h 
ADAPTER_SOURCES	:= PiperPluginLibrary.cpp

SDK_DIR		:= ../vamp-plugin-sdk
PIPERCPP_DIR    := ../piper-cpp

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

MODULE_EXT	:= .js
MODULE		:= $(MODULE_NAME)$(MODULE_EXT)
MODULE_SYMBOL	:= $(MODULE_NAME)Module

EMFLAGS		:= \
		--memory-init-file 0 \
		-s MODULARIZE=1 \
		-s NO_FILESYSTEM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
	    	-s EXPORT_NAME="'$(MODULE_SYMBOL)'" \
	    	-s EXPORTED_FUNCTIONS="['_piperRequestJson','_piperProcessRaw','_piperFreeJson']" \
		$(EMFLAGS)

CXX_SOURCES	:= $(MODULE_SOURCE) $(ADAPTER_SOURCES) $(PLUGIN_SOURCES) $(SDK_SOURCES) $(OTHER_SOURCES)
LDFLAGS		:= $(EMFLAGS)

CXX		:= em++
CC		:= emcc

#OPTFLAGS	:= -g3
OPTFLAGS	:= -O3 -ffast-math

DEFINES		:= $(DEFINES)

INCPATH		:= -I$(SDK_DIR) -I$(PIPERCPP_DIR) $(INCPATH)

CXXFLAGS	:= -std=c++11 -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(EMFLAGS) $(INCPATH)
CFLAGS		:= -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS) $(EMFLAGS) $(INCPATH)

CXX_OBJECTS	:= $(CXX_SOURCES:.cpp=.o)
C_OBJECTS	:= $(C_SOURCES:.c=.o)
OBJECTS		:= $(CXX_OBJECTS) $(C_OBJECTS)

all:		$(MODULE)

$(MODULE):	$(OBJECTS)
		$(CXX) $(OPTFLAGS) $(EMFLAGS) -o $(MODULE) $(OBJECTS) $(MODULE_LDFLAGS) && \
		( echo "if (typeof process === 'object') module.exports=$(MODULE_SYMBOL);" >> $(MODULE) )

clean:
		rm -f $(MODULE) $(OBJECTS)
