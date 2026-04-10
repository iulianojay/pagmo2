############################################################
####### An AI wrote this file so it's probably wrong #######
############################################################

# Pagmo2 Makefile - CMake wrapper
# Scientific library for massively parallel optimization

# Build configuration
CMAKE ?= $(shell which cmake)
BUILD_DIR = build/$(shell echo $(BUILD_TYPE) | tr '[:upper:]' '[:lower:]')

# Build-type shortcut targets (debug, release, relwithdebinfo, minsizerel)
# When one of these is used, remaining goals are forwarded to a recursive make
# with BUILD_TYPE set, and suppressed in the outer invocation.
_BT_TARGETS = debug release relwithdebinfo minsizerel
_BT_GOAL := $(filter $(_BT_TARGETS),$(MAKECMDGOALS))
ifneq ($(_BT_GOAL),)
$(filter-out $(_BT_GOAL),$(_BT_TARGETS) $(MAKECMDGOALS)): ;
endif
# Shared CPM source cache so all build types reuse the same downloaded deps
DEPS_CACHE_DIR = $(CURDIR)/build/_deps
CMAKE = /home/jay/projects/astrea/.venv/bin/cmake
CTEST ?= ctest
MAKE = make
INSTALL_PREFIX = ~/.local
NUM_JOBS = $(shell nproc)

# Build options (can be overridden via environment or command line)
PAGMO_BUILD_TESTS ?= ON
PAGMO_BUILD_BENCHMARKS ?= OFF
PAGMO_BUILD_TUTORIALS ?= OFF
PAGMO_WITH_EIGEN3 ?= OFF
PAGMO_WITH_NLOPT ?= OFF
PAGMO_WITH_IPOPT ?= OFF
PAGMO_WITH_MPI ?= OFF
PAGMO_ENABLE_IPO ?= OFF
PAGMO_BUILD_STATIC_LIBRARY ?= OFF

# CMake configuration flags
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
              -DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
              -DCPM_SOURCE_CACHE=$(DEPS_CACHE_DIR) \
              -DPAGMO_BUILD_TESTS=$(PAGMO_BUILD_TESTS) \
              -DPAGMO_BUILD_BENCHMARKS=$(PAGMO_BUILD_BENCHMARKS) \
              -DPAGMO_BUILD_TUTORIALS=$(PAGMO_BUILD_TUTORIALS) \
              -DPAGMO_WITH_EIGEN3=$(PAGMO_WITH_EIGEN3) \
              -DPAGMO_WITH_NLOPT=$(PAGMO_WITH_NLOPT) \
              -DPAGMO_WITH_IPOPT=$(PAGMO_WITH_IPOPT) \
              -DPAGMO_WITH_MPI=$(PAGMO_WITH_MPI) \
              -DPAGMO_ENABLE_IPO=$(PAGMO_ENABLE_IPO) \
              -DPAGMO_BUILD_STATIC_LIBRARY=$(PAGMO_BUILD_STATIC_LIBRARY)

# Default target
.PHONY: all
all: build

# Help target
.PHONY: help
help:
	@echo "Pagmo2 Build System"
	@echo "==================="
	@echo ""
	@echo "Main targets:"
	@echo "  build          - Build the library (default)"
	@echo "  debug          - Build debug version"
	@echo "  release        - Build release version (default)"
	@echo "  test           - Run tests"
	@echo "  install        - Install the library"
	@echo "  clean          - Clean build files"
	@echo "  distclean      - Complete cleanup"
	@echo ""
	@echo "Documentation:"
	@echo "  docs           - Build documentation"
	@echo "  docs-serve     - Serve documentation locally"
	@echo ""
	@echo "Development:"
	@echo "  benchmarks     - Build and run benchmarks"
	@echo "  tutorials      - Build tutorials"
	@echo "  format         - Format source code"
	@echo "  lint           - Run static analysis"
	@echo ""
	@echo "Configuration options (set via environment or make var=value):"
	@echo "  BUILD_TYPE              - Debug or Release (default: $(BUILD_TYPE))"
	@echo "  INSTALL_PREFIX          - Installation prefix (default: $(INSTALL_PREFIX))"
	@echo "  PAGMO_BUILD_TESTS       - Build tests (default: $(PAGMO_BUILD_TESTS))"
	@echo "  PAGMO_BUILD_BENCHMARKS  - Build benchmarks (default: $(PAGMO_BUILD_BENCHMARKS))"
	@echo "  PAGMO_BUILD_TUTORIALS   - Build tutorials (default: $(PAGMO_BUILD_TUTORIALS))"
	@echo "  PAGMO_WITH_EIGEN3       - Enable Eigen3 support (default: $(PAGMO_WITH_EIGEN3))"
	@echo "  PAGMO_WITH_NLOPT        - Enable NLopt support (default: $(PAGMO_WITH_NLOPT))"
	@echo "  PAGMO_WITH_IPOPT        - Enable Ipopt support (default: $(PAGMO_WITH_IPOPT))"
	@echo "  PAGMO_WITH_MPI          - Enable MPI (default: $(PAGMO_WITH_MPI))"
	@echo "  PAGMO_ENABLE_IPO        - Enable IPO optimization (default: $(PAGMO_ENABLE_IPO))"
	@echo "  PAGMO_BUILD_STATIC_LIBRARY - Build static library (default: $(PAGMO_BUILD_STATIC_LIBRARY))"
	@echo ""
	@echo "Examples:"
	@echo "  make debug                    - Build debug version"
	@echo "  make BUILD_TYPE=Debug test    - Build and test debug version"
	@echo "  make PAGMO_WITH_EIGEN3=ON     - Build with Eigen3 support"
	@echo "  make install INSTALL_PREFIX=/opt/pagmo - Install to custom prefix"

# Build targets
.PHONY: configure
configure: $(BUILD_DIR)/Makefile

$(BUILD_DIR)/Makefile: CMakeLists.txt
	@echo "==> Configuring build..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) ../.. $(CMAKE_FLAGS)

.PHONY: reconfigure
reconfigure:
	@echo "==> Forcing reconfiguration..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) ../.. $(CMAKE_FLAGS)

.PHONY: build
build: configure
	@echo "==> Building pagmo2..."
	@$(MAKE) -C $(BUILD_DIR) -j$(NUM_JOBS) --no-print-directory

.PHONY: debug
debug:
	@$(MAKE) BUILD_TYPE=Debug $(filter-out debug,$(MAKECMDGOALS))

.PHONY: release
release:
	@$(MAKE) BUILD_TYPE=Release $(filter-out release,$(MAKECMDGOALS))

.PHONY: relwithdebinfo
relwithdebinfo:
	@$(MAKE) BUILD_TYPE=RelWithDebInfo $(filter-out relwithdebinfo,$(MAKECMDGOALS))

.PHONY: minsizerel
minsizerel:
	@$(MAKE) BUILD_TYPE=MinSizeRel $(filter-out minsizerel,$(MAKECMDGOALS))

# Test targets
.PHONY: test
test: build
	@echo "==> Running tests..."
	@cd $(BUILD_DIR) && $(CTEST) -j$(NUM_JOBS) --output-on-failure

.PHONY: test-verbose
test-verbose: build
	@echo "==> Running tests (verbose)..."
	@cd $(BUILD_DIR) && $(CTEST) -j$(NUM_JOBS) -V

.PHONY: test-parallel
test-parallel: build
	@echo "==> Running tests in parallel..."
	@cd $(BUILD_DIR) && $(CTEST) -j$(NUM_JOBS) --parallel $(NUM_JOBS) --output-on-failure

# Installation targets
.PHONY: install
install: build
	@echo "==> Installing pagmo2..."
	@$(MAKE) -C $(BUILD_DIR) install --no-print-directory

.PHONY: uninstall
uninstall:
	@echo "==> Uninstalling pagmo2..."
	@if [ -f $(BUILD_DIR)/install_manifest.txt ]; then \
		xargs rm < $(BUILD_DIR)/install_manifest.txt; \
		rm -f $(BUILD_DIR)/install_manifest.txt; \
	else \
		echo "No install manifest found. Cannot uninstall."; \
		exit 1; \
	fi

# Documentation targets
.PHONY: docs
docs: configure
	@echo "==> Building documentation..."
	@if [ -d "doc" ]; then \
		if command -v doxygen >/dev/null 2>&1; then \
			$(MAKE) -C $(BUILD_DIR) docs --no-print-directory; \
		else \
			echo "Error: doxygen not found. Please install doxygen to build documentation."; \
			exit 1; \
		fi; \
	else \
		echo "No documentation directory found."; \
	fi

.PHONY: docs-serve
docs-serve: docs
	@echo "==> Serving documentation..."
	@if [ -d "$(BUILD_DIR)/doc/html" ]; then \
		echo "Documentation available at http://localhost:8000"; \
		cd $(BUILD_DIR)/doc/html && python3 -m http.server 8000; \
	else \
		echo "Documentation not built. Run 'make docs' first."; \
	fi

# Development targets
.PHONY: benchmarks
benchmarks:
	@echo "==> Building benchmarks..."
	@$(MAKE) build PAGMO_BUILD_BENCHMARKS=ON
	@echo "==> Running benchmarks..."
	@cd $(BUILD_DIR) && $(MAKE) -j$(NUM_JOBS) benchmarks --no-print-directory

.PHONY: tutorials
tutorials:
	@echo "==> Building tutorials..."
	@$(MAKE) build PAGMO_BUILD_TUTORIALS=ON

.PHONY: format
format:
	@echo "==> Formatting source code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find pagmo -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i; \
		find tests -name "*.cpp" | xargs clang-format -i; \
		echo "Code formatting complete."; \
	else \
		echo "Error: clang-format not found. Please install clang-format."; \
		exit 1; \
	fi

.PHONY: lint
lint:
	@echo "==> Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=warning,style,performance,portability --std=c++14 -I pagmo pagmo/; \
	else \
		echo "Error: cppcheck not found. Please install cppcheck for static analysis."; \
	fi

# Package targets
.PHONY: package
package: build
	@echo "==> Creating package..."
	@cd $(BUILD_DIR) && $(MAKE) package --no-print-directory

.PHONY: package-source
package-source: configure
	@echo "==> Creating source package..."
	@cd $(BUILD_DIR) && $(MAKE) package_source --no-print-directory

# Cleanup targets
.PHONY: clean
clean:
	@echo "==> Cleaning build files..."
	@if [ -d "$(BUILD_DIR)" ]; then \
		$(MAKE) -C $(BUILD_DIR) clean --no-print-directory; \
	fi

.PHONY: distclean
distclean:
	@echo "==> Complete cleanup..."
	@rm -rf $(BUILD_DIR)
	@rm -rf doc/doxygen/html doc/doxygen/xml
	@find . -name "*.o" -delete
	@find . -name "*.so" -delete
	@find . -name "*.a" -delete
	@find . -name "*~" -delete

.PHONY: new
new:
	@echo "==> Removing all build folders completely..."
	@rm -rf build

# Development convenience targets
.PHONY: rebuild
rebuild: clean build

.PHONY: debug-test
debug-test:
	@echo "==> Debug test build and run..."
	@$(MAKE) build PAGMO_BUILD_TESTS=ON BUILD_TYPE=Debug
	@$(MAKE) test

.PHONY: release-test
release-test:
	@echo "==> Release build and test..."
	@$(MAKE) build BUILD_TYPE=Release PAGMO_BUILD_TESTS=ON
	@$(MAKE) test

.PHONY: full-test
full-test: full-build
	@echo "==> Full test build with all features..."
	@$(MAKE) test

.PHONY: full-build
full-build:
	@echo "==> Full build with all features..."
	@$(MAKE) reconfigure \
		PAGMO_BUILD_TESTS=ON \
		PAGMO_BUILD_BENCHMARKS=ON \
		PAGMO_BUILD_TUTORIALS=ON \
		PAGMO_WITH_EIGEN3=ON \
		PAGMO_WITH_NLOPT=ON \
		PAGMO_WITH_MPI=ON \
		PAGMO_ENABLE_IPO=ON
	@$(MAKE) -C $(BUILD_DIR) -j$(NUM_JOBS) --no-print-directory 

.PHONY: mpi
mpi:
	@echo "==> Building with MPI..."
	@$(MAKE) build PAGMO_WITH_MPI=ON

# Status and info targets
.PHONY: status
status:
	@echo "Build Configuration Status:"
	@echo "=========================="
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Build type: $(BUILD_TYPE)"
	@echo "Install prefix: $(INSTALL_PREFIX)"
	@echo "Build tests: $(PAGMO_BUILD_TESTS)"
	@echo "Build benchmarks: $(PAGMO_BUILD_BENCHMARKS)"
	@echo "Build tutorials: $(PAGMO_BUILD_TUTORIALS)"
	@echo "With Eigen3: $(PAGMO_WITH_EIGEN3)"
	@echo "With NLopt: $(PAGMO_WITH_NLOPT)"
	@echo "With Ipopt: $(PAGMO_WITH_IPOPT)"
	@echo "Enable IPO: $(PAGMO_ENABLE_IPO)"
	@echo "Static library: $(PAGMO_BUILD_STATIC_LIBRARY)"
	@echo "Parallel jobs: $(NUM_JOBS)"
	@echo ""
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		echo "CMake configuration exists."; \
	else \
		echo "CMake not configured yet."; \
	fi

.PHONY: deps-check
deps-check:
	@echo "==> Checking dependencies..."
	@echo -n "CMake: "; command -v cmake >/dev/null && echo "✓ Found" || echo "✗ Missing"
	@echo -n "Make: "; command -v make >/dev/null && echo "✓ Found" || echo "✗ Missing"
	@echo -n "C++ Compiler: "; (command -v g++ >/dev/null || command -v clang++ >/dev/null) && echo "✓ Found" || echo "✗ Missing"
	@echo -n "Git: "; command -v git >/dev/null && echo "✓ Found" || echo "✗ Missing"
	@echo -n "Doxygen (docs): "; command -v doxygen >/dev/null && echo "✓ Found" || echo "✗ Missing (optional)"
	@echo -n "clang-format (format): "; command -v clang-format >/dev/null && echo "✓ Found" || echo "✗ Missing (optional)"
	@echo -n "cppcheck (lint): "; command -v cppcheck >/dev/null && echo "✓ Found" || echo "✗ Missing (optional)"

# Debug information
.PHONY: debug-info
debug-info:
	@echo "Debug Information:"
	@echo "=================="
	@echo "Current directory: $(shell pwd)"
	@echo "Build directory exists: $(shell [ -d $(BUILD_DIR) ] && echo "Yes" || echo "No")"
	@echo "CMake cache exists: $(shell [ -f $(BUILD_DIR)/CMakeCache.txt ] && echo "Yes" || echo "No")"
	@echo "CMAKE_FLAGS: $(CMAKE_FLAGS)"
	@echo ""
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		echo "CMake Cache Variables:"; \
		grep -E "(CMAKE_BUILD_TYPE|CMAKE_INSTALL_PREFIX|PAGMO_)" $(BUILD_DIR)/CMakeCache.txt | head -20; \
	fi

# Make sure intermediate files don't get deleted
.PRECIOUS: $(BUILD_DIR)/Makefile
