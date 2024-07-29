BUILD_DIR = build
T3D_DIR=$(HOME)/t3d
T3D_INST=$(shell realpath $(T3D_DIR))

include $(N64_INST)/include/n64.mk
include $(T3D_INST)/t3d.mk

PROJECT_NAME = animal-proc-anim

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

SRC = ProcAnim.cpp \
      Chain.cpp \
	  Fish.cpp \
      PVector.cpp \
      Snake.cpp \
      Utils.cpp

OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

all: $(PROJECT_NAME).z64

$(BUILD_DIR)/$(PROJECT_NAME).elf: $(SRC:%.cpp=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(N64_CXXFLAGS) -c $< -o $@

$(PROJECT_NAME).z64: N64_ROM_TITLE="Procedural Anims"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).elf

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf filesystem

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
