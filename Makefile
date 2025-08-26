ENGINE := engine
BUILD_DIR := build

all: $(ENGINE)

$(ENGINE):
	@echo "Building engine..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@cd $(BUILD_DIR) && make $(ENGINE)
	@cp $(BUILD_DIR)/$(ENGINE) .

.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(ENGINE)

.PHONY: all
