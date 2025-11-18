CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iinclude
PKG_CONFIG ?= pkg-config

OPENCV_CFLAGS := $(shell $(PKG_CONFIG) --cflags opencv4 2>/dev/null)
OPENCV_LIBS   := $(shell $(PKG_CONFIG) --libs opencv4 2>/dev/null)

ifeq ($(strip $(OPENCV_LIBS)),)
$(error OpenCV no encontrado. Instala libopencv-dev)
endif

CXXFLAGS += $(OPENCV_CFLAGS)
LDFLAGS  := $(OPENCV_LIBS)

# Folders
SRC_DIR  := src
OBJ_DIR  := build
BIN_DIR  := bin

# Executable file
TARGET := $(BIN_DIR)/spp

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

# Link executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Mkdir
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Execute
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean run
