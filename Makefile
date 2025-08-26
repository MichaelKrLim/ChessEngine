TARGET					=	ChessEngine
CFLAGS					=	-std=c++23 -Wall -pedantic -O3 -march=native -MMD -MP
BUILD_DIR				=	build

SRCS					=	src/main.cpp src/State.cpp src/Move_generator.cpp src/Uci_handler.cpp
OBJS					=	$(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS					=	$(OBJS:.o=.d)

all: $(TARGET)

check-cxx23:
	@echo "Checking for C++23 support..."
	@mkdir -p $(BUILD_DIR)
	@echo '#if __cplusplus < 202302L' > $(BUILD_DIR)/check.cpp
	@echo '#error "C++23 support is required (compiler reports __cplusplus = __cplusplus)"' >> $(BUILD_DIR)/check.cpp
	@echo '#endif' >> $(BUILD_DIR)/check.cpp
	@$(CXX) -std=c++23 -c $(BUILD_DIR)/check.cpp -o $(BUILD_DIR)/check.o
	@rm -f $(BUILD_DIR)/check.cpp $(BUILD_DIR)/check.o

$(TARGET): check-cxx23 $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)

.PHONY: all clean check-cxx23
