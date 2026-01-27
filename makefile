# ==========================================
# PROJECT: S32K_MediaPlayer
# BUILD SYSTEM: GNU Make
# AUTHOR: Architecture Team
# ==========================================

# 1. Compiler Settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Isrc -Isrc/utils
LDFLAGS  = -lgtest -lgtest_main -pthread

# 2. Project Directories
SRC_DIR = src
TEST_DIR = test
BUILD_DIR = build

# 3. Source Files
# Lưu ý: Vì các file trong src/utils hiện tại là Template (.h only)
# nên ta chưa cần compile file .cpp nào từ src cả.
# Chỉ compile file test.
TEST_SRCS = $(TEST_DIR)/testThreadSafeQueue.cpp

# 4. Object Files
# Tự động đổi đuôi .cpp thành .o và đặt vào thư mục build
TEST_OBJS = $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)

# 5. Executable Name
TEST_TARGET = $(BUILD_DIR)/unit_tests

# ==========================================
# RULES
# ==========================================

# Default target: Build the test executable
all: $(TEST_TARGET)

# Link: Tạo file chạy từ các file .o
$(TEST_TARGET): $(TEST_OBJS)
	@mkdir -p $(dir $@)
	@echo "Linking $@"
	$(CXX) $(TEST_OBJS) -o $@ $(LDFLAGS)

# Compile: Tạo file .o từ file .cpp
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run Test
run: all
	@echo "Running Tests..."
	./$(TEST_TARGET)

# Clean: Xóa sạch file build
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

.PHONY: all clean run