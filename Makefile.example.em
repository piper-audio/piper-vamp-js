
MODULE_NAME	:= VampExamplePlugins

EXAMPLE_DIR	:= ../vamp-plugin-sdk/examples

PLUGIN_SOURCES	:= \
		$(EXAMPLE_DIR)/ZeroCrossing.cpp \
		$(EXAMPLE_DIR)/SpectralCentroid.cpp \
		$(EXAMPLE_DIR)/PercussionOnsetDetector.cpp \
		$(EXAMPLE_DIR)/FixedTempoEstimator.cpp \
		$(EXAMPLE_DIR)/AmplitudeFollower.cpp \
		$(EXAMPLE_DIR)/PowerSpectrum.cpp

MODULE_SOURCE	:= examples/vamp-example-plugins.cpp

include Makefile.inc
