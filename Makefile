BUILD_DIR = build

include $(N64_INST)/include/n64.mk

PROJECT_NAME = 2d_shapes

DEBUG = 0

ifeq ($(DEBUG),0)
  N64_CXXFLAGS += -O2
else
  N64_CXXFLAGS += -g -ggdb
endif

N64_CXXFLAGS += -mno-check-zero-division \
	-funsafe-math-optimizations \
	-fsingle-precision-constant \
	-fno-unroll-loops \
	-fno-peel-loops \
	-falign-functions=32 \
	-fno-merge-constants \
    -fno-strict-aliasing \
	-ffast-math \
    -mips3 \

SRC = main.cpp \
      Point.cpp \
	  Render.cpp \
      Shape.cpp \
      Utils.cpp

OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

all: $(PROJECT_NAME).z64

$(BUILD_DIR)/$(PROJECT_NAME).elf: $(SRC:%.cpp=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "    [CXX] $@"
	$(CXX) $(N64_CXXFLAGS) -c $< -o $@

$(PROJECT_NAME).z64: N64_ROM_TITLE="2D Shapes"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).elf

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf filesystem

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
