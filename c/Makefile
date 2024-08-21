BUILD_DIR = build

include $(N64_INST)/include/n64.mk

PROJECT_NAME = 2d_shapes_c

DEBUG = 0

ifeq ($(DEBUG),0)
  N64_CFLAGS += -O2
else
  N64_CFLAGS += -g -ggdb
endif

N64_CFLAGS += -mno-check-zero-division \
	-funsafe-math-optimizations \
	-fsingle-precision-constant \
	-fno-unroll-loops \
	-fno-peel-loops \
	-falign-functions=32 \
	-fno-merge-constants \
    -fno-strict-aliasing \
	-ffast-math \
    -mips3 \

ASM = rdpq/rsp_rdpq_fan.S

SRC = template_main.c \
	point.c \
	render.c \
	shapes.c \
	utils.c

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

assets_png = $(wildcard assets/*.png)

assets_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite)))

all: $(PROJECT_NAME).z64
filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/n64brew.sprite: MKSPRITE_FLAGS=--format RGBA16

$(BUILD_DIR)/$(PROJECT_NAME).dfs: $(assets_conv)
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(ASM:%.S=$(BUILD_DIR)/%.o) $(SRC:%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	@echo "    [CC] $@"
	$(CC) $(N64_CFLAGS) -c $< -o $@

$(PROJECT_NAME).z64: N64_ROM_TITLE="2D Shapes C"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).dfs

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf filesystem

-include $(wildcard $(BUILD_DIR)/rdpq/*.d) $(wildcard $(BUILD_DIR)/*.d) $(wildcard $(BUILD_DIR)/examples/*.d)

.PHONY: all clean
