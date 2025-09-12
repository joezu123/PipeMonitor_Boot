/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\Display.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-23       Joe             First version
 @endverbatim

 */
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
//#include "drv_IIC.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void func_sprintf_metricsystem(signed short * buffer, float fb5h,char fb9h,char fbah);
extern void func_display_Authorize_Menu(void);
extern void func_display_PowerOn_Menu(void);

extern void func_DateTime_Battery_Status_View_Show();
extern void func_Not_Certified_View_Show(void);
extern void func_Measure_WaterLevel_View_Show();
extern void func_Device_Status_View_Show();
extern void func_Device_Starting_View_Show(unsigned char ucFlag1, unsigned char ucFlag2, unsigned char ucFlag3);
extern void func_Device_Upgrade_View_Show(void);
extern void func_Device_UpgradeResult_View_Show(unsigned char ucResult);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
