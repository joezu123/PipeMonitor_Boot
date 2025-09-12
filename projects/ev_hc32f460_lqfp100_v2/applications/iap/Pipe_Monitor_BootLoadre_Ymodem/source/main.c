/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\source\main.c
 * @brief Main program of IAP Boot.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-12       Joe             First version
 @endverbatim

 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "main.h"
#include "OLED.h"
#include "Display.h"
#include "W25Q128.h"
#include "string.h"
#include "stdio.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
/* Communication timeout */
#define IAP_COM_WAIT_TIME               (2000UL)

/* Print On/Off */
//#define IAP_PRINT_INFO                  (DDL_ON)

/* BSP XTAL Configure definition */
#define BSP_XTAL_PORT                   (GPIO_PORT_H)
#define BSP_XTAL_PIN                    (GPIO_PIN_00 | GPIO_PIN_01)



/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint32_t JumpAddr;
static func_ptr_t JumpToApp;

unsigned long gul_IAP_Upgrade_Total_Size = 1;	//升级文件总大小
unsigned long gul_IAP_Upgrade_Current_Size = 0;	//升级文件当前大小
unsigned char guc_TestCnt = 0;;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  SysTick interrupt handler function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    SysTick_IncTick();

    __DSB();  /* Arm Errata 838869 */
}

/**
 * @brief  Systick De-Initialize.
 * @param  None
 * @retval None
 */
void SysTick_DeInit(void)
{
    SysTick->CTRL  = 0UL;
    SysTick->LOAD  = 0UL;
    SysTick->VAL   = 0UL;
}

/**
 * @brief  IAP clock initialize.
 *         Set board system clock to PLL@200MHz
 * @param  None
 * @retval None
 */
void IAP_CLK_Init(void)
{
    stc_clock_xtal_init_t stcXtalInit;
    stc_clock_pll_init_t  stcMpllInit;

    /* Set bus clk div. */
    CLK_SetClockDiv(CLK_BUS_CLK_ALL, (CLK_HCLK_DIV1 | CLK_EXCLK_DIV2 | CLK_PCLK0_DIV1 |
                                      CLK_PCLK1_DIV2 | CLK_PCLK2_DIV4 | CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2));

    GPIO_AnalogCmd(BSP_XTAL_PORT, BSP_XTAL_PIN, ENABLE);
    (void)CLK_XtalStructInit(&stcXtalInit);
    /* Config Xtal and enable Xtal */
    stcXtalInit.u8Mode  = CLK_XTAL_MD_OSC;
    stcXtalInit.u8Drv   = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);

    (void)CLK_PLLStructInit(&stcMpllInit);
    /* MPLL config (XTAL / pllmDiv * plln / PllpDiv = 200M). */
    stcMpllInit.PLLCFGR = 0UL;
    stcMpllInit.PLLCFGR_f.PLLM = 1UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLN = 50UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLP = 2UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLQ = 2UL - 1UL;
    stcMpllInit.PLLCFGR_f.PLLR = 2UL - 1UL;
    stcMpllInit.u8PLLState = CLK_PLL_ON;
    stcMpllInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;
    (void)CLK_PLLInit(&stcMpllInit);
    /* Wait MPLL ready. */
    while (SET != CLK_GetStableStatus(CLK_STB_FLAG_PLL)) {
    }

    /* sram init include read/write wait cycle setting */
    SRAM_Init();
    SRAM_SetWaitCycle(SRAM_SRAM_ALL, SRAM_WAIT_CYCLE1, SRAM_WAIT_CYCLE1);
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    /* 3 cycles for 126MHz ~ 200MHz */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT3);
    /* flash read wait cycle setting */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    /* Switch driver ability */
    (void)PWC_HighSpeedToHighPerformance();
    /* Switch system clock source to MPLL. */
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_PLL);
}

/**
 * @brief  IAP clock De-Initialize.
 * @param  None
 * @retval None
 */
void IAP_CLK_DeInit(void)
{
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_MRC);
    /* Switch driver ability */
    (void)PWC_HighPerformanceToHighSpeed();
    /* Set bus clk div. */
    CLK_SetClockDiv(CLK_BUS_CLK_ALL, (CLK_HCLK_DIV1 | CLK_EXCLK_DIV1 | CLK_PCLK0_DIV1 |
                                      CLK_PCLK1_DIV1 | CLK_PCLK2_DIV1 | CLK_PCLK3_DIV1 | CLK_PCLK4_DIV1));
    CLK_PLLCmd(DISABLE);
    CLK_XtalCmd(DISABLE);
    /* sram init include read/write wait cycle setting */
    SRAM_SetWaitCycle(SRAM_SRAM_ALL, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    /* 0 cycles */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT0);
    /* flash read wait cycle setting */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE0);
}


/**
 * @brief  IAP peripheral initialize.
 * @param  None
 * @retval None
 */
void IAP_PeriphInit(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    EFM_FWMC_Cmd(ENABLE);
    /* Init Peripheral */
    IAP_CLK_Init();
    SysTick_Init(1000U);
    COM_Init();
    
    EC200U_4G_Module_GPIO_Init();
}

/**
 * @brief  IAP peripheral de-initialize.
 * @param  None
 * @retval None
 */
void IAP_PeriphDeinit(void)
{
    //关闭4G电源
    GPIO_ResetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
    /* De-Init Peripheral */
    COM_DeInit();
    BSP_W25QXX_DeInit();
    SysTick_DeInit();
    IAP_CLK_DeInit();
    /* Peripheral registers write protected */
    EFM_FWMC_Cmd(DISABLE);
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
}

/**
 * @brief  Print a string.
 * @param  None
 * @retval None
 */
void IAP_SendString(uint8_t *pu8Str)
{
#if IAP_PRINT_INFO == DDL_ON
    uint32_t u32Len = 0;

    while (pu8Str[u32Len] != '\0') {
        u32Len++;
    }
    COM_SendData(pu8Str, u32Len);
#else
    (void)pu8Str;
#endif
}

/**
 * @brief  Jump from boot to app function.
 * @param  [in] u32Addr                 APP address
 * @retval LL_ERR                       APP address error
 */
int32_t IAP_JumpToApp(uint32_t u32Addr)
{
    uint32_t u32StackTop = *((__IO uint32_t *)u32Addr);

    /* Check stack top pointer. */
    if ((u32StackTop > SRAM_BASE) && (u32StackTop <= (SRAM_BASE + SRAM_SIZE))) {
        //IAP_PeriphDeinit();
        /* Jump to user application */
        JumpAddr = *(__IO uint32_t *)(u32Addr + 4U);
        JumpToApp = (func_ptr_t)JumpAddr;
        /* Initialize user application's Stack Pointer */
        __set_MSP(u32StackTop);
        JumpToApp();
    }

    return LL_ERR;
}

/**
 * @brief  IAP check app.
 * @param  None
 * @retval None
 */
void IAP_CheckApp(void)
{
    #if 0
    if ((APP_UPGRADE_FLAG != *(__IO uint32_t *)APP_UPGRADE_FLAG_ADDR)) {
        if ((APP_EXIST_FLAG == *(__IO uint32_t *)APP_EXIST_FLAG_ADDR)) {
            if (LL_OK != IAP_JumpToApp(IAP_APP_ADDR)) {
                IAP_SendString((uint8_t *)"\r\nJump to app failed \r\n");
            }
        }
    }
    #endif
    //与服务器通讯，确认是否需要升级
    
}

/**
 * @brief  Download a file via serial port.
 * @param  None
 * @retval None
 */
void YModem_Download(void)
{
    uint8_t fileName[FILE_NAME_LEN] = {0};
    uint32_t fileSize = 0;
    int32_t i32Ret;
#if IAP_PRINT_INFO == DDL_ON
    uint8_t fileSizeStr[FILE_SIZE_LEN] = {0};
#endif

    i32Ret = YModem_Receive(fileName, &fileSize);
    if (i32Ret == YMODEM_COM_OK) {
#if IAP_PRINT_INFO == DDL_ON
        YModem_Int2Str(fileSizeStr, fileSize);
        IAP_SendString((uint8_t *)"\r\nfileName: ");
        IAP_SendString(fileName);
        IAP_SendString((uint8_t *)"\r\n");
        IAP_SendString((uint8_t *)"fileSize: ");
        IAP_SendString(fileSizeStr);
        IAP_SendString((uint8_t *)"\r\n");
        IAP_SendString((uint8_t *)"Download completed!\r\n");
#endif
    } else {
        IAP_SendString((uint8_t *)"\r\nFailed to receive the file \r\n");
    }
}

/**
 * @brief  Upload a file via serial port.
 * @param  None
 * @retval None
 */
void YModem_Upload(void)
{
    uint8_t u8Temp;

    u8Temp = YModem_Transmit((uint8_t *)IAP_APP_ADDR, (uint8_t *)"App.bin", IAP_APP_SIZE);
    if (u8Temp != YMODEM_COM_OK) {
        IAP_SendString((uint8_t *)"\r\nError occurred while transmitting file \r\n");
    } else {
        IAP_SendString((uint8_t *)"\r\nUpload completed!\r\n");
    }
}


void func_Board_Power_And_Control_GPIO_Init(void)
{
	stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);

	stcGpioInit.u16PinState = PIN_STAT_RST;
    stcGpioInit.u16PinDir = PIN_DIR_OUT;

    #ifndef HW_VERSION_V1_1
	GPIO_Init(GPIO_PORT_A, GPIO_PIN_08, &stcGpioInit);
    #endif
	GPIO_Init(SENSOR_PWRVO1_GPIO_PORT, SENSOR_PWRVO1_GPIO_PIN, &stcGpioInit);
	GPIO_Init(SENSOR_PWRVO2_GPIO_PORT, SENSOR_PWRVO2_GPIO_PIN, &stcGpioInit);
	GPIO_Init(PWRUP_CTLUP1_GPIO_PORT, PWRUP_CTLUP1_GPIO_PIN, &stcGpioInit);
	GPIO_Init(PWRUP_CTLUP2_GPIO_PORT, PWRUP_CTLUP2_GPIO_PIN, &stcGpioInit);
	GPIO_Init(PWRSNR_GPIO_PORT, PWRSNR_GPIO_PIN, &stcGpioInit);

    //蓝牙电源控制引脚
    GPIO_Init(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN, &stcGpioInit);
    //DCE电源控制引脚
    GPIO_Init(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN, &stcGpioInit);
    //LORA电源控制引脚
    GPIO_Init(PWRLORA_GPIO_PORT, PWRLORA_GPIO_PIN, &stcGpioInit);
    //北斗电源控制引脚
    GPIO_Init(PWRBD_GPIO_PORT, PWRBD_GPIO_PIN, &stcGpioInit);
    //BD 复位引脚
    GPIO_Init(BDRST_GPIO_PORT, BDRST_GPIO_PIN, &stcGpioInit);
    //OLED电源控制引脚
    GPIO_Init(PWRLCM_GPIO_PORT, PWRLCM_GPIO_PIN, &stcGpioInit);
    //RF电源控制引脚
    GPIO_Init(PWRRF_GPIO_PORT, PWRRF_GPIO_PIN, &stcGpioInit);
    //电池电量检测电源控制引脚
    GPIO_Init(BATTERY_PWRCHK_GPIO_PORT, BATTERY_PWRCHK_GPIO_PIN, &stcGpioInit);

    #ifndef HW_VERSION_V1_1
	GPIO_ResetPins(GPIO_PORT_A, GPIO_PIN_08);	//关闭3.8V电源
	SysTick_Delay(100);
	GPIO_SetPins(GPIO_PORT_A, GPIO_PIN_08);	//打开3.8V电源
    #endif

	//GPIO_ResetPins(GPIO_PORT_H, GPIO_PIN_02);	//关闭传感器电源
    PWRSNR_PIN_CLOSE();	//关闭传感器电源
	//Ddl_Delay1ms(100);
	//PWRSNR_PIN_OPEN();	//打开传感器电源

    #if 1
    //GPIO_ResetPins(GPIO_PORT_B, GPIO_PIN_13);    //关闭传感器电源
    SENSOR_PWRVO1_PIN_CLOSE();    //关闭传感器电源
    //GPIO_ResetPins(GPIO_PORT_D, GPIO_PIN_14);    //关闭传感器电源
    PWRUP_CTLUP1_PIN_CLOSE();    //关闭传感器电源
    //GPIO_ResetPins(GPIO_PORT_B, GPIO_PIN_12);    //关闭传感器电源
    SENSOR_PWRVO2_PIN_CLOSE();    //关闭传感器电源
    //GPIO_ResetPins(GPIO_PORT_C, GPIO_PIN_06);    //关闭传感器电源
    PWRUP_CTLUP2_PIN_CLOSE();    //关闭传感器电源
    //GPIO_SetPins(GPIO_PORT_C, GPIO_PIN_03);	//关闭蓝牙模块电源
    PWRBLE_PIN_CLOSE();	//关闭蓝牙模块电源
    //GPIO_ResetPins(GPIO_PORT_D, GPIO_PIN_13);	//关闭LoRa模块电源
    PWRLORA_PIN_CLOSE();	//关闭LoRa模块电源
    //GPIO_ResetPins(GPIO_PORT_B, GPIO_PIN_09);	//关闭北斗模块电源
    PWRBD_PIN_CLOSE();	//关闭北斗模块电源
    //GPIO_ResetPins(GPIO_PORT_C, GPIO_PIN_09);	//关闭电池电量检测电源
    BATTERY_PWRCHK_CLOSE();	//关闭电池电量检测电源
    //GPIO_ResetPins(GPIO_PORT_E, GPIO_PIN_10);	//关闭OLED电源
    PWRLCM_PIN_CLOSE();	//关闭OLED电源
    //GPIO_ResetPins(GPIO_PORT_D, GPIO_PIN_05);	//关闭RF模块电源
    PWRRF_PIN_CLOSE();	//关闭RF模块电源
    #endif

	//return 0;
}

void drv_WatchDog_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);

	stcGpioInit.u16PinState = PIN_STAT_RST;
    stcGpioInit.u16PinDir = PIN_DIR_OUT;

    GPIO_Init(WATCHDOG_GPIO_PORT, WATCHDOG_GPIO_PIN, &stcGpioInit);
}

void func_WatchDog_Refresh(void)
{
    GPIO_SetPins(WATCHDOG_GPIO_PORT,WATCHDOG_GPIO_PIN);
    DDL_DelayMS(10);
    GPIO_ResetPins(WATCHDOG_GPIO_PORT,WATCHDOG_GPIO_PIN);
}

 //软件复位
 void func_System_Soft_Reset(void)
 {
     __set_FAULTMASK(1);
     NVIC_SystemReset();
 }
/**
 * @brief  Main function of Boot.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t ucUpdateFlag = 0;
    //uint8_t ucDataPtFlag = 0;
    uint8_t uc4GInitFlag = 0;
    //uint8_t ucWaitCnt = 0;
    //uint8_t ucUpgradeCheckArr[50] = {0};
    //uint8_t ucDataPtArr[50] = {0};

    IAP_PeriphInit();
    /* Check app validity */
    func_Board_Power_And_Control_GPIO_Init();
    drv_WatchDog_Init();
    BSP_W25QXX_Init();
    //读取FLASH中设备参数信息
	BSP_W25QXX_Read(SYSTEM_PARA_ADDR,(uint8_t*)&gs_DevicePara.cDeviceID[0],sizeof(SysDeviceParaSt));

    //OLED初始化
    drv_OLED_Init();
    //OLED_Test(1);
    func_Device_Starting_View_Show(9,9,9);
    //DDL_DelayMS(1000);
    uc4GInitFlag = EC200U_4G_Module_Init();
    func_WatchDog_Refresh();
    #if 0
    if(uc4GInitFlag == 0)   //4G模块初始化成功
    {
        sprintf((char *)ucUpgradeCheckArr,"UpgradeCheck/%s",gs_DevicePara.cDeviceID);
        sprintf((char *)ucDataPtArr,"dataPt/%s",gs_DevicePara.cDeviceID);
        //等待服务器返回是否需要升级的数据
        while(ucWaitCnt < 25)
        {
            if(m_RecvFlag == 1)   //接收到服务器返回的数据
            {
                m_RecvFlag = 0;
                if(strstr((char *)m_au8RxBuf, (char *)ucUpgradeCheckArr) != NULL)   //接收到订阅的确认升级主题数据
                {
                    if(strstr((char *)m_au8RxBuf, "\"res\":0") != NULL)   //需要升级
                    {
                        ucUpdateFlag = 2;
                    }
                    else    //不需要升级
                    {
                        ucUpdateFlag = 0;
                        break;
                    }
                    
                }
                else if(strstr((char *)m_au8RxBuf, (char *)ucDataPtArr) != NULL)   //接收到订阅的数据透传主题数据
                {
                    if(strstr((char *)m_au8RxBuf, "\"req\":\"666666#Y#\"") != NULL)   //数据透传
                    {
                        //处理数据透传
                        func_Publish_Topic_DataPt_Cmd();
                        ucDataPtFlag = 1;
                        break;
                    }
                }
            }
            DDL_DelayMS(200);
            ucWaitCnt++;
        }
    }
    #endif
    IAP_CheckApp();
    /* Control Menu */
    //IAP_SendString((uint8_t *)"\r\n============= Bootloader Menu =============\r\n");
    //IAP_SendString((uint8_t *)" 1: Download execute program to the Flash \r\n");
    //IAP_SendString((uint8_t *)" 2: Upload execute program from the Flash \r\n");
    //IAP_SendString((uint8_t *)" 3: Jump to the application \r\n");
    #if 0
    for (;;) {
        if (LL_OK == COM_RecvData(&keyValue, 1, IAP_COM_WAIT_TIME)) {
            switch (keyValue) {
                case '1':
                    IAP_SendString((uint8_t *)"\r\nEnter download mode \r\n");
                    YModem_Download();
                    break;
                case '2':
                    IAP_SendString((uint8_t *)"\r\nEnter upload mode \r\n");
                    YModem_Upload();
                    break;
                case '3':
                    if (LL_OK != IAP_JumpToApp(IAP_APP_ADDR)) {
                        IAP_SendString((uint8_t *)"\r\nJump to app failed \r\n");
                    }
                    break;
            }
            keyValue = 0;
        }
    }
    #else
    //if((ucUpdateFlag == 2) && (ucDataPtFlag == 1))   //表示需要升级
    if(uc4GInitFlag == 0)   //4G模块初始化成功
    {
        //IAP_SendString((uint8_t *)"\r\nEnter download mode \r\n");
        //YModem_Download();
        func_Device_Upgrade_View_Show();
        ucUpdateFlag = func_4G_Module_Connect_HTTP(guc_URLArr, gus_URLArrLen, gul_UpdateFileSize); //获取升级文件
        func_Device_UpgradeResult_View_Show(ucUpdateFlag);
        func_4G_Up_Upgrade_Result(ucUpdateFlag);
        if(ucUpdateFlag != 0)
        {
            IAP_PeriphDeinit();
            func_System_Soft_Reset();    //软件复位
        }
    }
    func_WatchDog_Refresh();
    IAP_PeriphDeinit();
    func_WatchDog_Refresh();
    if (LL_OK != IAP_JumpToApp(IAP_APP_ADDR)) 
    {
        OLED_Test(2);
        DDL_DelayMS(1000);
        IAP_SendString((uint8_t *)"\r\nJump to app failed \r\n");
    }
    
    #endif
    
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
