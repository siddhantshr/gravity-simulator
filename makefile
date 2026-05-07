CXX = clang++
CXXFLAGS = -std=c++23 -Wall -g -fdiagnostics-color=always -I./include -I./include/imgui
LDFLAGS = -L./lib -lglfw3 -lyaml-cpp -limgui -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

.PHONY: all run clean

SRC = src/main.cpp src/glad.c src/physics.cpp src/utils.cpp
BUILD_DIR = build
TARGET = $(BUILD_DIR)/app

all: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)