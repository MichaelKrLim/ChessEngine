.PHONY: all

all:
	mkdir -p build
	@echo "Running CMake..."
	cd build && cmake .. || { echo "CMake failed"; exit 1; }
	@echo "Building engine..."
	cd build && make engine || { echo "Make failed"; exit 1; }
	@echo "Copying engine to parent directory..."
	cp build/engine .
	@echo "Cleaning up build directory..."
	rm -rf build
