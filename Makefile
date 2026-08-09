# Compiler and flags
CXX := g++

COMMON_CXXFLAGS := \
	-std=c++20 \
	-Wall \
	-Wextra \
	-Iinclude \
	-MMD \
	-MP \
	-fconstexpr-ops-limit=200000000

MODE ?= debug
NATIVE ?= 0

ifeq ($(MODE),debug)
	BUILD_DIR := build/debug
	CXXFLAGS := $(COMMON_CXXFLAGS) -Og -g3
else ifeq ($(MODE),release)
	BUILD_DIR := build/release
	CXXFLAGS := $(COMMON_CXXFLAGS) -O3 -DNDEBUG

	ifeq ($(NATIVE),1)
		CXXFLAGS += -march=native
	endif
else
	$(error Invalid MODE '$(MODE)'; expected debug or release)
endif

LDFLAGS :=
LDLIBS :=
TEST_LDLIBS := -lCatch2Main -lCatch2
TEST_ARGS ?=

# -------------------------------------------------------------------
# Source files
# -------------------------------------------------------------------

ENGINE_SRC := $(wildcard src/*.cc)
ENGINE_LIB_SRC := $(filter-out src/main.cc,$(ENGINE_SRC))
TEST_SRC := $(wildcard test/*.cc)

# -------------------------------------------------------------------
# Object files
# -------------------------------------------------------------------

OBJ_ENGINE := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(ENGINE_SRC))
OBJ_TEST := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(ENGINE_LIB_SRC) $(TEST_SRC))

# -------------------------------------------------------------------
# Executables
# -------------------------------------------------------------------

ENGINE_EXE := $(BUILD_DIR)/lightknight
TEST_EXE := $(BUILD_DIR)/tests

.PHONY: \
	all \
	debug \
	release \
	tests \
	test \
	benchmark \
	clean \
	magics \
	perft-debug

all: $(ENGINE_EXE)

debug:
	$(MAKE) MODE=debug all

release:
	$(MAKE) MODE=release all

# Compile .cc -> .o and create subdirectories automatically.
$(BUILD_DIR)/%.o: %.cc Makefile
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link engine.
$(ENGINE_EXE): $(OBJ_ENGINE)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Link tests.
$(TEST_EXE): $(OBJ_TEST)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ \
		$(LDLIBS) $(TEST_LDLIBS)

# -------------------------------------------------------------------
# Tests
# -------------------------------------------------------------------

# Compile tests without running.
tests: $(TEST_EXE)

# Compile and run tests.
test: $(TEST_EXE)
	./$(TEST_EXE) $(TEST_ARGS)

# Compule and run benchmarks.
benchmark:
	$(MAKE) \
		MODE=release \
		TEST_ARGS='"[!benchmark]"' \
		test

# -------------------------------------------------------------------
# Clean
# -------------------------------------------------------------------

clean:
	rm -rf build

# -------------------------------------------------------------------
# Magics generator executable
# -------------------------------------------------------------------

MAGICS_SRC := tools/magics-gen/magics_gen.cc
MAGICS_OBJ := $(BUILD_DIR)/tools/magics-gen/magics_gen.o
MAGICS_EXE := $(BUILD_DIR)/magics-gen

$(MAGICS_EXE): $(MAGICS_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

magics: $(MAGICS_EXE)
	./$(MAGICS_EXE)

# -------------------------------------------------------------------
# Perft debug executable
# -------------------------------------------------------------------

PERFT_DEBUG_SRC := tools/perft_debug/perft_debug.cc
PERFT_DEBUG_OBJ := $(BUILD_DIR)/tools/perft_debug/perft_debug.o

PERFT_DEBUG_ENGINE_SRC := $(ENGINE_LIB_SRC)
PERFT_DEBUG_ENGINE_OBJ := \
	$(patsubst %.cc,$(BUILD_DIR)/%.o,$(PERFT_DEBUG_ENGINE_SRC))

PERFT_DEBUG_EXE := $(BUILD_DIR)/perft-debug

$(PERFT_DEBUG_EXE): $(PERFT_DEBUG_OBJ) $(PERFT_DEBUG_ENGINE_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

perft-debug: $(PERFT_DEBUG_EXE)

# -------------------------------------------------------------------
# Header dependency files
# -------------------------------------------------------------------

ALL_OBJ := $(sort \
	$(OBJ_ENGINE) \
	$(OBJ_TEST) \
	$(MAGICS_OBJ) \
	$(PERFT_DEBUG_OBJ) \
	$(PERFT_DEBUG_ENGINE_OBJ))

DEPS := $(ALL_OBJ:.o=.d)

-include $(DEPS)