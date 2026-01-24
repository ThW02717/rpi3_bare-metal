ARMGNU ?= aarch64-linux-gnu
QEMU ?= qemu-system-aarch64
QEMU_SMP ?= 4
NUM_CORES ?= $(QEMU_SMP)
QEMU_FLAGS ?= -M raspi3b -serial stdio -display none -smp $(QEMU_SMP)
QEMU_DEFS ?= -DQEMU

COPS = -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
COPS += -DNUM_CORES=$(NUM_CORES)
ASMOPS = -Iinclude 
BUILD_DIR = build
SRC_DIR = src

.PHONY: all clean qemu

all : kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -MP -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

qemu: COPS += $(QEMU_DEFS)
qemu: ASMOPS += $(QEMU_DEFS)
# Target: prerequsite
# $<: the first prerequsite
qemu: kernel8.img
	$(QEMU) $(QEMU_FLAGS) -kernel $<
