.PHONY: debug release test coverage apps clean help

# Default target
all: release

help:
	@echo "ABox Build Targets:"
	@echo "  make debug      - Debug build with tests (ASAN enabled)"
	@echo "  make release    - Optimized release build"
	@echo "  make test       - Build and run tests (debug mode)"
	@echo "  make coverage   - Generate code coverage report"
	@echo "  make apps       - Build with applications"
	@echo "  make clean      - Remove all build artifacts"

debug:
	@echo "Configuring debug build..."
	@cmake --preset debug
	@echo "Building..."
	@cmake --build --preset debug --parallel
	@echo "Debug build complete: build/"

release:
	@echo "Configuring release build..."
	@cmake --preset release
	@echo "Building..."
	@cmake --build --preset release --parallel
	@echo "Release build complete: build/"

test: debug
	@echo "Running tests..."
	@ctest --preset debug
	@echo "Tests complete"

coverage:
	@echo "Configuring coverage build..."
	@cmake --preset coverage
	@echo "Building..."
	@cmake --build --preset coverage --parallel
	@echo "Running tests and generating coverage report..."
	@cmake --build build --target coverage
	@echo "Coverage report: build/coverage-report/index.html"
	@echo "Open with: xdg-open build/coverage-report/index.html"

apps:
	@echo "Configuring apps build..."
	@cmake --preset apps
	@echo "Building..."
	@cmake --build --preset apps --parallel
	@echo "Apps build complete: build/"

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build/
	@rm -f compile_commands.json
	@echo "Clean complete"
