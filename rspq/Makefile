BUILD_DIR=build
include $(N64_INST)/include/n64.mk

all: rspq_test.z64

$(BUILD_DIR)/rspq_test.elf: $(BUILD_DIR)/rspq_test.o $(BUILD_DIR)/rsp_vec.o $(BUILD_DIR)/vec.o

rspqdemo.z64: N64_ROM_TITLE="RSPQ Test"

clean:
	rm -rf $(BUILD_DIR) rspq_test.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
