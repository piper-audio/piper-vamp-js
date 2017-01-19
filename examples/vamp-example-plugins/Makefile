
PIPER_VAMP_JS_DIR	:= ../..
PLUGIN_SRC_DIR		:= ../../../vamp-plugin-sdk/examples

MODULE_NAME		:= VampExamplePlugins
MODULE_SOURCE		:= vamp-example-plugins.cpp

SOURCE_FILES	:= \
		ZeroCrossing.cpp \
		SpectralCentroid.cpp \
		PercussionOnsetDetector.cpp \
		FixedTempoEstimator.cpp \
		AmplitudeFollower.cpp \
		PowerSpectrum.cpp

PLUGIN_SOURCES	:= $(addprefix $(PLUGIN_SRC_DIR)/,$(SOURCE_FILES))

INCLUDES	:= -I$(PLUGIN_SRC_DIR)

include $(PIPER_VAMP_JS_DIR)/Makefile.inc
