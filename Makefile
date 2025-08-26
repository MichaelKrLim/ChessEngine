ENGINE := ChessEngine
BUILD_DIR := build

CXX ?= g++
EXE ?= $(ENGINE)

all: $(ENGINE)

$(ENGINE):
	@echo "Building engine..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_CXX_COMPILER=$(CXX) ..
	@cd $(BUILD_DIR) && make $(ENGINE)
	@cp $(BUILD_DIR)/$(ENGINE) $(EXE)

.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(ENGINE)

.PHONY: all
