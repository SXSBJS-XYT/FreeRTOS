##############################################################################
# HC32F460 SystemView Demo - GCC Makefile
# 适配 DDL Rev3.3.0 + FreeRTOS + SEGGER SystemView
##############################################################################

##############################
# 工具链
##############################
PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
AS      = $(PREFIX)gcc -x assembler-with-cpp
LD      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE    = $(PREFIX)size

##############################
# 目标
##############################
TARGET  = hc32f460_sysview_demo
BUILD   = build

##############################
# 芯片与 FPU 配置
# 注: startup 文件里是 softvfp，这里保持一致用软浮点
# 如果改成硬浮点，需要同步修改 startup_hc32f460.S 中的 .fpu 指令
##############################
# MCU = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
MCU = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
##############################
# 源文件
##############################

# --- Application ---
C_SOURCES += Application/main.c

# --- Drivers: CMSIS ---
C_SOURCES += Drivers/CMSIS/Device/HDSC/hc32f4xx/Source/system_hc32f460.c

# --- Drivers: DDL Rev3.3.0 (按需添加用到的模块) ---
DDL_SRC = Drivers/HC32F460_DDL_Rev3.3.0/drivers/hc32_ll_driver/src
C_SOURCES += $(DDL_SRC)/hc32_ll.c
C_SOURCES += $(DDL_SRC)/hc32_ll_clk.c
C_SOURCES += $(DDL_SRC)/hc32_ll_efm.c
C_SOURCES += $(DDL_SRC)/hc32_ll_fcg.c
C_SOURCES += $(DDL_SRC)/hc32_ll_gpio.c
C_SOURCES += $(DDL_SRC)/hc32_ll_icg.c
C_SOURCES += $(DDL_SRC)/hc32_ll_interrupts.c
C_SOURCES += $(DDL_SRC)/hc32f460_ll_interrupts_share.c
C_SOURCES += $(DDL_SRC)/hc32_ll_pwc.c
C_SOURCES += $(DDL_SRC)/hc32_ll_sram.c
C_SOURCES += $(DDL_SRC)/hc32_ll_utility.c

# --- Middleware: FreeRTOS ---
FREERTOS_SRC = Middleware/FreeRTOS/Source
C_SOURCES += $(FREERTOS_SRC)/tasks.c
C_SOURCES += $(FREERTOS_SRC)/queue.c
C_SOURCES += $(FREERTOS_SRC)/list.c
C_SOURCES += $(FREERTOS_SRC)/timers.c
C_SOURCES += $(FREERTOS_SRC)/event_groups.c
C_SOURCES += $(FREERTOS_SRC)/stream_buffer.c
C_SOURCES += $(FREERTOS_SRC)/portable/GCC/ARM_CM4F/port.c
C_SOURCES += $(FREERTOS_SRC)/portable/MemMang/heap_4.c

# --- Middleware: SEGGER SystemView ---
# 注意: 你需要从 SEGGER SystemView 安装目录复制以下源文件到 Middleware/SEGGER/
SEGGER_SRC = Middleware/SEGGER
C_SOURCES += $(SEGGER_SRC)/SEGGER_SYSVIEW_Config_FreeRTOS.c
C_SOURCES += $(SEGGER_SRC)/SEGGER_RTT.c
C_SOURCES += $(SEGGER_SRC)/SEGGER_SYSVIEW.c
C_SOURCES += $(SEGGER_SRC)/SEGGER_SYSVIEW_FreeRTOS.c

# --- Startup (GCC 汇编) ---
ASM_SOURCES = Drivers/CMSIS/Device/HDSC/hc32f4xx/Source/GCC/startup_hc32f460.S
# SEGGER汇编文件
ASM_SOURCES += $(SEGGER_SRC)/SEGGER_RTT_ASM_ARMv7M.S
##############################
# 头文件路径
##############################
C_INCLUDES  = -IApplication
C_INCLUDES += -IDrivers/CMSIS/Core/Include
C_INCLUDES += -IDrivers/CMSIS/Device/HDSC/hc32f4xx/Include
C_INCLUDES += -IDrivers/HC32F460_DDL_Rev3.3.0/drivers/hc32_ll_driver/inc
C_INCLUDES += -IDrivers/BSP
C_INCLUDES += -IMiddleware/FreeRTOS/Source/include
C_INCLUDES += -IMiddleware/FreeRTOS/Source/portable/GCC/ARM_CM4F
C_INCLUDES += -IMiddleware/SEGGER

##############################
# 宏定义
##############################
C_DEFS  = -DHC32F460
C_DEFS += -D__DEBUG
C_DEFS += -DUSE_DDL_DRIVER
##############################
# 编译选项
##############################
CFLAGS  = $(MCU) $(C_DEFS) $(C_INCLUDES)
CFLAGS += -Wall -Wextra -Wno-unused-parameter
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -g -gdwarf-2 -O0
# O0 方便调试; 发布时可改 -Os 或 -O2

ASFLAGS = $(MCU) -Wall -fdata-sections -ffunction-sections -g

##############################
# 链接选项
##############################
LDSCRIPT = Linker/HC32F460xE.ld
LDFLAGS  = $(MCU) -specs=nano.specs -specs=nosys.specs -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map,--cref
LDFLAGS += -lc -lm

##############################
# 构建规则
##############################
OBJECTS  = $(addprefix $(BUILD)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD)/,$(notdir $(ASM_SOURCES:.S=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

.PHONY: all clean flash size

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin size

$(BUILD)/%.o: %.c | $(BUILD)
	@echo "[CC] $<"
	@$(CC) -c $(CFLAGS) -MMD -MP $< -o $@

$(BUILD)/%.o: %.S | $(BUILD)
	@echo "[AS] $<"
	@$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJECTS)
	@echo "[LD] $@"
	@$(LD) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	@$(OBJCOPY) -O ihex $< $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	@$(OBJCOPY) -O binary $< $@

$(BUILD):
ifeq ($(OS),Windows_NT)
	if not exist $(BUILD) mkdir $(BUILD)
else
	mkdir -p $(BUILD)
endif

size: $(BUILD)/$(TARGET).elf
	@echo ""
	@echo "========== Build Size =========="
	@$(SIZE) $<
	@echo "================================"

clean:
ifeq ($(OS),Windows_NT)
	if exist $(BUILD) rmdir /s /q $(BUILD)
else
	rm -rf $(BUILD)
endif

# J-Link 烧录 (需要 JLink.exe 在 PATH 中)
flash: $(BUILD)/$(TARGET).hex
	JLink -device HC32F460PETB -if SWD -speed 4000 -autoconnect 1 -CommandFile jlink_flash.jlink

# 添加一个调试目标，查看所有文件
debug:
	@echo "========== Source Files =========="
	@for f in $(C_SOURCES); do echo "  $$f"; done
	@for f in $(ASM_SOURCES); do echo "  $$f"; done
	@echo "========== Object Files =========="
	@for f in $(OBJECTS); do echo "  $$f"; done

# 包含自动依赖文件
-include $(wildcard $(BUILD)/*.d)
