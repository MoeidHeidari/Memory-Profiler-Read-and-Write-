# Memory profiler (read and write)
A bunch memory profiler developed in LLVM/Clang and GCC.
## LLVM/Clang
```bash
clang {pluginnName.cc} -o {pluginnName.so} -shared -fPIC `llvm-config --cxxflags` -L`llvm-config --libnames`

```
after this section you will get a *.so file as a shared library
```bash
clang -S -emit-llvm {test_file.c}
```
Load the plugin using opt
```bash
opt -load-pass-plugin=./{pluginnName.so} -passes="myplugin" {test_file.ll} -o {test_file.bc}
```

## GCC
se the Makefile to make the plugin

```bash

make plugin_name={plugin_name}
``
* plugin_name should be path specific
## Adjust the make file
```bash
#set the path of GXX executable files
GCCDIR={GCC executable files path}
#set the name of the plugin with exact path
PLUGIN_NAME=$(plugin_name)
#-----------------------------------------------------------
#set GXX flags
CXX       =  $(GCCDIR)/g++
CXXFLAGS  =  -Wall -fno-rtti
CXXFLAGS  += -Wno-literal-suffix
#find the GXX plugin path
PLUGINDIR =  $(shell $(CXX) -print-file-name=plugin)

#set GXX flags
CXXFLAGS  += -I$(PLUGINDIR)/include
#enable C++ support
LDFLAGS   =  -std=c++11
#-----------------------------------------------------------
# make command to build the plugin
all: $(PLUGIN_NAME).so
#make the shared library file
$(PLUGIN_NAME).so: $(PLUGIN_NAME).o
$(CXX) $(LDFLAGS) -shared -o $@ $<
#-----------------------------------------------------------
#make the plugin object file
$(PLUGIN_NAME).o:
$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<
#-----------------------------------------------------------
#make clean command to delete generated object and shared library files
clean:
rm -f $(PLUGINNAME).o $(PLUGINNAME).so

```
**Example**  
GCCDIR=../../gccdir/installation/bin

to use the plugin
```bash
$(GCCDIR)/g++ -fplugin=plugin_name.so -c test.cc -o test

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.  
let me know if you should have any issue  
moeidheidari@mail.ru

## License
[MIT](https://choosealicense.com/licenses/mit/)
