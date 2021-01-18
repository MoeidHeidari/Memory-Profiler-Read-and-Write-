GCCDIR = /home/andromeda/gcc-plugins/gcc-install/bin
PLUGINNAME=$(PluginName)
CXX = $(GCCDIR)/g++
PLUGINDIR=/home/andromeda/gcc-plugins/gcc-install/lib/gcc/x86_64-pc-linux-gnu/10.2.0/plugin
# Flags for the C++ compiler: C++11 and all the warnings
CXXFLAGS = -Wall -fno-rtti
# Workaround for an issue of -std=c++11 and the current GCC headers
CXXFLAGS += -Wno-literal-suffix

# Determine the plugin-dir and add it to the flags
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

LDFLAGS = -std=c++11

# top level goal: build our plugin as a shared library
all: $(PLUGINNAME).so

$(PLUGINNAME).so: $(PLUGINNAME).o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

$(PLUGINNAME).o : $(PLUGINNAME).cc
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f $(PLUGINNAME).o $(PLUGINNAME).so

check: warn_unused.so test.cc
	$(CXX) -fplugin=./warn_unused.so -c test.cc -o /dev/null 2> test.dot
	dot -Tpdf test.dot > test.pdf
	xdg-open test.pdf

.PHONY: all clean check
