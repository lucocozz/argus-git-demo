# Makefile for Argus Test Project
# This is a convenience wrapper around Meson

# Default target
.PHONY: all
all: build

# Setup build directory
.PHONY: setup
setup:
	@echo "Setting up build directory..."
	@meson setup build

# Build the project
.PHONY: build
build: setup
	@echo "Building project..."
	@meson compile -C build

# Clean build files (keep configuration)
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@if [ -d build ]; then ninja -C build clean; fi

# Full clean (remove build directory)
.PHONY: fclean
fclean:
	@echo "Removing build directory..."
	@rm -rf build

# Rebuild everything
.PHONY: rebuild
rebuild: fclean build

# Run the program
.PHONY: run
run: build
	@echo "Running git-clone..."
	@./build/git-clone

# Run with help
.PHONY: help-run
help-run: build
	@echo "Running git-clone --help..."
	@./build/git-clone --help

# Install the program
.PHONY: install
install: build
	@echo "Installing..."
	@meson install -C build

# Show build configuration
.PHONY: configure
configure:
	@echo "Build configuration:"
	@if [ -d build ]; then meson configure build; else echo "Build directory not found. Run 'make setup' first."; fi

# Reconfigure build
.PHONY: reconfigure
reconfigure:
	@echo "Reconfiguring build..."
	@if [ -d build ]; then meson setup --reconfigure build; else meson setup build; fi

# Development builds
.PHONY: debug
debug:
	@echo "Setting up debug build..."
	@meson setup build -Dbuildtype=debug
	@meson compile -C build

.PHONY: release
release:
	@echo "Setting up release build..."
	@meson setup build -Dbuildtype=release
	@meson compile -C build

# Test different command examples
.PHONY: test-commands
test-commands: build
	@echo "Testing various Git commands..."
	@echo "\n=== Main help ==="
	@./build/git-clone --help || true
	@echo "\n=== Git init help ==="
	@./build/git-clone init --help || true
	@echo "\n=== Git remote help ==="
	@./build/git-clone remote --help || true
	@echo "\n=== Git remote add help ==="
	@./build/git-clone remote add --help || true
	@echo "\n=== Git config help ==="
	@./build/git-clone config --help || true
	@echo "\n=== Git stash help ==="
	@./build/git-clone stash --help || true
	@echo "\n=== Git stash push help ==="
	@./build/git-clone stash push --help || true

# Create data directory if it doesn't exist
.PHONY: setup-data
setup-data:
	@echo "Setting up data directory..."
	@mkdir -p data

# Development workflow
.PHONY: dev
dev: rebuild test-commands

# Show help
.PHONY: help
help:
	@echo "Argus Test Project Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all            - Build the project (default)"
	@echo "  setup          - Setup build directory"
	@echo "  build          - Build the project"
	@echo "  clean          - Clean build files"
	@echo "  fclean         - Remove build directory"
	@echo "  rebuild        - Clean and rebuild everything"
	@echo "  run            - Build and run the program"
	@echo "  help-run       - Build and run with --help"
	@echo "  install        - Install the program"
	@echo "  configure      - Show build configuration"
	@echo "  reconfigure    - Reconfigure build"
	@echo "  debug          - Setup debug build"
	@echo "  release        - Setup release build"
	@echo "  test-commands  - Test various command combinations"
	@echo "  setup-data     - Create data directory"
	@echo "  dev            - Development workflow (rebuild + test)"
	@echo "  help           - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make build              # Build the project"
	@echo "  make run                # Build and run"
	@echo "  make test-commands      # Test command help outputs"
	@echo "  make debug              # Create debug build"
	@echo "  make clean build        # Clean and build"