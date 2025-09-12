/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\source\main.h
 * @brief This file contains the including files of main routine.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-12       Joe             First version
 @endverbatim

 */
#ifndef __MAIN_H__
#define __MAIN_H__

#include "hc32_ll.h"
#include "hc32_ll_gpio.h"
#include "hc32_ll_clk.h"
#include "hc32_ll_sram.h"
#include "hc32f460.h"

#include "flash.h"
#include "com.h"
#include "ymodem.h"
#include "4G_EC200U.h"

#ifdef HW_VERSION_V1_1
#define WATCHDOG_GPIO_PORT   (GPIO_PORT_C)
#define WATCHDOG_GPIO_PIN    (GPIO_PIN_10)
#else
#define WATCHDOG_GPIO_PORT   (GPIO_PORT_B)
#define WATCHDOG_GPIO_PIN    (GPIO_PIN_08)
#endif

#ifdef HW_VERSION_V1_1
#define SENSOR_PWRVO1_GPIO_PORT  GPIO_PORT_A
#define SENSOR_PWRVO1_GPIO_PIN   GPIO_PIN_09
#else
#define SENSOR_PWRVO1_GPIO_PORT  GPIO_PORT_B
#define SENSOR_PWRVO1_GPIO_PIN   GPIO_PIN_13
#endif
#define SENSOR_PWRVO1_PIN_CLOSE()  GPIO_ResetPins(SENSOR_PWRVO1_GPIO_PORT,SENSOR_PWRVO1_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define SENSOR_PWRVO2_GPIO_PORT  GPIO_PORT_C
#define SENSOR_PWRVO2_GPIO_PIN   GPIO_PIN_08
#else
#define SENSOR_PWRVO2_GPIO_PORT  GPIO_PORT_B
#define SENSOR_PWRVO2_GPIO_PIN   GPIO_PIN_12
#endif
#define SENSOR_PWRVO2_PIN_CLOSE()  GPIO_ResetPins(SENSOR_PWRVO2_GPIO_PORT,SENSOR_PWRVO2_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define PWRUP_CTLUP1_GPIO_PORT  GPIO_PORT_C
#define PWRUP_CTLUP1_GPIO_PIN   GPIO_PIN_06
#else
#define PWRUP_CTLUP1_GPIO_PORT  GPIO_PORT_D
#define PWRUP_CTLUP1_GPIO_PIN   GPIO_PIN_14
#endif
#define PWRUP_CTLUP1_PIN_CLOSE()  GPIO_ResetPins(PWRUP_CTLUP1_GPIO_PORT,PWRUP_CTLUP1_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define PWRUP_CTLUP2_GPIO_PORT  GPIO_PORT_D
#define PWRUP_CTLUP2_GPIO_PIN   GPIO_PIN_14
#else
#define PWRUP_CTLUP2_GPIO_PORT  GPIO_PORT_C
#define PWRUP_CTLUP2_GPIO_PIN   GPIO_PIN_06
#endif
#define PWRUP_CTLUP2_PIN_CLOSE()  GPIO_ResetPins(PWRUP_CTLUP2_GPIO_PORT,PWRUP_CTLUP2_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define PWRSNR_GPIO_PORT  GPIO_PORT_B
#define PWRSNR_GPIO_PIN   GPIO_PIN_07
#else
#define PWRSNR_GPIO_PORT  GPIO_PORT_H
#define PWRSNR_GPIO_PIN   GPIO_PIN_02
#endif
#define PWRSNR_PIN_CLOSE()  GPIO_ResetPins(PWRSNR_GPIO_PORT,PWRSNR_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define PWRBLE_GPIO_PORT GPIO_PORT_C
#define PWRBLE_GPIO_PIN  GPIO_PIN_02
#else
#define PWRBLE_GPIO_PORT GPIO_PORT_C
#define PWRBLE_GPIO_PIN  GPIO_PIN_03
#endif
#define PWRBLE_PIN_CLOSE() GPIO_SetPins(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN)

#define PWRLORA_GPIO_PORT              (GPIO_PORT_D)
#define PWRLORA_GPIO_PIN               (GPIO_PIN_13)
#define PWRLORA_PIN_CLOSE()      GPIO_ResetPins(PWRLORA_GPIO_PORT, PWRLORA_GPIO_PIN)

//BD模块引脚定义
#ifdef HW_VERSION_V1_1
#define PWRBD_GPIO_PORT  GPIO_PORT_D
#define PWRBD_GPIO_PIN   GPIO_PIN_06
#else
#define PWRBD_GPIO_PORT  GPIO_PORT_B
#define PWRBD_GPIO_PIN   GPIO_PIN_09
#endif
#define PWRBD_PIN_CLOSE() GPIO_ResetPins(PWRBD_GPIO_PORT, PWRBD_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define BDRST_GPIO_PORT  GPIO_PORT_D
#define BDRST_GPIO_PIN   GPIO_PIN_05
#else
#define BDRST_GPIO_PORT  GPIO_PORT_D
#define BDRST_GPIO_PIN   GPIO_PIN_15
#endif
#define BDRST_PIN_CLOSE() GPIO_ResetPins(BDRST_GPIO_PORT, BDRST_GPIO_PIN)

//RFID_NFC  复位等引脚定义
#ifdef HW_VERSION_V1_1
#define PWRRF_GPIO_PORT	GPIO_PORT_E
#define PWRRF_GPIO_PIN	GPIO_PIN_09
#else
#define PWRRF_GPIO_PORT	GPIO_PORT_D
#define PWRRF_GPIO_PIN	GPIO_PIN_05
#endif
#define PWRRF_PIN_CLOSE() GPIO_ResetPins(PWRRF_GPIO_PORT,PWRRF_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define BATTERY_PWRCHK_GPIO_PORT		GPIO_PORT_B
#define BATTERY_PWRCHK_GPIO_PIN		GPIO_PIN_12
#else
#define BATTERY_PWRCHK_GPIO_PORT		GPIO_PORT_C
#define BATTERY_PWRCHK_GPIO_PIN		GPIO_PIN_09
#endif
#define BATTERY_PWRCHK_CLOSE()		(GPIO_ResetPins(BATTERY_PWRCHK_GPIO_PORT, BATTERY_PWRCHK_GPIO_PIN))

extern unsigned long gul_IAP_Upgrade_Total_Size;	//升级文件总大小
extern unsigned long gul_IAP_Upgrade_Current_Size;	//升级文件当前大小
extern unsigned char guc_TestCnt;

extern void func_WatchDog_Refresh(void);

#endif /* __MAIN_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
