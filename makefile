CXX = clang++
CC = clang
CXXFLAGS = -std=c++23 -Wall -g -fdiagnostics-color=always -I./include -I./include/imgui -I./include/imgui/backends
CFLAGS = -Wall -g -I./include

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
	LDFLAGS = -L./lib/darwin -lglfw3 -lyaml-cpp -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
	LDFLAGS = -L./lib/linux -lglfw3 -lyaml-cpp -lGL -lX11 -lpthread -ldl -lm
endif

.PHONY: all run clean

CXX_SRC = src/main.cpp src/physics.cpp src/utils.cpp
C_SRC = src/glad.c
IMGUI_SRC = $(wildcard external/imgui/*.cpp)
BUILD_DIR = build
TARGET = $(BUILD_DIR)/app
IMGUI_OBJECTS = $(patsubst external/imgui/%.cpp, $(BUILD_DIR)/imgui/%.o, $(IMGUI_SRC))

all: $(BUILD_DIR) $(BUILD_DIR)/glad.o $(IMGUI_OBJECTS)
	$(CXX) $(CXXFLAGS) $(CXX_SRC) $(BUILD_DIR)/glad.o $(IMGUI_OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/glad.o: src/glad.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) src/glad.c -o $(BUILD_DIR)/glad.o

$(BUILD_DIR)/imgui/%.o: external/imgui/%.cpp | $(BUILD_DIR)/imgui
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui:
	mkdir -p $(BUILD_DIR)/imgui

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)