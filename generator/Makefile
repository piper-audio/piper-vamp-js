
VAMPSDK_DIR	:= ../../vamp-plugin-sdk
PIPERCPP_DIR    := ../../piper-vamp-cpp
PIPER_DIR	:= ../../piper

INCFLAGS	:= -I$(PIPERCPP_DIR) -I$(PIPERCPP_DIR)/ext -I$(PIPERCPP_DIR)/ext/serd -I$(PIPERCPP_DIR)/ext/sord -I$(VAMPSDK_DIR) -I/usr/local/include

OBJECTS		:= ./generator.o $(PIPERCPP_DIR)/ext/sord/sord-single.o

CXXFLAGS	:= -Wall -Wextra -g3 -std=c++11 $(INCFLAGS)
CFLAGS		:= -Wall -g3 -std=c99 $(INCFLAGS)

LDFLAGS		:= $(VAMPSDK_DIR)/libvamp-hostsdk.a

LDFLAGS		+= -ldl

all:		../bin/piper-vamp-stub-generator

../bin/piper-vamp-stub-generator:	$(OBJECTS)
		$(CXX) $^ -o $@ $(LDFLAGS)

clean:
		rm -f $(OBJECTS)
