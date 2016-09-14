
ADAPTER_HEADERS	:= VamPipeAdapter.h VamPipePluginLibrary.h 
ADAPTER_SOURCES	:= VamPipePluginLibrary.cpp

SDK_DIR		:= ../vamp-plugin-sdk

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
		../json/json11/json11.cpp

MODULE_EXT	:= .js
MODULE		:= $(MODULE_NAME)$(MODULE_EXT)
MODULE_SYMBOL	:= $(MODULE_NAME)Module

EMFLAGS		:= \
		--memory-init-file 0 \
		-s MODULARIZE=1 \
		-s NO_FILESYSTEM=1 \
		-s ERROR_ON_UNDEFINED_SYMBOLS=1 \
		-s DISABLE_EXCEPTION_CATCHING=0 \
	    	-s EXPORT_NAME="'$(MODULE_SYMBOL)'" \
	    	-s EXPORTED_FUNCTIONS="['_vampipeRequestJson','_vampipeProcessRaw','_vampipeFreeJson']"

SOURCES		:= $(MODULE_SOURCE) $(ADAPTER_SOURCES) $(PLUGIN_SOURCES) $(OTHER_SOURCES)
LDFLAGS		:= $(EMFLAGS)

CXX		:= em++

#OPTFLAGS	:= -g3
OPTFLAGS	:= -O3 -ffast-math

DEFINES		:= -DSINGLE_PRECISION_FFT $(DEFINES)

CXXFLAGS	:= -std=c++11 -fPIC -Wall -Wextra $(DEFINES) $(OPTFLAGS)

INCPATH		:= -I$(SDK_DIR) -I.. -I../json $(INCPATH)

all:		$(MODULE)

$(MODULE):	$(SOURCES) $(ADAPTER_HEADERS) $(SDK_SOURCES)
		$(CXX) $(CXXFLAGS) $(EMFLAGS) $(INCPATH) -o $(MODULE) \
		       $(SOURCES) $(SDK_SOURCES) $(MODULE_LDFLAGS) && \
		( echo "module.exports=$(MODULE_SYMBOL);" >> $(MODULE) )

clean:
		rm -f $(MODULE)
