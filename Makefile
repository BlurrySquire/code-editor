G++ := g++
LD  := g++

CPP_FLAGS := -std=c++20 -O2 $(shell wx-config --cxxflags)
LD_FLAGS :=

INCLUDES := -Isrc
LIBS := $(shell wx-config --libs stc,aui,core,base)

TARGET := code-editor

SOURCES := $(shell find src -name '*.cpp')
OBJECTS := $(SOURCES:src/%.cpp=build/%.o)

clean:
	rm -rf $(TARGET) build

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) $(LD_FLAGS) $(LIBS) $(OBJECTS) -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(G++) $(CPP_FLAGS) $(INCLUDES) -c $< -o $@