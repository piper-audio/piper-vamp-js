
VAMPSDK_DIR	:= ../../vamp-plugin-sdk
PIPERCPP_DIR    := ../../piper-vamp-cpp
PIPER_DIR	:= ../../piper

##!!! review
INCFLAGS	:= -I$(PIPERCPP_DIR)/ext -I/usr/include/sord-0 -I/usr/include/serd-0 -I$(VAMPSDK_DIR) -I$(PIPERCPP_DIR) -I/usr/local/include

CXXFLAGS	:= -Wall -Wextra -g3 -std=c++11 $(INCFLAGS)

LDFLAGS		:= $(VAMPSDK_DIR)/libvamp-hostsdk.a -lsord-0 -lserd-0

LDFLAGS		+= -ldl

all:		../bin/piper-vamp-stub-generator

../bin/piper-vamp-stub-generator:	./generator.cpp
		$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

