/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\OLED.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-02       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "OLED.h"
#include "Display.h"
#include "Font.h"
#include "string.h"
#include "hc32_ll_utility.h"
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
unsigned char guc_OLED_Buf[128][8] = {0};	//OLED显示数据缓存; 8*8=64行128列
//SystemPataSt *pst_OLEDSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**************************************************
*uint8_t drv_mcu_OLED_IIC_Init(void)
*入口参数：无
*出口参数：0: init success; 1: init failed
*功能说明：I2C 引脚初始化
***************************************************/
uint8_t drv_mcu_OLED_IIC_Init(void)  
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_gpio_init_t stcPortInit;

    /* configure structure initialization */
    GPIO_StructInit(&stcPortInit);

    /* Flash NSS */
	stcPortInit.u16PinState = PIN_STAT_RST;
    stcPortInit.u16PinDir = PIN_DIR_OUT;
	//stcPortInit.enPinOType = Pin_OType_Od;
    GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);		
	GPIO_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
    	  
	OLED_IIC_SDA_H;//SDA拉低
	OLED_IIC_SCL_H;//SCL拉高

	return 0;
}

//设置IIC SDA 引脚为输出引脚
void drv_mcu_OLED_IIC_SDA_PIN_SET_OUTPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_gpio_init_t stcPortInit;

    /* configure structure initialization */
    GPIO_StructInit(&stcPortInit);
	stcPortInit.u16PinDir = PIN_DIR_OUT;
	//stcPortInit.enPinOType = Pin_OType_Od;
    GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);	
}

//设置IIC SDA 引脚为输入引脚
void drv_mcu_OLED_IIC_SDA_PIN_SET_INPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_gpio_init_t stcPortInit;

    /* configure structure initialization */
    GPIO_StructInit(&stcPortInit);
	stcPortInit.u16PinDir = PIN_DIR_IN;
	//stcPortInit.enPinOType = Pin_OType_Od;
    GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);	
}

/*********************IIC停止信号***********************/
void drv_OLED_IIC_Stop(void)
{	
	OLED_IIC_SCL_L;  //
	DDL_DelayUS(5);
	OLED_IIC_SDA_L;
	DDL_DelayUS(5);
	OLED_IIC_SCL_H;
	DDL_DelayUS(5);
	OLED_IIC_SDA_H;
}

/*********************IIC起始信号***********************/
void drv_OLED_IIC_Start(void)
{
	OLED_IIC_SDA_H;
	DDL_DelayUS(5);
	OLED_IIC_SCL_H;
	DDL_DelayUS(5);	
	OLED_IIC_SCL_L;//SCL拉低
	DDL_DelayUS(5);	
	OLED_IIC_SDA_L;//SDA拉低
	DDL_DelayUS(5);
}

/************MCU向XYC_ALS发送应答/非应答信号**********************/
void drv_OLED_IIC_ACKorNACK(uint8_t uAck)
{	
	DDL_DelayUS(4);
	OLED_IIC_SCL_H;//SCL拉高
	DDL_DelayUS(4);	
	OLED_IIC_SCL_L;//SCL拉低
	OLED_IIC_SDA_H;      //SDA拉高

	OLED_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    DDL_DelayUS(5);	
    
    if(uAck==1) 
	{
        OLED_IIC_SDA_H;
    } 
    else 
	{
        OLED_IIC_SDA_L;
    }
    DDL_DelayUS(10);
    OLED_IIC_SCL_H;                  //    P90:TxD_SCL     _________|~~~~~~~~~~
    DDL_DelayUS(10);

    OLED_IIC_SCL_L;                //     ~~~~~|______________
    DDL_DelayUS(5);
}

/****************************************************************************/
/**
  * @brief  I2C获取应答信号
  * @param   
  * @retval 1 表示没有停止信号 
  *         0 表示有停止信号
  */
/****************************************************************************/
uint8_t drv_OLED_IIC_Checkack(void)
{  
	uint8_t tmp_flag = 0;

    OLED_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    DDL_DelayUS(5);
    OLED_IIC_SDA_H;          //SDA  ______/------ 
    DDL_DelayUS(5);

///*---------------------------------------------------------------------------*/
    drv_mcu_OLED_IIC_SDA_PIN_SET_INPUT();       //I/O 设置为输入
	
    OLED_IIC_SCL_H;          //SCL   ------------
    DDL_DelayUS(5);
	
    if(OLED_IIC_SdaState) 
	{
        tmp_flag = 1;
    }

    OLED_IIC_SCL_L;         //SCL   ------\______
    DDL_DelayUS(5);
	
    drv_mcu_OLED_IIC_SDA_PIN_SET_OUTPUT();

    return(tmp_flag);
}

/*************** 发送一个字节数据子函数 **********************/
uint8_t drv_OLED_IIC_Sendbyte(unsigned char Sdata)
{ 
	char temp,i;
	uint8_t tmp_flag = 0;
	temp = Sdata;
	for(i=0;i<8;i++)
	{
		OLED_IIC_SCL_L; 
		DDL_DelayUS(2);	
    	if((temp&0x80) == 0x80)
		{
			OLED_IIC_SDA_H;
		}
		else
		{
			OLED_IIC_SDA_L;
		}
			
		DDL_DelayUS(2);
    	OLED_IIC_SCL_H; 
		DDL_DelayUS(4);
		temp = temp<<1;
	}

	tmp_flag = drv_OLED_IIC_Checkack();
    tmp_flag = (!tmp_flag) & 0x01;

    return(tmp_flag);
}

void OLED_Write_CMD(uint8_t ucmd)
{
	#if 0
	drv_OLED_IIC_Start();
	drv_OLED_IIC_Sendbyte(0x78);
	drv_OLED_IIC_Sendbyte(0x00);
	drv_OLED_IIC_Sendbyte(ucmd);
	drv_OLED_IIC_Stop();
	#else
	uint8_t i;

	LCMCS_PIN_RESET;
	LCMRS_PIN_RESET;
	for(i=0; i<8; i++)
	{
		OLED_IIC_SCL_L;
		//Ddl_Delay1us(2);
		if(ucmd & 0x80)
		{
			OLED_IIC_SDA_H;
		}
		else
		{
			OLED_IIC_SDA_L;
		}
		//Ddl_Delay1us(2);
		OLED_IIC_SCL_H;
		//Ddl_Delay1us(4);
		ucmd = ucmd << 1;
	}
	LCMCS_PIN_SET;
	#endif
}

void OLED_Write_Data(uint8_t uData)
{
	#if 0
	drv_OLED_IIC_Start();
	drv_OLED_IIC_Sendbyte(0x78);
	drv_OLED_IIC_Sendbyte(0x40);
	drv_OLED_IIC_Sendbyte(uData);
	drv_OLED_IIC_Stop();
	#else
	uint8_t i;
	LCMCS_PIN_RESET;
	LCMRS_PIN_SET;
	for(i=0; i<8; i++)
	{
		OLED_IIC_SCL_L;
		//Ddl_Delay1us(2);
		if(uData & 0x80)
		{
			OLED_IIC_SDA_H;
		}
		else
		{
			OLED_IIC_SDA_L;
		}
		//Ddl_Delay1us(2);
		OLED_IIC_SCL_H;
		//Ddl_Delay1us(4);
		uData = uData << 1;
	}
	LCMCS_PIN_SET;
	#endif
}

uint8_t drv_OLED_Init(void)
{
	stc_gpio_init_t stcPortInit;

	GPIO_StructInit(&stcPortInit);

	stcPortInit.u16PinDir = PIN_DIR_OUT;

    GPIO_Init(PWRLCM_GPIO_PORT, PWRLCM_GPIO_PIN, &stcPortInit);

	GPIO_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);

	GPIO_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);

	GPIO_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);

	drv_mcu_OLED_IIC_Init();

	//pst_OLEDSystemPara = GetSystemPara();

	memset(guc_OLED_Buf, 0, sizeof(guc_OLED_Buf));

	//开启OLED电源
	PWRLCM_PIN_CLOSE();
	DDL_DelayMS(100);
	PWRLCM_PIN_OPEN();

	LCMCS_PIN_RESET;

	//复位芯片
	LCMRST_PIN_RESET;
	DDL_DelayMS(100);
	LCMRST_PIN_SET;
	DDL_DelayMS(100);

	OLED_Write_CMD(0xAE);	//关显示
	OLED_Write_CMD(0x40);	//起始行
	OLED_Write_CMD(0x81);	//微调对比度，不可更改
	OLED_Write_CMD(0x32);	//微调对比度的值，可设置范围0x00~0xff ,默认0x32
	OLED_Write_CMD(0xc8); 	//行扫描顺序：从上到下 
	OLED_Write_CMD(0xa1); 	//列扫描顺序：从左到右 
	OLED_Write_CMD(0xa6); 	//正常显示模式 
	OLED_Write_CMD(0xa8); 	//duty 设置 
	OLED_Write_CMD(0x3f); 	//duty=1/64 
	OLED_Write_CMD(0xd3); 	//显示偏移 
	OLED_Write_CMD(0x00); 
	OLED_Write_CMD(0xd5); 	//晶振频率 
	OLED_Write_CMD(0xa0); 	//0x80 
	OLED_Write_CMD(0xd9); 	//Set Pre-Charge Period 
	OLED_Write_CMD(0xf1); 
	OLED_Write_CMD(0xda); 	//sequential configuration 
	OLED_Write_CMD(0x12); 
 	OLED_Write_CMD(0x91); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0xaf); 	//开显示

	return 0;
}

//OLED模块开启电源及初始化配置
void func_OLED_PowerUp_Init(void)
{
	stc_gpio_init_t stcPortInit;
    GPIO_StructInit(&stcPortInit);
    stcPortInit.u16PinDir = PIN_DIR_OUT;
    GPIO_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	GPIO_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	GPIO_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);

	//开启OLED电源
	PWRLCM_PIN_CLOSE();
	DDL_DelayMS(100);
	PWRLCM_PIN_OPEN();

	LCMCS_PIN_RESET;

	//复位芯片
	LCMRST_PIN_RESET;
	DDL_DelayMS(100);
	LCMRST_PIN_SET;
	DDL_DelayMS(100);

	OLED_Write_CMD(0xAE);	//关显示
	OLED_Write_CMD(0x40);	//起始行
	OLED_Write_CMD(0x81);	//微调对比度，不可更改
	OLED_Write_CMD(0x32);	//微调对比度的值，可设置范围0x00~0xff ,默认0x32
	OLED_Write_CMD(0xc8); 	//行扫描顺序：从上到下 
	OLED_Write_CMD(0xa1); 	//列扫描顺序：从左到右 
	OLED_Write_CMD(0xa6); 	//正常显示模式 
	OLED_Write_CMD(0xa8); 	//duty 设置 
	OLED_Write_CMD(0x3f); 	//duty=1/64 
	OLED_Write_CMD(0xd3); 	//显示偏移 
	OLED_Write_CMD(0x00); 
	OLED_Write_CMD(0xd5); 	//晶振频率 
	OLED_Write_CMD(0xa0); 	//0x80 
	OLED_Write_CMD(0xd9); 	//Set Pre-Charge Period 
	OLED_Write_CMD(0xf1); 
	OLED_Write_CMD(0xda); 	//sequential configuration 
	OLED_Write_CMD(0x12); 
 	OLED_Write_CMD(0x91); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0xaf); 	//开显示
}

void func_OLED_PowerDown_DeInit(void)
{
	stc_gpio_init_t stcPortInit;
    GPIO_StructInit(&stcPortInit);
    stcPortInit.u16PinDir = PIN_DIR_IN;
	stcPortInit.u16PinAttr = PIN_ATTR_ANALOG;
	GPIO_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	GPIO_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	GPIO_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
	//LCMCS_PIN_SET;
	PWRLCM_PIN_CLOSE();	//关闭OLED电源
}

void lcd_address(uint8_t page,uint8_t column) 
{ 
 	column=column-1; //我们平常所说的第 1 列，在 LCD 驱动 IC 里是第 0 列。所以在这里减去1. 
 	page=page-1; 
 	OLED_Write_CMD(0xb0+page); //设置页地址。每页是 8 行。一个画面的 64 行被分成 8 个页。我们平常所说的第 1 页，在LCD 驱动 IC 里是第 0 页，所以在这里减去 1 
 	OLED_Write_CMD(((column>>4)&0x0f)+0x10); //设置列地址的高 4 位 
 	OLED_Write_CMD(column&0x0f); //设置列地址的低 4 位 
}

//全屏清屏 
void clear_screen() 
{ 
 	unsigned char i,j; 
	stc_gpio_init_t stcPortInit;
	GPIO_StructInit(&stcPortInit);
	stcPortInit.u16PinDir = PIN_DIR_OUT;
	stcPortInit.u16PinAttr = PIN_ATTR_DIGITAL;
	//PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	GPIO_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	GPIO_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
 	for(j=0;j<8;j++) 
 	{ 
 		lcd_address(1+j,1); 
 		for(i=0;i<128;i++) 
 		{ 
 			OLED_Write_Data(0x00); 
 		} 
 	} 
	 memset(guc_OLED_Buf, 0, sizeof(guc_OLED_Buf));
} 
 
//显示 128x64 点阵图像 
void display_128x64(uint8_t *dp) 
{ 
 	uint8_t i,j; 
	stc_gpio_init_t stcPortInit;
	GPIO_StructInit(&stcPortInit);
	stcPortInit.u16PinDir = PIN_DIR_IN;
	stcPortInit.u16PinAttr = PIN_ATTR_ANALOG;
 	for(j=0;j<8;j++) 
 	{ 
 		lcd_address(j+1,1); 
 		for (i=0;i<128;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
 		} 
 	} 
	//PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	GPIO_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	GPIO_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
}

void func_Display_128x64()
{
	uint8_t i,j; 
	stc_gpio_init_t stcPortInit;
	GPIO_StructInit(&stcPortInit);
	stcPortInit.u16PinDir = PIN_DIR_IN;
	stcPortInit.u16PinAttr = PIN_ATTR_ANALOG;
	for(j=0xb0; j<0xb8; j++)
	{
		/*选择LCD的地址*/
		OLED_Write_CMD(j);
		OLED_Write_CMD(0x10);
		OLED_Write_CMD(0x00);

		for(i=0; i<128; i++) 
		{
			OLED_Write_Data(guc_OLED_Buf[i][j-0xb0]);
		}
		
	}
	//PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	GPIO_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	GPIO_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	GPIO_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
}

//显示 32x32 点阵图像、汉字、生僻字或 32x32 点阵的其他图标 
void display_graphic_32x32(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
	uint8_t i,j; 
 	for(j=0;j<4;j++) 
 	{ 
 		lcd_address(page+j,column); 
 		for (i=0;i<32;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
 		} 
	} 
} 
 
//显示 16x16 点阵图像、汉字、生僻字或 16x16 点阵的其他图标 
void display_graphic_16x16(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
	uint8_t i,j; 
 	for(j=0;j<2;j++) 
 	{ 
 		lcd_address(page+j,column); 
 		for (i=0;i<16;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
		} 
 	} 
} 
 
//显示 8x16 点阵图像、ASCII, 或 8x16 点阵的自造字符、其他图标 
void display_graphic_8x16(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
	uint8_t i,j; 
 	for(j=0;j<2;j++) 
 	{ 
 		lcd_address(page+j,column); 
 		for (i=0;i<8;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
 		} 
 	} 
} 

void OLED_Test(uint8_t ucType)
{
	uint8_t i,j;
	//uint16_t k;
	//uint8_t ucValue = 0;
	//signed short sTestArr[11] = {0};
	clear_screen();
	memset(guc_OLED_Buf,0,sizeof(guc_OLED_Buf));
	switch (ucType)
	{
	case 0:
		for(i=0; i<4; i++)
		{
			for(j=0; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64();
		break;
	case 1:
		for(i=4; i<8; i++)
		{
			for(j=0; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64();
		break;
	case 2:
		for(i=0; i<8; i++)
		{
			for(j=0; j<64; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64();
		break;
	case 3:
		for(i=0; i<8; i++)
		{
			for(j=64; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64();
		break;
	case 4:
		for(i=0; i<8; i++)
		{
			for(j=0; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64();
		break;
	case 5:	//SK LOGO
		display_128x64((uint8_t *)&g_sSKLogo[0]);
		break;
	case 6:	
		//姿态数据显示
		
		break;
	case 7:	
		//蓝牙数据
		
		break;
	case 8:	//显示BD数据
		
		break;
	default:
		break;
	}
	func_Display_128x64();
}




/* -------------------------------------------------------------------------------------------------------------
 *
 *         LCD 坐标原点位于左上角, 纵向向下为Y 8个字节, 横向128列
 * @brief  根据菜单文字编码数组将字库点阵存入显示屏RAM
 * @param   X_dot:0..127
			Y_dot:0..63
			in_DisplayString_pt:菜单文字编码数组
 * @retval 
 --------------------------------------------------------------------------------------------------------------- */
void func_display_string(uint8_t X_dot,uint8_t Y_dot,signed short *text)
{
	signed short j,sPosi;
	signed short sSize,sCode;
	signed short sWidth,Y_Bias,fbm19h;
	//unsigned char ucX_dot_H, ucX_dot_L;
	//unsigned short s_xSum = 0;
	signed char fbm1h,fbm2h,fbm3h;

	sPosi = 0;
	while(text[sPosi] != -1)
	{        
		//根据高8位判断显示像素大小，进行显示
		sSize = (text[sPosi]>>8)&0x00FF;	
		sCode =	text[sPosi]&0x00FF;
		#if 0
		ucX_dot_H = (( X_dot + s_xSum + 4  )>>4)|0x10;
		ucX_dot_L = ( X_dot + s_xSum + 4  )&0x0f;
		#endif
		switch (sSize)
		{
		case 0:	//汉字
			j = 0;
			sWidth = 12;
			while(j < sWidth)
			{
				Y_Bias = Y_dot % 8;
				//fbm13h = (x_fbm5h<<3)	+ (y_fbm7h/8);
				if(Y_Bias < 4)
				{
					fbm1h = g_sint8_Font_16x12_12864[sCode][j*2]<<(Y_Bias);
					
					guc_OLED_Buf[X_dot][Y_dot/8] |= fbm1h;		
					
					fbm1h = ((unsigned char)g_sint8_Font_16x12_12864[sCode][j*2])>>(8-Y_Bias)|(g_sint8_Font_16x12_12864[sCode][j*2+1] << Y_Bias);	
					
					guc_OLED_Buf[X_dot][Y_dot/8+1] |= fbm1h;	
					
				}
				else
				{
					fbm1h = g_sint8_Font_16x12_12864[sCode][j*2] << Y_Bias;
					
					guc_OLED_Buf[X_dot][Y_dot/8] |= fbm1h;
					
					fbm1h = ((unsigned char)g_sint8_Font_16x12_12864[sCode][j*2])>>(8-Y_Bias) | (g_sint8_Font_16x12_12864[sCode][j*2+1]<<Y_Bias);
					
					guc_OLED_Buf[X_dot][Y_dot/8+1] |= fbm1h;	
					
					fbm1h = ((unsigned char)g_sint8_Font_16x12_12864[sCode][j*2+1])>>(8-Y_Bias);
					
					guc_OLED_Buf[X_dot][Y_dot/8+2] |= fbm1h;			
					
				}
				X_dot++;
				j++;
			}
			break;
		case 5:	//数字+字母
			
			j = 0;
			if(sCode == 0x32 || sCode == 0x33)// '.' ' '
			{
				sWidth = 4;
			}
			else if(sCode == 0x46)	// ':'
			{
				sWidth = 7;
			}
			else
			{
				sWidth = 10;
			}
			
			while(j < sWidth)
			{	
				fbm2h = g_sint8_Font_16x10_12864[sCode][j * 2];
				fbm3h = g_sint8_Font_16x10_12864[sCode][j * 2+1];
				
				//		[0x0045,0x004B] is GODA logo
				if(sCode < 0x0045 || sCode> 0x004B)
				{
					fbm2h = (((unsigned char)fbm2h)>>2) | (fbm3h << 6) ;		
					fbm3h = ((unsigned char)fbm3h)>>2;
				}
				
				Y_Bias = Y_dot % 8;
				
				if(Y_Bias <= 3)
				{
					fbm19h = 0x00FF;
					
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;		
					fbm1h = (guc_OLED_Buf[X_dot][Y_dot/8+1] & (fbm19h << (13-(8-Y_Bias))));
				
					
					fbm1h |= ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));
				
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;		
				
				}
				else
				{
					fbm19h = 0x00FF;
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
					fbm1h |= fbm2h<<Y_Bias;			
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;

					fbm1h = ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));	
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;			

					fbm1h = guc_OLED_Buf[X_dot][Y_dot/8 + 2] & (fbm19h << (5-(8-Y_Bias)));
							
					fbm1h |= (((unsigned char)fbm3h) >> (8-Y_Bias));
					guc_OLED_Buf[X_dot][Y_dot/8 + 2] = fbm1h;	
				}

				X_dot++;
				j++;	
			}
			break;
		case 7:	//ASCII 6X12
			j = 0;

			if(sCode == 0x00 || sCode == 0x0E || sCode == 0x1A)// '.' ' '
			{
				sWidth = 4;
			}
			else
			{
				sWidth = 6;
			}
			
			while(j < sWidth)
			{	
				fbm2h = g_sint8_Font_6x12_AsciiDot[sCode][j * 2];
				fbm3h = g_sint8_Font_6x12_AsciiDot[sCode][j * 2+1];
				
				//		[0x0045,0x004B] is GODA logo
				//if(sCode < 0x0045 || sCode> 0x004B)
				{
				//	fbm2h = (((unsigned char)fbm2h)>>2) | (fbm3h << 6) ;		
				//	fbm3h = ((unsigned char)fbm3h)>>2;
				}
				
				Y_Bias = Y_dot % 8;
				
				if(Y_Bias <= 3)
				{
					fbm19h = 0x00FF;
					
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;		
					fbm1h = (guc_OLED_Buf[X_dot][Y_dot/8+1] & (fbm19h << (13-(8-Y_Bias))));
				
					
					fbm1h |= ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));
				
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;		
				
				}
				else
				{
					fbm19h = 0x00FF;
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
					fbm1h |= fbm2h<<Y_Bias;			
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;

					fbm1h = ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));	
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;			

					fbm1h = guc_OLED_Buf[X_dot][Y_dot/8 + 2] & (fbm19h << (5-(8-Y_Bias)));
							
					fbm1h |= (((unsigned char)fbm3h) >> (8-Y_Bias));
					guc_OLED_Buf[X_dot][Y_dot/8 + 2] = fbm1h;	
				}

				X_dot++;
				j++;	
			}
			break;
		default:
			break;
		}
		
		sPosi++;
	}
}

/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
