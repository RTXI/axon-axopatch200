PLUGIN_NAME = axon_axopatch200

HEADERS = axon_axopatch200_commander.h \
          axon_axopatch200_commanderUI.h

SOURCES = axon_axopatch200_commander.cpp \
          axon_axopatch200_commanderUI.cpp \
          moc_axon_axopatch200_commander.cpp \
          moc_axon_axopatch200_commanderUI.cpp 

LIBS = 

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile
