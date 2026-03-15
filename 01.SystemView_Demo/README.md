# HC32F460 SystemView Demo

基于 HC32F460（Cortex-M4F）+ FreeRTOS + SEGGER SystemView 的验证工程，使用 GCC 工具链 + Makefile 在 VS Code 中开发调试。

---

## 工程概述

本工程用于验证 SystemView 在 HC32F460 上的任务调度可视化功能。包含两个 FreeRTOS 任务和一个二值信号量，通过 SEGGER SystemView 实时观察任务切换、信号量操作和中断行为。

**任务设计：**

- **Task2_Sender**（优先级 3）：每 500ms 释放一次信号量，翻转 LED1
- **Task1_LED**（优先级 2）：阻塞等待信号量，收到后翻转 LED0

---

## 硬件环境

| 项目 | 规格 |
|------|------|
| MCU | HC32F460xE（512KB Flash / 192KB SRAM） |
| 内核 | ARM Cortex-M4F @ 200MHz |
| 外部晶振 | 24MHz |
| 调试器 | J-Link（SWD 接口） |

---

## 软件环境

| 工具 | 版本 |
|------|------|
| GCC 工具链 | arm-gnu-toolchain 15.2 Rel1 (arm-none-eabi) |
| 构建系统 | GNU Make |
| IDE | VS Code + Cortex-Debug 扩展 |
| DDL 驱动库 | HC32F460 DDL Rev3.3.0 (LL API) |
| RTOS | FreeRTOS（Kernel V11.x） |
| Trace 工具 | SEGGER SystemView + RTT |

---

## 工程目录结构

```
HC32F460_SystemView_Demo/
├── Application/                        # 应用层代码
│   ├── main.c                          # 主程序（任务创建、时钟初始化）
│   ├── FreeRTOSConfig.h                # FreeRTOS 配置（含 SystemView trace 宏）
│   └── hc32f4xx_conf.h                 # DDL 驱动模块使能配置
│
├── Drivers/                            # 驱动层
│   ├── CMSIS/
│   │   ├── Core/Include/               # ARM CMSIS 内核头文件（core_cm4.h 等）
│   │   └── Device/HDSC/hc32f4xx/
│   │       ├── Include/                # hc32f460.h, system_hc32f460.h
│   │       └── Source/
│   │           ├── system_hc32f460.c   # 系统初始化
│   │           └── GCC/
│   │               ├── startup_hc32f460.S  # GCC 格式启动文件
│   │               └── svd/HC32F460.svd    # 寄存器描述文件
│   ├── HC32F460_DDL_Rev3.3.0/         # 华大 LL 外设驱动库
│   │   └── drivers/
│   │       ├── hc32_ll_driver/         # LL 驱动源文件和头文件
│   │       └── bsp/                    # 官方 BSP（本工程未使用）
│   └── BSP/                            # 自定义板级支持（预留）
│
├── Middleware/                          # 中间件层
│   ├── FreeRTOS/
│   │   └── Source/                     # FreeRTOS 内核源码
│   │       ├── tasks.c, queue.c, list.c, timers.c ...
│   │       └── portable/
│   │           ├── GCC/ARM_CM4F/       # GCC Cortex-M4F 移植层
│   │           └── MemMang/heap_4.c    # 动态内存管理
│   └── SEGGER/                         # SEGGER SystemView + RTT
│       ├── SEGGER_RTT.c/.h             # RTT 通信核心
│       ├── SEGGER_RTT_Conf.h           # RTT 配置
│       ├── SEGGER_RTT_ASM_ARMv7M.S    # RTT 汇编优化
│       ├── SEGGER_SYSVIEW.c/.h         # SystemView 核心
│       ├── SEGGER_SYSVIEW_FreeRTOS.c/.h  # FreeRTOS 适配层
│       └── SEGGER_SYSVIEW_Config_FreeRTOS.c  # SystemView 配置（HC32F460 适配）
│
├── Linker/
│   └── HC32F460xE.ld                   # 512KB Flash 链接脚本（本工程使用）
│
├── Makefile                            # GCC 构建脚本
├── README.md
│
└── .vscode/
    ├── tasks.json                      # 构建任务（Ctrl+Shift+B）
    ├── launch.json                     # J-Link 调试配置（F5）
    └── c_cpp_properties.json           # IntelliSense 配置
```

---

## 关键配置说明

### 时钟树

```
XTAL 24MHz ──→ MPLL ──→ 200MHz (HCLK)
                │
                ├─ PLLM = 3    (24MHz / 3 = 8MHz)
                ├─ PLLN = 50   (8MHz × 50 = 400MHz VCO)
                ├─ PLLP = 2    (400MHz / 2 = 200MHz → SYSCLK)
                ├─ PLLQ = 4    (400MHz / 4 = 100MHz)
                └─ PLLR = 4    (400MHz / 4 = 100MHz)

总线分频：
  HCLK  = 200MHz (÷1)    PCLK0 = 200MHz (÷1)
  PCLK1 = 100MHz (÷2)    PCLK2 = 50MHz  (÷4)
  PCLK3 = 50MHz  (÷4)    PCLK4 = 100MHz (÷2)

Flash 等待周期：5 (200MHz 需要)
```

### FreeRTOS 配置要点

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `configCPU_CLOCK_HZ` | 200000000 | 系统主频 |
| `configTICK_RATE_HZ` | 1000 | 1ms Tick |
| `configTOTAL_HEAP_SIZE` | 32KB | 动态内存池 |
| `configUSE_TRACE_FACILITY` | 1 | SystemView 必须 |
| `configUSE_STATS_FORMATTING_FUNCTIONS` | 1 | SystemView 必须 |
| `INCLUDE_xTaskGetIdleTaskHandle` | 1 | SystemView 必须 |

FreeRTOSConfig.h 末尾必须包含：

```c
#include "SEGGER_SYSVIEW_FreeRTOS.h"  /* 必须放在最后 */
```

### SystemView 配置要点（SEGGER_SYSVIEW_Config_FreeRTOS.c）

- **时间戳源**：DWT CYCCNT（Cortex-M4 硬件周期计数器，32 位，无额外开销）
- **SRAM 基地址**：`0x1FFF8000`（HC32F460 SRAM 起始地址）
- **DWT 初始化**：在 `SEGGER_SYSVIEW_Conf()` 中通过寄存器直接操作使能

### 编译选项

| 选项 | 值 | 说明 |
|------|-----|------|
| MCU | `-mcpu=cortex-m4 -mthumb -mfloat-abi=soft` | 软浮点（与 startup 的 `.fpu softvfp` 匹配） |
| 优化级别 | `-O0 -g -gdwarf-2` | 调试模式，无优化 |
| 链接 | `-specs=nano.specs -specs=nosys.specs` | 精简 C 库 |
| 关键宏 | `-DHC32F460 -D__DEBUG -DUSE_DDL_DRIVER` | DDL Rev3.3.0 必须定义 `USE_DDL_DRIVER` |

> **注意**：如需硬浮点，需同步修改 startup_hc32f460.S 中 `.fpu softvfp` 为 `.fpu fpv4-sp-d16`，并将 Makefile 中 `-mfloat-abi=soft` 改为 `-mfloat-abi=hard -mfpu=fpv4-sp-d16`。

---

## 构建与调试

### 编译

```cmd
make -j8          # 并行编译
make clean        # 清理构建产物
make flash        # J-Link 命令行烧录
```

### VS Code 快捷操作

| 操作 | 快捷键 |
|------|--------|
| 编译 | `Ctrl+Shift+B` |
| 调试 | `F5` |
| 停止调试 | `Shift+F5` |

### SystemView 录制

1. 编译烧录，确认程序正常运行（LED 闪烁）
2. 打开 PC 端 SEGGER SystemView 应用
3. 配置：Target → J-Link → Device: `Cortex-M4` → Interface: SWD
4. 点击 **Start Recording**
5. Timeline 视图观察任务调度、信号量操作

如果 SystemView 连接不上 RTT，在 `build/hc32f460_sysview_demo.map` 中搜索 `_SEGGER_RTT` 获取地址，手动填入 SystemView 连接设置。

---

## SystemView 使用技巧

### 自定义标记

在代码中插入标记点，可以在 Timeline 上精确测量代码段执行时间：

```c
SEGGER_SYSVIEW_MarkStart(0);       /* 开始测量 */
/* ... 被测代码 ... */
SEGGER_SYSVIEW_MarkStop(0);        /* 结束测量 */

/* 打印自定义事件到 Timeline */
SEGGER_SYSVIEW_PrintfHost("ADC val=%d", adcValue);
```

### 中断名称映射

在 `SEGGER_SYSVIEW_Config_FreeRTOS.c` 的 `_cbSendSystemDesc()` 中添加中断描述，中断号 = IRQn + 16：

```c
SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
SEGGER_SYSVIEW_SendSysDesc("I#62=SPI1_Rx");
```

### 统计分析

- `Analysis → Task Statistics`：查看各任务 CPU 占用率、执行时间统计
- `Analysis → System Information`：查看中断响应延迟统计

---

## 常见问题

### 编译报 `stc_gpio_init_t` 未定义

Makefile 的 `C_DEFS` 中必须包含 `-DUSE_DDL_DRIVER`，这是 DDL Rev3.3.0 的条件编译开关。

### SystemClock_Init 卡住

检查外部晶振频率是否与代码中的 PLL 配置匹配。本工程配置为 24MHz 晶振，如果你的板子使用 8MHz 晶振，需要修改 PLLM 分频值（改为 1）和 `hc32f4xx_conf.h` 中的 `XTAL_VALUE`。

### J-Link 调试报 ENOENT

`JLinkGDBServerCL.exe` 不在系统 PATH 中。将 J-Link 安装目录（如 `C:\Program Files\SEGGER\JLink\`）加入 PATH，或在 launch.json 中添加 `"serverpath"` 指定完整路径。

