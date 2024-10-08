BUILD_DIR = build

include $(N64_INST)/include/n64.mk

PROJECT_NAME = 2d_shapes_cpp

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

assets_png = $(wildcard assets/*.png)

assets_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite)))

all: $(PROJECT_NAME).z64
filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/n64brew.sprite: MKSPRITE_FLAGS=--format RGBA16

$(BUILD_DIR)/$(PROJECT_NAME).dfs: $(assets_conv)
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(SRC:%.cpp=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "    [CXX] $@"
	$(CXX) $(N64_CXXFLAGS) -c $< -o $@

$(PROJECT_NAME).z64: N64_ROM_TITLE="2D Shapes C++"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).dfs

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf filesystem

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
