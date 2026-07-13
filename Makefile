# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude -fconstexpr-ops-limit=200000000
BUILD_DIR := build
TEST_LDFLAGS := -lCatch2Main -lCatch2

# Source files
SRC := $(wildcard src/*.cc)
SRC_ENGINE := $(wildcard src/*.cc) # engine files
SRC_TEST := $(filter-out src/main.cc,$(SRC))  # engine files for tests (exclude main.cc)
TEST_SRC := $(wildcard test/*.cc)      # Catch2 tests + test main
TEST_ARGS ?=

# Object files
OBJ_ENGINE := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(SRC_ENGINE))
OBJ_TEST := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(SRC_TEST) $(TEST_SRC))

# Executables
ENGINE_EXE := $(BUILD_DIR)/lightknight
TEST_EXE := $(BUILD_DIR)/tests

# Default target: build engine and tests
all: $(ENGINE_EXE) $(TEST_EXE)

# Compile .cc -> .o (create subdirs automatically)
$(BUILD_DIR)/%.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link engine
$(ENGINE_EXE): $(OBJ_ENGINE)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Link tests
$(TEST_EXE): $(OBJ_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(TEST_LDFLAGS)

# Run tests
test: $(TEST_EXE)
	./$(TEST_EXE) $(TEST_ARGS)

test-list: $(TEST_EXE)
	./$(TEST_EXE) --list-tests

test-verbose: $(TEST_EXE)
	./$(TEST_EXE) --success

test-help: $(TEST_EXE)
	./$(TEST_EXE) --help

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)

# -------------------------------------------------------------------
# Magics generator executable
# -------------------------------------------------------------------
MAGICS_SRC := helpers/magics-gen/magics_gen.cc
MAGICS_OBJ := $(BUILD_DIR)/helpers/magics-gen/magics_gen.o
MAGICS_EXE := $(BUILD_DIR)/magics-gen

$(MAGICS_OBJ): $(MAGICS_SRC)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(MAGICS_EXE): $(MAGICS_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

magics: $(MAGICS_EXE)
	./$(MAGICS_EXE)
