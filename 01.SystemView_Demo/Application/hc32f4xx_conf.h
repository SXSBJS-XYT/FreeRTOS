/**
 * @file  hc32f4xx_conf.h
 * @brief HC32F460 DDL Rev3.3.0 驱动模块配置文件
 *        选择需要使用的 LL 驱动模块，按需开启
 */

#ifndef __HC32F4XX_CONF_H__
#define __HC32F4XX_CONF_H__

/*******************************************************************************
 * LL 驱动模块使能开关
 * DDL_ON  = 使能
 * DDL_OFF = 禁用
 ******************************************************************************/
#define LL_ICG_ENABLE                       (DDL_ON)
#define LL_UTILITY_ENABLE                   (DDL_ON)
#define LL_CLK_ENABLE                       (DDL_ON)
#define LL_FCG_ENABLE                       (DDL_ON)
#define LL_EFM_ENABLE                       (DDL_ON)
#define LL_GPIO_ENABLE                      (DDL_ON)
#define LL_INTERRUPTS_ENABLE                (DDL_ON)
#define LL_PWC_ENABLE                       (DDL_ON)
#define LL_SRAM_ENABLE                      (DDL_ON)

/* 暂时不用的外设，按需开启 */
#define LL_ADC_ENABLE                       (DDL_OFF)
#define LL_AES_ENABLE                       (DDL_OFF)
#define LL_AOS_ENABLE                       (DDL_OFF)
#define LL_CAN_ENABLE                       (DDL_OFF)
#define LL_CMP_ENABLE                       (DDL_OFF)
#define LL_CRC_ENABLE                       (DDL_OFF)
#define LL_DBGC_ENABLE                      (DDL_OFF)
#define LL_DCU_ENABLE                       (DDL_OFF)
#define LL_DMA_ENABLE                       (DDL_OFF)
#define LL_EMB_ENABLE                       (DDL_OFF)
#define LL_EVENT_PORT_ENABLE                (DDL_OFF)
#define LL_FCM_ENABLE                       (DDL_OFF)
#define LL_HASH_ENABLE                      (DDL_OFF)
#define LL_I2C_ENABLE                       (DDL_OFF)
#define LL_I2S_ENABLE                       (DDL_OFF)
#define LL_INTERRUPTS_SHARE_ENABLE          (DDL_ON)
#define LL_KEYSCAN_ENABLE                   (DDL_OFF)
#define LL_MPU_ENABLE                       (DDL_OFF)
#define LL_OTS_ENABLE                       (DDL_OFF)
#define LL_QSPI_ENABLE                      (DDL_OFF)
#define LL_RMU_ENABLE                       (DDL_OFF)
#define LL_RTC_ENABLE                       (DDL_OFF)
#define LL_SDIOC_ENABLE                     (DDL_OFF)
#define LL_SPI_ENABLE                       (DDL_OFF)
#define LL_SWDT_ENABLE                      (DDL_OFF)
#define LL_TMR0_ENABLE                      (DDL_OFF)
#define LL_TMR4_ENABLE                      (DDL_OFF)
#define LL_TMR6_ENABLE                      (DDL_OFF)
#define LL_TMRA_ENABLE                      (DDL_OFF)
#define LL_TRNG_ENABLE                      (DDL_OFF)
#define LL_USART_ENABLE                     (DDL_OFF)
#define LL_USB_ENABLE                       (DDL_OFF)
#define LL_WDT_ENABLE                       (DDL_OFF)

/*******************************************************************************
 * XTAL 晶振频率 (根据板子实际硬件)
 ******************************************************************************/
#define XTAL_VALUE                          (24000000UL)     /* 24MHz */

#endif /* __HC32F4XX_CONF_H__ */
