/**
 * @file  main.c
 * @brief HC32F460 SystemView 验证工程
 *        两个任务 + 一个信号量，用于验证 SystemView 任务切换可视化
 *        适配 DDL Rev3.3.0 LL API
 */
#include "hc32_ll.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "SEGGER_SYSVIEW.h"

/*-----------------------------------------------------------
 * 硬件定义 (HC32F460PETB 开发板 LED)
 *----------------------------------------------------------*/
/* LED0 - PD3 */
#define LED0_PORT       (GPIO_PORT_D)
#define LED0_PIN        (GPIO_PIN_10)

/* LED1 - PD4 */
#define LED1_PORT       (GPIO_PORT_E)
#define LED1_PIN        (GPIO_PIN_15)

/*-----------------------------------------------------------
 * 全局变量
 *----------------------------------------------------------*/
static SemaphoreHandle_t xBinarySem = NULL;
static TaskHandle_t xTask1Handle = NULL;
static TaskHandle_t xTask2Handle = NULL;

/*-----------------------------------------------------------
 * LED 初始化
 *----------------------------------------------------------*/
static void LED_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinState = PIN_STAT_SET;     /* 初始高电平 (灭) */
    stcGpioInit.u16PinDir   = PIN_DIR_OUT;
    stcGpioInit.u16PullUp   = PIN_PU_ON;

    (void)GPIO_Init(LED0_PORT, LED0_PIN, &stcGpioInit);
    (void)GPIO_Init(LED1_PORT, LED1_PIN, &stcGpioInit);
}

static void LED0_Toggle(void)
{
    GPIO_TogglePins(LED0_PORT, LED0_PIN);
}

static void LED1_Toggle(void)
{
    GPIO_TogglePins(LED1_PORT, LED1_PIN);
}

/*-----------------------------------------------------------
 * 系统时钟初始化 (200MHz)
 * XTAL 8MHz -> MPLL VCO 400MHz -> PLLP 200MHz
 *----------------------------------------------------------*/
static void SystemClock_Init(void)
{
    stc_clock_xtal_init_t stcXtalInit;
    stc_clock_pll_init_t  stcPllInit;

    /* 配置 Flash 等待周期 (200MHz 需要 5 个等待周期) */
    /* 必须在提高主频之前设置 */
    EFM_FWMC_Cmd(ENABLE);
    (void)EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    EFM_FWMC_Cmd(DISABLE);

    /* 配置各总线时钟分频 */
    CLK_SetClockDiv(CLK_BUS_HCLK,  CLK_HCLK_DIV1);    /* HCLK  = 200MHz */
    CLK_SetClockDiv(CLK_BUS_PCLK0, CLK_PCLK0_DIV1);   /* PCLK0 = 200MHz */
    CLK_SetClockDiv(CLK_BUS_PCLK1, CLK_PCLK1_DIV2);   /* PCLK1 = 100MHz */
    CLK_SetClockDiv(CLK_BUS_PCLK2, CLK_PCLK2_DIV4);   /* PCLK2 = 50MHz  */
    CLK_SetClockDiv(CLK_BUS_PCLK3, CLK_PCLK3_DIV4);   /* PCLK3 = 50MHz  */
    CLK_SetClockDiv(CLK_BUS_PCLK4, CLK_PCLK4_DIV2);   /* PCLK4 = 100MHz */

    /* 配置并使能外部高速晶振 */
    (void)CLK_XtalStructInit(&stcXtalInit);
    stcXtalInit.u8State      = CLK_XTAL_ON;
    stcXtalInit.u8Drv        = CLK_XTAL_DRV_LOW;
    stcXtalInit.u8Mode       = CLK_XTAL_MD_OSC;
    stcXtalInit.u8SuperDrv   = CLK_XTAL_SUPDRV_OFF;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);

    /* 配置 MPLL: XTAL 8MHz / M(1) * N(50) = VCO 400MHz */
    /*           PLLP = VCO / P(2) = 200MHz              */
    (void)CLK_PLLStructInit(&stcPllInit);
    stcPllInit.u8PLLState        = CLK_PLL_ON;
    stcPllInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;
    stcPllInit.PLLCFGR_f.PLLM   = 3U - 1U;    /* 24MHz / 3 = 8MHz */
    stcPllInit.PLLCFGR_f.PLLN   = 50U - 1U;   /* 8MHz * 50 = 400MHz VCO */
    stcPllInit.PLLCFGR_f.PLLP   = 2U - 1U;    /* 400MHz / 2 = 200MHz */
    stcPllInit.PLLCFGR_f.PLLQ   = 4U - 1U;    /* 400MHz / 4 = 100MHz */
    stcPllInit.PLLCFGR_f.PLLR   = 4U - 1U;    /* 400MHz / 4 = 100MHz */
    (void)CLK_PLLInit(&stcPllInit);

    /* 等待 PLL 稳定 */
    while (SET != CLK_GetStableStatus(CLK_STB_FLAG_PLL)) {
    }

    /* 切换系统时钟到 MPLL */
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_PLL);

    /* 更新 SystemCoreClock 变量 */
    SystemCoreClockUpdate();
}

/*-----------------------------------------------------------
 * 任务1：LED0 闪烁 (低优先级)
 * 等待信号量，收到后翻转 LED0
 *----------------------------------------------------------*/
static void Task1_LED(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(xBinarySem, portMAX_DELAY) == pdTRUE) {
            LED0_Toggle();
        }
    }
}

/*-----------------------------------------------------------
 * 任务2：周期发送信号量 (高优先级)
 * 每 500ms 释放一次信号量，同时翻转 LED1
 *----------------------------------------------------------*/
static void Task2_Sender(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(500));
        xSemaphoreGive(xBinarySem);
        LED1_Toggle();
    }
}

/*-----------------------------------------------------------
 * FreeRTOS Hook 函数
 *----------------------------------------------------------*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;);
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;);
}

/*-----------------------------------------------------------
 * 主函数
 *----------------------------------------------------------*/
int main(void)
{
    /* 解锁外设写保护 */
    LL_PERIPH_WE(LL_PERIPH_ALL);

    /* 系统时钟初始化 */
    SystemClock_Init();

    /* LED 初始化 */
    LED_Init();

    /* 锁回写保护 */
    LL_PERIPH_WP(LL_PERIPH_ALL);

    /* SystemView 初始化 - 必须在创建任务之前 */
    SEGGER_SYSVIEW_Conf();

    /* 创建二值信号量 */
    xBinarySem = xSemaphoreCreateBinary();
    configASSERT(xBinarySem != NULL);

    /* 创建任务 */
    xTaskCreate(Task1_LED,    "LED",    128, NULL, 2, &xTask1Handle);
    xTaskCreate(Task2_Sender, "Sender", 128, NULL, 3, &xTask2Handle);

    /* 启动调度器 */
    vTaskStartScheduler();

    for (;;);
}
