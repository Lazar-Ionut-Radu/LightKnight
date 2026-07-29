# Compiler and flags
CXX := g++

COMMON_CXXFLAGS := \
	-std=c++20 \
	-Wall \
	-Wextra \
	-Iinclude \
	-fconstexpr-ops-limit=200000000

MODE ?= debug

ifeq ($(MODE),release)
	BUILD_DIR := build/release
	CXXFLAGS := $(COMMON_CXXFLAGS) -O3 -DNDEBUG -march=native
else
	BUILD_DIR := build/debug
	CXXFLAGS := $(COMMON_CXXFLAGS) -Og -g3
endif

LDFLAGS :=
TEST_LDFLAGS := -lCatch2Main -lCatch2
TEST_ARGS ?=

# Source files
SRC := $(wildcard src/*.cc)
SRC_ENGINE := $(wildcard src/*.cc)
SRC_TEST := $(filter-out src/main.cc,$(SRC))
TEST_SRC := $(wildcard test/*.cc)

# Object files
OBJ_ENGINE := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(SRC_ENGINE))
OBJ_TEST := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(SRC_TEST) $(TEST_SRC))

# Executables
ENGINE_EXE := $(BUILD_DIR)/lightknight
TEST_EXE := $(BUILD_DIR)/tests

.PHONY: all debug release test test-list test-verbose test-help clean magics perft-debug

all: $(ENGINE_EXE) $(TEST_EXE)

debug:
	$(MAKE) MODE=debug all

release:
	$(MAKE) MODE=release all

# Compile .cc -> .o (create subdirs automatically)
$(BUILD_DIR)/%.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link engine
$(ENGINE_EXE): $(OBJ_ENGINE)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

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

benchmark:
	$(MAKE) \
		MODE=release \
		TEST_ARGS='"[!benchmark]"' \
		test

# Clean build directory
clean:
	rm -rf build

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

# -------------------------------------------------------------------
# Perft debug executable
# -------------------------------------------------------------------
PERFT_DEBUG_SRC := helpers/perft_debug/perft_debug.cc
PERFT_DEBUG_OBJ := $(BUILD_DIR)/helpers/perft_debug/perft_debug.o

PERFT_DEBUG_ENGINE_SRC := $(filter-out src/main.cc,$(SRC))
PERFT_DEBUG_ENGINE_OBJ := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(PERFT_DEBUG_ENGINE_SRC))
PERFT_DEBUG_EXE := $(BUILD_DIR)/perft-debug

$(PERFT_DEBUG_EXE) : $(PERFT_DEBUG_OBJ) $(PERFT_DEBUG_ENGINE_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

FEN ?= 
DEPTH ?= 1
TRACE ?= 0

PERFT_DEBUG_ARGS := "$(FEN)" $(DEPTH)
ifneq ($(filter 1 true yes,$(TRACE)),)
	PERFT_DEBUG_ARGS := --trace $(PERFT_DEBUG_ARGS)
endif

perft-debug: $(PERFT_DEBUG_EXE)
	./$(PERFT_DEBUG_EXE) $(PERFT_DEBUG_ARGS)