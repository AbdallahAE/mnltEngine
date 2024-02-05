CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

BUILD = MoonLight

TARGET = main.cpp mnlt/*.cpp

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))


all: $(TARGET) $(vertObjFiles) $(fragObjFiles)
	g++ $(CFLAGS) -o $(BUILD) $(TARGET) $(LDFLAGS)

%.spv: %
	glslc $< -o $@

run: $(BUILD)
	./$(BUILD)

clean:
	rm -f $(BUILD)
	rm -f shaders/*.spv