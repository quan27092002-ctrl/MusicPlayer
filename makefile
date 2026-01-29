# ==========================================
# PROJECT: S32K_MediaPlayer
# BUILD SYSTEM: GNU Make
# AUTHOR: Architecture Team
# ==========================================

# Enable parallel compilation (auto-detect CPU cores)
MAKEFLAGS += -j$(shell nproc)

# 1. Compiler Settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Isrc -Isrc/utils -Isrc/model -Isrc/controller -Isrc/view -Isrc/view/imgui
CXXFLAGS += $(shell pkg-config --cflags sdl2 SDL2_mixer)

# Linker flags
LDFLAGS  = -lgtest -lgtest_main -pthread
LDFLAGS += $(shell pkg-config --libs sdl2 SDL2_mixer)

# 2. Project Directories
SRC_DIR = src
TEST_DIR = test
BUILD_DIR = build
IMGUI_DIR = src/view/imgui

# 3. Source Files
# Core sources
SRC_SRCS = $(SRC_DIR)/utils/Buffer.cpp \
           $(SRC_DIR)/utils/Logger.cpp \
           $(SRC_DIR)/model/MediaFile.cpp \
           $(SRC_DIR)/model/PlayerState.cpp \
           $(SRC_DIR)/controller/AudioPlayer.cpp \
           $(SRC_DIR)/controller/SerialManager.cpp \
           $(SRC_DIR)/controller/AppController.cpp

# ImGui sources
IMGUI_SRCS = $(IMGUI_DIR)/imgui.cpp \
             $(IMGUI_DIR)/imgui_draw.cpp \
             $(IMGUI_DIR)/imgui_tables.cpp \
             $(IMGUI_DIR)/imgui_widgets.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

# View sources
VIEW_SRCS = $(SRC_DIR)/view/ImGuiView.cpp

# Test sources
TEST_SRCS = $(TEST_DIR)/testThreadSafeQueue.cpp \
            $(TEST_DIR)/testBuffer.cpp \
            $(TEST_DIR)/testLogger.cpp \
            $(TEST_DIR)/testMediaFile.cpp \
            $(TEST_DIR)/testPlayerState.cpp \
            $(TEST_DIR)/testAudioPlayer.cpp \
            $(TEST_DIR)/testSerialManager.cpp \
            $(TEST_DIR)/testAppController.cpp

# 4. Object Files
SRC_OBJS = $(SRC_SRCS:%.cpp=$(BUILD_DIR)/%.o)
IMGUI_OBJS = $(IMGUI_SRCS:%.cpp=$(BUILD_DIR)/%.o)
VIEW_OBJS = $(VIEW_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS = $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
ALL_TEST_OBJS = $(SRC_OBJS) $(TEST_OBJS)
ALL_APP_OBJS = $(SRC_OBJS) $(IMGUI_OBJS) $(VIEW_OBJS)

# 5. Executable Names
TEST_TARGET = $(BUILD_DIR)/unit_tests
APP_TARGET = $(BUILD_DIR)/media_player

# ==========================================
# RULES
# ==========================================

# Default target: Build the test executable
all: $(TEST_TARGET)

# Build the application (with GUI)
app: $(APP_TARGET)

# Link test executable
$(TEST_TARGET): $(ALL_TEST_OBJS)
	@mkdir -p $(dir $@)
	@echo "Linking $@"
	$(CXX) $(ALL_TEST_OBJS) -o $@ $(LDFLAGS)

# Link application executable
$(APP_TARGET): $(ALL_APP_OBJS) $(BUILD_DIR)/src/main.o
	@mkdir -p $(dir $@)
	@echo "Linking $@"
	$(CXX) $(ALL_APP_OBJS) $(BUILD_DIR)/src/main.o -o $@ $(LDFLAGS)

# Compile: .cpp -> .o
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run Tests
run: all
	@echo "Running Tests..."
	./$(TEST_TARGET)

# Run Application
run_app: app
	@echo "Running Media Player..."
	./$(APP_TARGET)

# Clean
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

.PHONY: all app clean run run_app