.PHONY: all
all: build

.PHONY: setup
setup:
	@echo "Setting up build directory..."
	@meson setup build

.PHONY: build
build: setup
	@echo "Building project..."
	@meson compile -C build
	@echo ""
	@echo "\033[32mBuild complete.\033[0m Run '\033[33m./build/git\033[0m' to execute\033[0m."

.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@if [ -d build ]; then ninja -C build clean; fi

.PHONY: fclean
fclean:
	@echo "Removing build directory..."
	@rm -rf build

.PHONY: rebuild
rebuild: fclean build


.PHONY: help
help:
	@echo "Argus Git Demo Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all            - Build the project (default)"
	@echo "  setup          - Setup build directory"
	@echo "  build          - Build the project"
	@echo "  clean          - Clean build files"
	@echo "  fclean         - Remove build directory"
	@echo "  rebuild        - Clean and rebuild everything"
	@echo "  help           - Show this help"
	@echo ""