G++ := g++
LD  := g++
WINDRES := windres

CPP_FLAGS := -std=c++20 -O2 $(shell wx-config --cxxflags)
LD_FLAGS :=

INCLUDES := -Isrc
LIBS := $(shell wx-config --libs stc,aui,core,base)

TARGET := code-editor
SOURCES := $(shell find src -name '*.cpp')
OBJECTS := $(SOURCES:src/%.cpp=build/%.o)

ifeq ($(OS),Windows_NT)
	TARGET := code-editor.exe
	RC_SOURCE := src/resources.rc
	RC_OBJECT := build/resources_rc.o

	RC_FLAGS := $(shell wx-config --cppflags)
endif

all: $(TARGET)

clean:
	rm -rf $(TARGET) build

$(TARGET): $(OBJECTS) $(RC_OBJECT)
	@mkdir -p $(dir $@)
	$(LD) $(LD_FLAGS) $(OBJECTS) $(RC_OBJECT) -o $@ $(LIBS)

ifdef RC_OBJECT
$(RC_OBJECT): $(RC_SOURCE)
	@mkdir -p $(dir $@)
	windres $(RC_FLAGS) $< -o $@
endif

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(G++) $(CPP_FLAGS) $(INCLUDES) -c $< -o $@