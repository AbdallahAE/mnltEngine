BUILD = MoonLight
TARGET = main.cpp mnlt/*.cpp
CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

all: $(TARGET)
	g++ $(CFLAGS) -o $(BUILD) $(TARGET) $(LDFLAGS)

run: $(BUILD)
	./$(BUILD)

clean:
	rm -f $(BUILD)