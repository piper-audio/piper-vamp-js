
MODULE_NAME	:= VampTestPlugin

TESTPLUGIN_DIR	:= ../../vamp-test-plugin

INCPATH		:= -I$(TESTPLUGIN_DIR)

PLUGIN_SOURCES	:= \
		$(TESTPLUGIN_DIR)/VampTestPlugin.cpp

MODULE_SOURCE	:= vamp-test-plugin.cpp

include Makefile.inc.em
