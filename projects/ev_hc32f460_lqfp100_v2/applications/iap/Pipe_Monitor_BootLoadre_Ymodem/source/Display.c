/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\Display.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-23       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "Display.h"
#include "math.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "Font.h"
#include "OLED.h"
#include "main.h"
//#include "drv_RTC.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

void func_sprintf_metricsystem(signed short * buffer, float fb5h,char fb9h,char fbah)
{
	//signed short * ptr_fb2h = buffer;
	//signed short sBuf[20] = {0};
	short len_fbm4h;
	short i_fbm6h;
	float fbmeh;
	short int_fbmeh;
	float fbm16h;
	short int_fbm16h;
	float fbm1eh;
	char buf_fbm3c[30];
	unsigned char buf_fbm5a[30];
	float fValue;
	long lValue;
	
	if(fb9h > 3)
	{
		fb9h = 3;
	}
	
	switch(fb9h)
	{
	case 0:
		if(fb5h >= 0.00)
		{
		fb5h = (float)(0.5 + fb5h);
		}
		else
		{
			fb5h = (float)(-0.5 + fb5h);
		}
		break;
	case 1:
		if(fb5h >= 0.00)
		{
		fb5h = (float)(0.05 + fb5h);
		}
		else
		{
			fb5h = (float)(-0.05 + fb5h);
		}
		break;
	case 2:
		if(fb5h >= 0.00)
		{
		fb5h = (float)(0.005 + fb5h);
		}
		else
		{
			fb5h = (float)(-0.005 + fb5h);
		}
		break;
	case 3:
		if(fb5h >= 0.00)
		{
		fb5h = (float)(0.0005 + fb5h);
		}
		else
		{
			fb5h = (float)(-0.0005 + fb5h);
		}
		break;
	default:
		break;
	}

	if((9999999.000000>=fb5h))
	{
		#if 1
		i_fbm6h = 0;
		while(i_fbm6h < 7)
		{
			fbmeh = (0.000100 +fb5h);		//fb5h+0.000100
			if((0.0>fbmeh))
			{	// 0.0 > fbmeh, that is ,fbmeh < 0
				fbm1eh = -fbmeh;	
			}
			else
			{
				fbm1eh = fbmeh;				
			}
			fbm16h = fbm1eh;
			fValue = pow(10.0,i_fbm6h);
			fValue = fbm16h / fValue;
			lValue = (long)fValue;
			lValue = lValue % 10;
			buf_fbm5a[i_fbm6h+5] = (char)lValue;
			//sBuf[i_fbm6h+5] = (char)lValue;
			i_fbm6h++;
		}
		#endif
		i_fbm6h = 3;
		while(i_fbm6h >= 0)
		{
			fbmeh = (0.000100+ fb5h);
			if((0.0>fbmeh))
			{
				fbm16h = -fbmeh;
			}
			else
			{
				fbm16h = fbmeh;
			}
			fbm1eh = fbm16h;
			fValue = pow(10.0,4-i_fbm6h);
			fValue = fbm1eh * fValue;
			lValue = (long)fValue;
			lValue = lValue % 10;
			buf_fbm5a[i_fbm6h] = (char)lValue;
			//sBuf[i_fbm6h] = (char)lValue;
			//buf_fbm5a[i_fbm6h]  = (char)((long)((fbm1eh*(pow(10.0,4-i_fbm6h))) )%10);
			i_fbm6h--;
					
		}
		buf_fbm5a[4] = 0x2E;		//'.'
		i_fbm6h = 0x000B;
		while(i_fbm6h > 5)
		{
			if(buf_fbm5a[i_fbm6h] > 0)
			{
				break;
			}
			i_fbm6h--;
		}
		int_fbm16h = i_fbm6h;
		int_fbmeh = 0;
		if(!(0.0<=fb5h))
		{
			buf_fbm3c[int_fbmeh++] = 0x2D;		//	'-'
		}
		i_fbm6h = int_fbm16h;
		while(i_fbm6h > 4)
		{
			buf_fbm3c[int_fbmeh] = buf_fbm5a[i_fbm6h]+0x30;			//0x03 + 0x30 == '3'
			i_fbm6h--;
			int_fbmeh++;			
		}
		if(fb9h > 0)
		{
			buf_fbm3c[int_fbmeh++] = 0x2E;			//'.'
		}
		i_fbm6h = 3;
		while(i_fbm6h >= (4 - fb9h))
		{
			buf_fbm3c[int_fbmeh] = buf_fbm5a[i_fbm6h]+0x30;
			int_fbmeh++;
			i_fbm6h--;
		}
		buf_fbm3c[int_fbmeh] = 0;
		len_fbm4h = int_fbmeh;
	}
	else
	{
		sprintf(&buf_fbm3c[0],"%.3e",(double)fb5h);
		len_fbm4h = strlen(&buf_fbm3c[0]);		
	}
	i_fbm6h = 0;
	while(i_fbm6h < len_fbm4h)
	{
		if(buf_fbm3c[i_fbm6h] == 0x2E)
		{		//'.'
			switch(fbah)
			{
			case 1:	
				buffer[i_fbm6h] = 0x031B;
				break;
			case 5:
				buffer[i_fbm6h] = 0x0532;
				break;
			case 4:
				buffer[i_fbm6h] = 0x040A;
				break;
			case 7:
				buffer[i_fbm6h] = 0x070A;
				break;
			case 0x0E:
				buffer[i_fbm6h] = 0x0E0A;
				break;
			default:				
				break;
			}
		}
		else if(buf_fbm3c[i_fbm6h] == 0x2D)
		{		//'-'
			switch(fbah)
			{
			case 1:	
				buffer[i_fbm6h] = 0x031F;
				break;
			case 5:
				buffer[i_fbm6h] = 0x050C;
				break;
			case 4:
				buffer[i_fbm6h] = 0x040E;
				break;
			case 7:
				buffer[i_fbm6h] = 0x0716;
				break;
			case 0x0E:
				buffer[i_fbm6h] = 0x0E0E;
				break;
			default:				
				break;
			}
		}
		else if(buf_fbm3c[i_fbm6h] == 0x65)
		{// 	'e'
			switch(fbah)
			{
			case 1:	
				buffer[i_fbm6h] = 0x0304;
				break;
			case 5:
				buffer[i_fbm6h] = 0x052E;
				break;
			case 4:
				buffer[i_fbm6h] = 0x052E;
				break;
			default:				
				break;
			}
		}
		else if(buf_fbm3c[i_fbm6h] >= 0x30 && buf_fbm3c[i_fbm6h] <= 0x39)
		{
			buffer[i_fbm6h] = (((short)fbah)<<8) | (buf_fbm3c[i_fbm6h]- 0x30);
		}
		else
		{
			buffer[i_fbm6h] = 0x031A;
		}
		i_fbm6h++;
	}
	buffer[i_fbm6h] = 0xFFFF;
}

//开机界面，显示SK LOGO
void func_display_PowerOn_Menu(void)
{
	clear_screen();
	display_128x64((uint8_t *)&g_sSKLogo[0]);
	
}

//设备启动中...
void func_Device_Starting_View_Show(unsigned char ucFlag1, unsigned char ucFlag2, unsigned char ucFlag3)
{
	signed short sShowArr[10] = {0};
	clear_screen();
	sShowArr[0] = 0x000F;
	sShowArr[1] = 0x0010;
	sShowArr[2] = 0x002B;
	sShowArr[3] = 0x002C;
	sShowArr[4] = 0X002D;
	sShowArr[5] = 0x0532;
	sShowArr[6] = 0x0532;
	sShowArr[7] = 0x0532;
	sShowArr[8] = 0xFFFF;
	func_display_string(0,30,&sShowArr[0]);
	#if 0
	sShowArr[0] = (ucFlag1 / 10) | 0x0500; //十位
	sShowArr[1] = (ucFlag1 % 10) | 0x0500; //个位
	sShowArr[2] = 0xFFFF;
	func_display_string(0,10,&sShowArr[0]);
	sShowArr[0] = (ucFlag2 / 10) | 0x0500; //十位
	sShowArr[1] = (ucFlag2 % 10) | 0x0500; //个位
	sShowArr[2] = 0xFFFF;
	func_display_string(0,50,&sShowArr[0]);
	sShowArr[0] = (ucFlag3 / 10) | 0x0500; //十位
	sShowArr[1] = (ucFlag3 % 10) | 0x0500; //个位
	sShowArr[2] = 0xFFFF;
	func_display_string(50,50,&sShowArr[0]);
	#endif
	func_Display_128x64();
}

//设备升级中...
void func_Device_Upgrade_View_Show(void)
{
	signed short sShowArr[12] = {0};
	unsigned short usCnt = 0;

	usCnt = (unsigned short)((float)gul_IAP_Upgrade_Current_Size / (float)gul_IAP_Upgrade_Total_Size * 100.0);
	clear_screen();
	sShowArr[0] = 0x000F;
	sShowArr[1] = 0x0010;
	sShowArr[2] = 0x002E;
	sShowArr[3] = 0x002F;
	sShowArr[4] = 0X002D;
	sShowArr[5] = (usCnt / 10) | 0x0500;
	sShowArr[6] = (usCnt % 10) | 0x0500;
	sShowArr[7] = 0x0528;
	sShowArr[8] = 0x0532;
	sShowArr[9] = 0x0532;
	sShowArr[10] = 0x0532;
	sShowArr[11] = 0xFFFF;
	func_display_string(0,30,&sShowArr[0]);
	func_Display_128x64();
}

//设备升级结果
void func_Device_UpgradeResult_View_Show(unsigned char ucResult)
{
	signed short sShowArr[12] = {0};
	
	clear_screen();
	sShowArr[0] = 0x000F;
	sShowArr[1] = 0x0010;
	sShowArr[2] = 0x002E;
	sShowArr[3] = 0x002F;
	if(ucResult == 0)	//成功
	{
		sShowArr[4] = 0x0030;
		sShowArr[5] = 0x0031;
	}
	else	//失败
	{
		sShowArr[4] = 0x0032;
		sShowArr[5] = 0x0033;
	}
	sShowArr[6] = 0xFFFF;
	func_display_string(0,30,&sShowArr[0]);
	func_Display_128x64();
}


//显示授权通过界面
void func_display_Authorize_Menu(void)
{
	
}

//显示日期时间，电池电量等状态信息界面
void func_DateTime_Battery_Status_View_Show()
{
	
}

//屏幕最下方显示当前设备状态
void func_Device_Status_View_Show()
{
	
}

//显示设备未经过认证界面
void func_Not_Certified_View_Show(void)
{
	
}

//显示测量界面：液位
void func_Measure_WaterLevel_View_Show()
{
	
}

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/



/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
