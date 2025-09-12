/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\OLED.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-02       Joe             First version
 @endverbatim

 */
#ifndef __OLED_H__
#define __OLED_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
//#include "drv_IIC.h"
#include "hc32_ll_gpio.h"
#include "com.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
//OLED显示
#define  OLED_IIC_SDA_PORT  (GPIO_PORT_D)
#define  OLED_IIC_SDA_PIN   (GPIO_PIN_01)
#define  OLED_IIC_SDA_H            (GPIO_SetPins(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN))
#define  OLED_IIC_SDA_L            (GPIO_ResetPins(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN))

#define  OLED_IIC_SCL_PORT  (GPIO_PORT_D)
#define  OLED_IIC_SCL_PIN   (GPIO_PIN_00)
#define  OLED_IIC_SCL_H            (GPIO_SetPins(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN))
#define  OLED_IIC_SCL_L            (GPIO_ResetPins(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN))

#define  OLED_IIC_SdaState             GPIO_ReadInputPins(OLED_IIC_SDA_PORT,OLED_IIC_SDA_PIN)
#define  OLED_IIC_SclState             GPIO_ReadOutputPins(OLED_IIC_SCL_PORT,OLED_IIC_SCL_PIN)

#ifdef HW_VERSION_V1_1
#define PWRLCM_GPIO_PORT	GPIO_PORT_E
#define PWRLCM_GPIO_PIN	  GPIO_PIN_11
#else
#define PWRLCM_GPIO_PORT	GPIO_PORT_E
#define PWRLCM_GPIO_PIN	  GPIO_PIN_10
#endif

#define PWRLCM_PIN_OPEN()	GPIO_SetPins(PWRLCM_GPIO_PORT,PWRLCM_GPIO_PIN)
#define PWRLCM_PIN_CLOSE()	GPIO_ResetPins(PWRLCM_GPIO_PORT, PWRLCM_GPIO_PIN)

#define LCMRS_PORT	GPIO_PORT_D
#define LCMRS_PIN	GPIO_PIN_04

#define LCMRS_PIN_SET	GPIO_SetPins(LCMRS_PORT, LCMRS_PIN)
#define LCMRS_PIN_RESET	GPIO_ResetPins(LCMRS_PORT, LCMRS_PIN)

#define LCMRST_PORT	GPIO_PORT_D
#define LCMRST_PIN	GPIO_PIN_03

#define LCMRST_PIN_SET	GPIO_SetPins(LCMRST_PORT, LCMRST_PIN)
#define LCMRST_PIN_RESET	GPIO_ResetPins(LCMRST_PORT, LCMRST_PIN)

#define LCMCS_PORT	GPIO_PORT_D
#define LCMCS_PIN	GPIO_PIN_02

#define LCMCS_PIN_SET	GPIO_SetPins(LCMCS_PORT, LCMCS_PIN)
#define LCMCS_PIN_RESET	GPIO_ResetPins(LCMCS_PORT, LCMCS_PIN)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void OLED_Write_CMD(uint8_t ucmd);
extern void OLED_Write_Data(uint8_t uData);
extern uint8_t drv_OLED_Init(void);
extern void OLED_Test(uint8_t ucType);
extern void func_display_string(uint8_t X_dot,uint8_t Y_dot,signed short *text);

extern void func_OLED_PowerUp_Init(void);
extern void func_OLED_PowerDown_DeInit(void);

extern void clear_screen();
extern void display_128x64(uint8_t *dp) ;
extern void func_Display_128x64();

//extern SystemPataSt *pst_OLEDSystemPara;
extern unsigned char guc_OLED_Buf[128][8];	//OLED显示数据缓存;8*8=64行128列

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
