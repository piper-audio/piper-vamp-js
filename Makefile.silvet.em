
MODULE_NAME	:= Silvet

SILVET_DIR	:= ../silvet

SRC_DIR		:= $(SILVET_DIR)/src
CQ_DIR	     	:= $(SILVET_DIR)/constant-q-cpp
CQSRC_DIR     	:= $(SILVET_DIR)/constant-q-cpp/src
KFFT_DIR  	:= $(SILVET_DIR)/constant-q-cpp/src/ext/kissfft
BQVEC_DIR     	:= $(SILVET_DIR)/bqvec
FD_DIR	     	:= $(SILVET_DIR)/flattendynamics

INCPATH		:= -I$(SRC_DIR) -I$(CQ_DIR) -I$(CQ_DIR)/cq -I$(KFFT_DIR) -I$(KFFT_DIR)/tools -I$(SILVET_DIR) -I$(FD_DIR) -I$(BQVEC_DIR)

EMFLAGS		:= -s TOTAL_MEMORY=100000000

DEFINES		:= -Dkiss_fft_scalar=double
 
PLUGIN_SOURCES 	:= \
		$(SRC_DIR)/Silvet.cpp \
		$(SRC_DIR)/EM.cpp \
		$(SRC_DIR)/Instruments.cpp \
		$(SRC_DIR)/LiveInstruments.cpp

BQVEC_SOURCES	:= \
		$(BQVEC_DIR)/src/Allocators.cpp

FD_SOURCES	:= \
		$(FD_DIR)/flattendynamics-ladspa.cpp

CQ_SOURCES	:= \
		$(CQSRC_DIR)/CQKernel.cpp \
		$(CQSRC_DIR)/ConstantQ.cpp \
		$(CQSRC_DIR)/CQSpectrogram.cpp \
		$(CQSRC_DIR)/CQInverse.cpp \
		$(CQSRC_DIR)/Chromagram.cpp \
		$(CQSRC_DIR)/Pitch.cpp \
		$(CQSRC_DIR)/dsp/FFT.cpp \
		$(CQSRC_DIR)/dsp/KaiserWindow.cpp \
		$(CQSRC_DIR)/dsp/MathUtilities.cpp \
		$(CQSRC_DIR)/dsp/Resampler.cpp \
		$(CQSRC_DIR)/dsp/SincWindow.cpp

KFFT_SOURCES    := \
		$(KFFT_DIR)/kiss_fft.c \
		$(KFFT_DIR)/tools/kiss_fftr.c

PLUGIN_SOURCES	:= $(PLUGIN_SOURCES) $(BQVEC_SOURCES) $(FD_SOURCES) $(CQ_SOURCES)

PLUGIN_C_SOURCES   := $(KFFT_SOURCES)

MODULE_SOURCE	:= silvet.cpp

include Makefile.inc.em
