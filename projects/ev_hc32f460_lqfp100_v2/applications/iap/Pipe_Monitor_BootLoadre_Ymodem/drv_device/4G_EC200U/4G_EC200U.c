/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\drivers\device_drv\4G_EC200U\4G_EC200U.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-12       Joe             First version
 @endverbatim

 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "4G_EC200U.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "flash.h"
#include "main.h"
#include "Display.h"
#include "time.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EC200U_BUF_SIZE             (300)
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
en_4G_Module_Init_State gE_4G_Module_Init_CMD = Module_INIT_STATE_MAX;
en_4G_Mudule_Connect_HTTP_State gE_4G_Module_Connect_HTTP_CMD = Module_HTTP_ATE0_CMD;
uint8_t ucSendBuf[EC200U_BUF_SIZE] = {0};
uint8_t guc_URLArr[200] = {0};	//用于存储URL地址
uint16_t gus_URLArrLen = 0; //URL网址链接长度
uint32_t gul_UpdateFileSize = 0;	//升级文件大小
uint8_t guc_NewVersion[15] = {0};	//新版本号
uint8_t guc_StartDateTime[20] = {0};	//新版本开始时间

//uint8_t ucRecvBuf[1030] = {0};
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
//判断接收的数据数组中是否包含指定的字符串
//0->包含; 1->不包含
unsigned char func_Array_Find_Str(char *ucRecvBuf, unsigned short usDataLen, char *ucCheckBuf, unsigned short usCheckLen, unsigned short* usPosi)
{
	unsigned short i = 0;
    //unsigned char j = 0;
	unsigned char ucRes = 1;
	for(i = 0; i < usDataLen; i++)
	{
        //if(i >= 1020)
        //{
        //    j = 1;
        //}
		if(ucRecvBuf[i] == ucCheckBuf[0])
		{
			if(memcmp(ucRecvBuf+i, ucCheckBuf, usCheckLen) == 0)
			{
                *usPosi = i;	//记录位置
				ucRes = 0;	// 找到
				break;
			}
		}
	}

	return ucRes;
}

/**
 * @brief  4G EC200U Module GPIO Initialize.
 * @param  None
 * @retval None
 */
void EC200U_4G_Module_GPIO_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinState = PIN_STAT_SET;
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN, &stcGpioInit);
    (void)GPIO_Init(EC200U_4G_MODULE_RST_PORT, EC200U_4G_MODULE_RST_PIN, &stcGpioInit);   

    GPIO_ResetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
    SysTick_Delay(500);
    GPIO_SetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
    SysTick_Delay(500);
}

//拼接获取升级检查命令
uint16_t func_Get_UpgradeCheck_CMD(uint8_t *ucDataArr)
{
    uint8_t ucTempArr[300] = {0};
    uint16_t usDataLen = 0;
    time_t now;
    struct tm tm;
    sscanf((char*)guc_StartDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":%d,\"msgId\":%ld,\"data\":{\"SWVersion\":\"%s\"}}", gs_DevicePara.cDeviceID,
                                                                                        24,
                                                                                        (long)now,
                                                                                        gs_DevicePara.cDeviceSWVersion);

    usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;      
}

//拼接返回升级结果命令
uint16_t func_Get_UpgradeResult_CMD(uint8_t *ucDataArr, unsigned char ucResult)
{
    uint8_t ucTempArr[300] = {0};
    uint16_t usDataLen = 0;
    char c_Result[10] = {0};
    char c_Result1[20] = {0};
    time_t now;
    struct tm tm;
    sscanf((char*)guc_StartDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    if(ucResult == 0)
    {
        memcpy(c_Result, "true", 4); //升级成功
        memcpy(c_Result1, "\"\"", 2); //升级失败
    }
    else
    {
        memcpy(c_Result, "false", 5); //升级失败
        memcpy(c_Result1, "\"Time out\"", 10); //升级失败
    }
    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":%d,\"msgId\":%ld,\"data\":{\"success\":%s,\"errorReason\":%s,\"version\":%s}}", gs_DevicePara.cDeviceID,
                                                                                        24,
                                                                                        (long)now,
                                                                                        c_Result,
                                                                                        c_Result1,
                                                                                        guc_NewVersion);

    usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;      
}


//拼接获取数据透传命令
uint16_t func_Get_DataPt_CMD(uint8_t *ucDataArr)
{
    uint8_t ucTempArr[300] = {0};
    uint16_t usDataLen = 0;
    struct tm tm;
    time_t now;
    sscanf((char*)guc_StartDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":%d,\"msgId\":%ld,\"data\":{\"ack\":\"%s;%s\"}}", gs_DevicePara.cDeviceID,
                                                                                        24,
                                                                                        (long)now,
                                                                                        gs_DevicePara.cDeviceID,
                                                                                        gs_DevicePara.cDeviceSWVersion);

    usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;      
}

//发布主题-数据透传
uint8_t func_Publish_Topic_DataPt_Cmd(void)
{
    uint8_t ucRetryCnt = 0;
    uint16_t usRecvTimeOutCnt = 0;
    uint8_t ucRecvCheckData[50] = {0};
    uint16_t usSendDataLen = 0;
    uint16_t usDataPosi = 0;
    memset(ucSendBuf, 0, EC200U_BUF_SIZE);
    memset(ucRecvCheckData, 0, 50);
    m_u16RxLen = 0;
    memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);

    usSendDataLen = func_Get_UpgradeCheck_CMD(ucSendBuf);
    memset(ucSendBuf, 0, EC200U_BUF_SIZE);
    sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/dataPt/%s\",%d\r\n",gs_DevicePara.cDeviceID, usSendDataLen);
    //sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/iVOw212I78/ZCJ2025042801/thing/property/post\",%d\r\n",usSendDataLen);
    usSendDataLen = strlen((char *)ucSendBuf);
    COM_SendData(ucSendBuf, usSendDataLen);
    SysTick_Delay(400);
    m_u16RxLen = 0;
    memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);
    memset(ucSendBuf, 0, EC200U_BUF_SIZE);
    usSendDataLen = func_Get_UpgradeCheck_CMD(ucSendBuf);
    memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
    usSendDataLen += 2;
    sprintf((char *)ucRecvCheckData, "QMTPUBEX");
    usRecvTimeOutCnt = 0;
    while((m_RecvFlag == 0))
    {
        DDL_DelayMS(20);
        usRecvTimeOutCnt++;
        if(usRecvTimeOutCnt >= 500)
        {
            return 2;
        }
    }
    if(m_u16RxLen > 0)
    {
        usRecvTimeOutCnt = 0;
        while(m_au8RxBuf[m_u16RxLen-1] == 0x00)
        {
            DDL_DelayMS(20);
            usRecvTimeOutCnt++;
            if(usRecvTimeOutCnt >= 500)
            {
                return 2;
            }
        }
    }
    m_RecvFlag = 0;
    
    //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
    if(func_Array_Find_Str((char *)m_au8RxBuf,m_u16RxLen,(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usDataPosi) == 0) //接收到的数据中包含OK
    {
        if(gE_4G_Module_Init_CMD == 7)
        {
            return 0;
        }
        else
        {
            gE_4G_Module_Init_CMD++;
        }
    }
    else
    {
        ucRetryCnt++;
        if(ucRetryCnt >= 3) //重复发送三次
        {
            //u8Temp = 0;
            return 3;
        }
    }
    return 1;
}

/**
 * @brief  4G EC200U Module AT CMD Initialize.
 * @param  None
 * @retval 0: module init failed 1: no update 2: need update
 */
uint8_t EC200U_4G_Module_Configuration_Init(void)
{
    //int32_t i32Ret = 0;
    //int32_t i32Ret1 = 0;
    uint8_t u8Temp = 1;
    uint8_t ucRetryCnt = 0;
    uint16_t usRecvTimeOutCnt = 0;
    uint16_t usSendDataLen = 0;
    uint8_t ucRecvCheckData[50] = {0};
    uint16_t usDataPosi = 0;
    unsigned short usPosi1 = 0;
    uint8_t ucDataLenArr[10] = {0};
    uint16_t i = 0;
    uint8_t ucFlag = 0;
    uint8_t j = 0;
    char *cTemp;
    //unsigned char ucDisFlag = 0;

    while(u8Temp == 1)
    {
        memset(ucSendBuf, 0, EC200U_BUF_SIZE);
        //memset(ucRecvBuf, 0, 1030);
        memset(ucRecvCheckData, 0, 50);
        m_u16RxLen = 0;
        memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);

        switch (gE_4G_Module_Init_CMD)
        {
        case Module_TEST_AT_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "AT\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_TEST_ATE0_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "ATE0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_SIM_CARD_STATE_CMD: //查询SIM卡状态
            (void)strcpy((char *)ucSendBuf, "AT+CPIN?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CPIN: READY");
            break;
        case Module_QUERY_SIGNAL_STRENGTH_CMD: //查询信号强度
            (void)strcpy((char *)ucSendBuf, "AT+CSQ\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CSQ:");
            break;
        case Module_QUERY_PS_DOMAIN_REG_STATE_CMD://查询PS域注册状态：0：未注册，1/5：注册，2：正在搜索
            //(void)strcpy((char *)ucSendBuf, "AT+CREG?\r\n");    //CS域，只使用在2G网络上
            (void)strcpy((char *)ucSendBuf, "AT+CGREG?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGREG: 0,1");
            break;
        case Module_ACTIVATE_NETWORK_CMD: //激活网络
            (void)strcpy((char *)ucSendBuf, "AT+CGATT=1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_NETWORK_ACTIVATE_STATE_CMD: //查询网络激活状态
            (void)strcpy((char *)ucSendBuf, "AT+CGATT?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGATT: 1");
            break;
        case Module_QUERY_LOCAL_DATE_TIME_CMD: //查询本地日期时间
            (void)strcpy((char *)ucSendBuf, "AT+QLTS=2\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            func_WatchDog_Refresh();
            break;
        //MQTT配置
        case Module_SET_DATA_FORMAT_CMD: //设置数据格式
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"recv/mode\",0,0,1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SET_MQTT_KEEPALIVE_TIME_CMD:    //心跳时间建议60s~300s.这里设置120s
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"keepalive\",0,120\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SET_MQTT_VERSION_CMD:   //设置MQTT 版本
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"version\",0,4\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_OPEN_MQTT_INTERFACE_CMD: //打开物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTOPEN=0,\"218.85.5.161\",7243\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "QMTOPEN");
            break;
        case Module_CONN_MQTT_INTERFACE_CMD: //连接物联网云端口
            (void)sprintf((char *)ucSendBuf, "AT+QMTCONN=0,\"%s\",\"xfgd\",\"xfgd@1234\"\r\n",gs_DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_UPGRADECHECK_CMD:    //订阅主题-升级确认
            //data/down/0040/0004/UpgradeCheck/MQ0001003A123456(序列号)
            //(void)strcpy((char *)ucSendBuf, "AT+QMTSUB=0,1,\"$data/down/0040/0004/UpgradeCheck/");
            //strcat(ucSendBuf,&gs_DevicePara.cDeviceID[0]); //将设备ID添加到主题中
            //strcat(ucSendBuf,"\",2\r\n");
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/UpgradeCheck/%s\",2\r\n",gs_DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        //case Module_SUBSCRIBE_TOPIC_DATAPT_CMD: //订阅主题-数据透传
            //data/down/0040/0004/DataPt/MQ0001003A123456(序列号)
        //    sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/dataPt/%s\",2\r\n",gs_DevicePara.cDeviceID);
            //strcat(ucSendBuf,&gs_DevicePara.cDeviceID[0]); //将设备ID添加到主题中
            //strcat(ucSendBuf,"\",2\r\n");
        //    usSendDataLen = strlen((char *)ucSendBuf);
        //    sprintf((char *)ucRecvCheckData, "OK");
        //    break;
        case Module_PUBLISH_TOPIC_UPGRADECHECK_CMD: //发布物联网云端口:发布主题为GetVersionUpdateFlag，QoS为2，消息体为Get
            usSendDataLen = func_Get_UpgradeCheck_CMD(ucSendBuf);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/UpgradeCheck/%s\",%d\r\n",gs_DevicePara.cDeviceID, usSendDataLen);
            //sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/iVOw212I78/ZCJ2025042801/thing/property/post\",%d\r\n",usSendDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            COM_SendData(ucSendBuf, usSendDataLen);
            DDL_DelayMS(400);
            m_u16RxLen = 0;
            memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            usSendDataLen = func_Get_UpgradeCheck_CMD(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        case Module_CLOSE_CONN_CMD: //关闭物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTCLOSE=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_DISCONNECT_MQTT_INTERFACE_CMD: //关闭物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTDISC=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_TUNS_TOPIC_CMD: //关闭物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTUNS=0,1,\"topic\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        default:
            break;
        }
        m_RecvFlag = 0;
        //发送AT指令
        COM_SendData(ucSendBuf, usSendDataLen);
        //等待接收到OK
        #if 0
        i32Ret = COM_RecvData(ucRecvBuf, sizeof(ucRecvBuf), 2000);
        if ((i32Ret == LL_OK) )    //接收到数据或者接收超时
        {
            if (strstr((char *)ucRecvBuf, (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
            {
                #if 0
                if(guc_4G_Module_Init_CMD == 11)    //  延时等待服务器返回是否需要升级的信息
                {
                    memset(ucRecvCheckData, 0, 50);
                    sprintf((char *)ucRecvCheckData, "QMTRECV:0,1,VersionUpdateFlag");
                    i32Ret1 = COM_RecvData(ucRecvBuf, 60, 5000);
                    if ((i32Ret1 == LL_OK) || (i32Ret1 == LL_ERR))    //接收到数据或者接收超时
                    {
                        if (strstr((char *)ucRecvBuf, (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
                        {
                            if (strstr((char *)ucRecvBuf, "YES") != NULL)   //含有YES,表示要进行升级
                            {
                                return 2;
                            }
                            else    //含有NO,表示不需要升级
                            {
                                return 1;
                            }
                        }
                        else
                        {
                            u8Temp = 0;
                            return 0;
                        }
                    }
                    else
                    {
                        u8Temp = 0;
                        return 0;
                    }
                }
                else
                {
                    guc_4G_Module_Init_CMD++;
                }
                #else
                if(guc_4G_Module_Init_CMD == 7)
                {
                    return 1;
                }
                else
                {
                    guc_4G_Module_Init_CMD++;
                }
                #endif
            }
            else
            {
                //考虑上述case 4的情况，如果查询到的不是1，那么需要继续查询是否为5，1表示注册本地网;5表示注册漫游网
                if (guc_4G_Module_Init_CMD == 4)
                {
                    if (strstr((char *)ucRecvBuf, "+CGREG: 0,5") == NULL)
                    {
                        u8Temp = 0;
                        return 0;
                    }
                    else
                    {
                        guc_4G_Module_Init_CMD++;
                    }
                }
                else
                {
                    //guc_4G_Module_Init_CMD++;
                }
                
            }
        }
        else
        {
            ucRetryCnt++;
            if(ucRetryCnt >= 3)
            {
                u8Temp = 0;
                return 2;
            }
            
        }
        #else
        usRecvTimeOutCnt = 0;
        while(m_RecvFlag == 0)
        {
            DDL_DelayMS(20);
            usRecvTimeOutCnt++;
            if(usRecvTimeOutCnt >= 100)
            {
                if(strlen((char*)m_au8RxBuf) > 0)
                {
                    //如果接收到数据，但没有OK，则继续等待
                    if(func_Array_Find_Str((char *)m_au8RxBuf,strlen((char*)m_au8RxBuf),(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usDataPosi) != 0)
                    {
                        //接收到数据，但没有OK
                        //u8Temp = 1;
                        break;
                    }
                }
            }
            if(usRecvTimeOutCnt >= 500)
            {
                return 2;
            }
        }
        if(m_u16RxLen > 0)
        {
            usRecvTimeOutCnt = 0;
            while(m_au8RxBuf[m_u16RxLen-1] == 0x00)
            {
                DDL_DelayMS(20);
                usRecvTimeOutCnt++;
                if(usRecvTimeOutCnt >= 500)
                {
                    return 2;
                }
            }
        }
        m_RecvFlag = 0;
        //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
        if(func_Array_Find_Str((char *)m_au8RxBuf,m_u16RxLen,(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usDataPosi) == 0) //接收到的数据中包含OK
        {
            #if 1
            if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_UPGRADECHECK_CMD)
            {
                if(func_Array_Find_Str((char *)m_au8RxBuf,m_u16RxLen,"\"res\":0",7, &usDataPosi) == 0)
                {
                    for(i=usDataPosi+14; i < m_u16RxLen; i++)
                    {
                        if(ucFlag == 0)
                        {
                            if(m_au8RxBuf[i] == ':')
                            {
                                ucFlag = 1;
                            }
                        }
                        else
                        {
                            if(m_au8RxBuf[i] == ',')
                            {
                                usPosi1 = i;
                                gul_UpdateFileSize = atoi((char *)ucDataLenArr); //获取升级文件大小
                                break;
                            }
                            else
                            {
                                ucDataLenArr[j++] = m_au8RxBuf[i];
                            }
                        }
                        
                    }
                    //获取新版本号
                    ucFlag = 0;
                    j = 0;
                    //usPosi1 = 0;
                    for(i=usPosi1+11; i < m_u16RxLen; i++)
                    {
                        if(m_au8RxBuf[i] == ',')
                        {
                            ucFlag = 1;
                            usPosi1 = i;
                            break;
                        }
                        if(ucFlag == 0)
                        {
                            guc_NewVersion[j++] = m_au8RxBuf[i];
                        }
                    }
                    //获取升级url
                    ucFlag = 0;
                    j = 0;
                    for(i=usPosi1+8; i < m_u16RxLen; i++)
                    {
                        if(m_au8RxBuf[i] == '\"')
                        {
                            ucFlag = 1;
                            break;
                        }
                        if(ucFlag == 0)
                        {
                            guc_URLArr[j++] = m_au8RxBuf[i];
                        }
                    }
                    //memcpy(guc_URLArr, m_au8RxBuf+usDataPosi+14, 200); //获取升级文件的URL地址
                    gus_URLArrLen = strlen((char *)guc_URLArr);
                    return 0; //需要升级
                }
                else if(func_Array_Find_Str((char *)m_au8RxBuf,m_u16RxLen,"\"res\":1",7, &usDataPosi) == 0)
                {
                    return 1; //不需要升级
                }
                else
                {
                    return 2; //获取升级状态失败
                }
            }
            else if(gE_4G_Module_Init_CMD == Module_QUERY_LOCAL_DATE_TIME_CMD)
            {
                cTemp = strchr((char*)m_au8RxBuf,'"');
                memcpy(guc_StartDateTime,&cTemp[1],19);
                guc_StartDateTime[10] = ' ';
                gE_4G_Module_Init_CMD++;
                ucRetryCnt = 0;
            }
            #else
            if(gE_4G_Module_Init_CMD == Module_SET_MQTT_VERSION_CMD)
            {
                return 2;
            }
            #endif
            else
            {
                gE_4G_Module_Init_CMD++;
                ucRetryCnt = 0;
            }
        }
        else
        {
            DDL_DelayMS(200);
            if(ucRetryCnt >= 5) //重复发送三次
            //if(ucRetryCnt >= 3) //重复发送三次
            {
                //u8Temp = 0;
                return 3;
            }
            ucRetryCnt++;
        }
        #endif
    }
    return 1;
}

/**
 * @brief  4G EC200U Module Init.
 * @param  None
 * @retval 0: module init failed 1: no update 2: need update
 */
uint8_t EC200U_4G_Module_Init(void)
{
    uint8_t ucResult = 0xFF;
    uint8_t ucResult1 = 0xFF;
    uint16_t usRecvTimeOutCnt = 0;
    uint8_t i = 0;
    uint8_t l = 0;
    EC200U_4G_Module_GPIO_Init();

    for (i = 0; i < 3; i++)
    {
        //拉低4G模块电源引脚2s以上，让4G模块开机
        GPIO_ResetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
        //等待4G模块开机
        SysTick_Delay(2000);
        GPIO_SetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
        //等待4G模块串口开始工作
        SysTick_Delay(2000);

        //等待模块启动成功主动传回"RDY\r\n"
        usRecvTimeOutCnt = 0;
        while(m_RecvFlag == 0)
        {
            SysTick_Delay(10);
            usRecvTimeOutCnt++;
            if(usRecvTimeOutCnt >= 1500)
            {
                ucResult = 1;
                break;
            }
        }
        m_RecvFlag = 0;

        if(strstr((char *)m_au8RxBuf, "RDY") != NULL)
        {
            //模块启动成功
            ucResult = 0;
            //break;
        }
        else
        {
            //模块启动失败
            ucResult = 1;
        }
        if(ucResult == 0)
        {
            for(l=0; l<3; l++)
            {
                m_u16RxLen = 0;
                gE_4G_Module_Init_CMD = Module_TEST_AT_CMD;
                memset(m_au8RxBuf,0,APP_FRAME_LEN_MAX);
                
                ucResult1 = EC200U_4G_Module_Configuration_Init();
                if(ucResult1 == 0)
                {
                    i = 3;  //跳出外层循环
                    break;
                }
            }
            
        }
    }
    
    return ucResult1;
}

//当设备需要进行升级时，进行HTTP连接并获取升级文件
//ucURLArr: 需要连接的URL地址; usURLLen: URL地址长度
uint32_t ulDataStartPosi = 0;
unsigned char func_4G_Module_Connect_HTTP(unsigned char* ucURLArr, uint16_t usURLLen, uint32_t ulDataTotalSize)
{
    uint8_t ucRetryCnt = 0;
    uint16_t usRecvTimeOutCnt = 0;
    uint16_t usSendDataLen = 0;
    uint8_t ucRecvCheckData[50] = {0};
    
    uint32_t ulDataLen = 1024; //数据长度
    uint16_t usRXCheckLen = 0;
    uint16_t usDataPosi = 0;
    uint8_t ucResult = 0;
    uint8_t ucFilehandle = 0;
    unsigned long i = 0;
    unsigned short j = 0;
    unsigned char ucFlag = 0;
    uint8_t ucDataLenArr[10] = {0}; //用于存储数据长度
    __IO uint32_t appFlashAddr;

    appFlashAddr = IAP_APP_ADDR;

    //拉低4G模块电源引脚2s以上，让4G模块开机
    GPIO_ResetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
    //等待4G模块开机
    SysTick_Delay(2000);
    memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);
    GPIO_SetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
    //等待4G模块串口开始工作
    SysTick_Delay(2000);

    //等待模块启动成功主动传回"RDY\r\n"
    usRecvTimeOutCnt = 0;
    while(m_RecvFlag == 0)
    {
        SysTick_Delay(10);
        usRecvTimeOutCnt++;
        if(usRecvTimeOutCnt >= 1500)
        {
            ucResult = 1;
            break;
        }
    }
    m_RecvFlag = 0;

    if(strstr((char *)m_au8RxBuf, "RDY") != NULL)
    {
        //模块启动成功
        ucResult = 0;

    }
    else
    {
        //模块启动失败
        ucResult = 1;
    }

    if(ucResult != 0)
    {
        return 1; //模块启动失败
    }
    while(1)
    {
        memset(ucSendBuf, 0, EC200U_BUF_SIZE);
        memset(ucRecvCheckData, 0, 50);
        m_u16RxLen = 0;
        memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);

        switch (gE_4G_Module_Connect_HTTP_CMD)
        {
        case Module_HTTP_ATE0_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "ATE0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_HTTP_CFG: //配置PDP上下文
            (void)strcpy((char *)ucSendBuf, "AT+QHTTPCFG=\"contextid\",1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_HTTP_CFG1: //配置PDP上下文
            (void)strcpy((char *)ucSendBuf, "AT+QHTTPCFG=\"requestheader\",0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_HTTP_ICSGP: //配置APN
            (void)strcpy((char *)ucSendBuf, "AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        //case Module_HTTP_CGDCONT:
        //    (void)strcpy((char *)ucSendBuf, "AT+CGDCONT?\r\n");
        //    usSendDataLen = strlen((char *)ucSendBuf);
        //    sprintf((char *)ucRecvCheckData, "OK");
        //    break;
        case Module_HTTP_IACT: //激活PDP上下文
            (void)strcpy((char *)ucSendBuf, "AT+QIACT=1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_HTTP_URL_DATALEN: //设置要访问的URL
            //(void)strcpy((char *)ucSendBuf, "AT+QIURC=1\r\n");
            sprintf((char *)ucSendBuf, "AT+QHTTPURL=%d,80\r\n", usURLLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "CONNECT");
            break;
        case Module_HTTP_URL: //发送URL
            sprintf((char *)ucSendBuf, "%s\r\n", ucURLArr);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_HTTP_GETEX: //发送GET请求
            //(void)strcpy((char *)ucSendBuf, "AT+QIURC=2\r\n");
            //sprintf((char *)ucSendBuf, "AT+QHTTPGETEX=80,%d,%d\r\n",ulDataStartPosi, ulDataLen);
            sprintf((char *)ucSendBuf, "AT+QHTTPGET=80\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_HTTP_READ: //读取HTTP响应
            (void)strcpy((char *)ucSendBuf, "AT+QHTTPREADFILE=\"UFS:app.bin\",80\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+QHTTPREADFILE");
            break;
        case Module_HTTP_CLOSE: //关闭HTTP连接
            (void)strcpy((char *)ucSendBuf, "AT+QHTTPSTOP\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #if 1
        case Module_FILE_QFLDS: //查询文件大小
            (void)strcpy((char *)ucSendBuf, "AT+QFLDS=UFS\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+QFLDS:");
            break;
        case Module_FILE_QFLST: //查询文件列表
            (void)strcpy((char *)ucSendBuf, "AT+QFLST=\"*\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+QFLST:");
            break;
        #endif
        case Module_FILE_QFOPEN: //打开文件
            (void)strcpy((char *)ucSendBuf, "AT+QFOPEN=\"app.bin\",2\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_FILE_BASICPOSI: //定位初始地址,0x13C00
            sprintf((char *)ucSendBuf, "AT+QFSEEK=%d,%d,0\r\n", ucFilehandle,0x13C00);
            ulDataTotalSize = ulDataTotalSize - 0x13C00; //减去初始地址
            gul_IAP_Upgrade_Total_Size = ulDataTotalSize; //保存升级文件总大小
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            if (ulDataTotalSize < IAP_APP_SIZE) 
            {
                /* Erase user application area */
                (void)FLASH_EraseSector(IAP_APP_ADDR, ulDataTotalSize);
                (void)FLASH_EraseSector(APP_EXIST_FLAG_ADDR, 0U);
            }
            break;
        case Module_FILE_QFREAD: //读取文件
            sprintf((char *)ucSendBuf, "AT+QFREAD=%d,%d\r\n", ucFilehandle, ulDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        //case Module_FILE_QFSEEK: //设置文件指针位置
        //    sprintf((char *)ucSendBuf, "AT+QFSEEK=%d,%d,1\r\n", ucFilehandle,ulDataLen);
        //    usSendDataLen = strlen((char *)ucSendBuf);
        //    sprintf((char *)ucRecvCheckData, "OK");
        //    break;
        case Module_FILE_QFCLOSE: //关闭文件
            (void)sprintf((char *)ucSendBuf, "AT+QFCLOSE=%d\r\n",ucFilehandle);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_FILE_DELETE:    //删除文件
            (void)sprintf((char *)ucSendBuf, "AT+QFDEL=\"app.bin\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        default:
            return 1; //错误状态
        }
        m_RecvFlag = 0;
        //发送AT指令
        COM_SendData(ucSendBuf, usSendDataLen);
        //等待接收到OK
        
        usRecvTimeOutCnt = 0;
        while(m_RecvFlag == 0)
        {
            DDL_DelayMS(20);
            usRecvTimeOutCnt++;
            if(usRecvTimeOutCnt >= 100)
            {
                if(strlen((char*)m_au8RxBuf) > 0)
                {
                    //如果接收到数据，但没有OK，则继续等待
                    if(func_Array_Find_Str((char *)m_au8RxBuf,strlen((char*)m_au8RxBuf),(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usDataPosi) != 0)
                    {
                        //接收到数据，但没有OK
                        //u8Temp = 1;
                        break;
                    }
                }
            }
            if(usRecvTimeOutCnt >= 500)
            {
                return 2;
            }
        }
        if(m_u16RxLen > 0)
        {
            usRecvTimeOutCnt = 0;
            while(m_au8RxBuf[m_u16RxLen-1] == 0x00)
            {
                DDL_DelayMS(20);
                usRecvTimeOutCnt++;
                if(usRecvTimeOutCnt >= 500)
                {
                    return 2;
                }
            }
        }
        m_RecvFlag = 0;

        if(gE_4G_Module_Connect_HTTP_CMD == Module_HTTP_READ)
        {
            DDL_DelayMS(500); //等待数据接收完成
        }
        
        if(gE_4G_Module_Connect_HTTP_CMD == Module_FILE_QFREAD)
        {
            usRXCheckLen = APP_FRAME_LEN_MAX;
            DDL_DelayMS(100); //等待数据接收完成
        }
        else
        {
            usRXCheckLen = m_u16RxLen;
        }
        //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
        if(func_Array_Find_Str((char *)m_au8RxBuf,usRXCheckLen,(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usDataPosi) == 0) //接收到的数据中包含OK
        {
            if(gE_4G_Module_Connect_HTTP_CMD == Module_FILE_QFOPEN)
            {
                if(func_Array_Find_Str((char *)m_au8RxBuf,m_u16RxLen,"QFOPEN",6, &usDataPosi) == 0)
                {
                    j = 0;
                    for(i=usDataPosi; i < m_u16RxLen; i++)
                    {
                        if(ucFlag == 0)
                        {
                            if(m_au8RxBuf[i] == ':')
                            {
                                ucFlag = 1;
                            }
                        }
                        else
                        {
                            if(m_au8RxBuf[i] == '\r')
                            {
                                ucFilehandle = atoi((char *)ucDataLenArr); //获取升级文件大小
                                break;
                            }
                            else
                            {
                                if(m_au8RxBuf[i] >= '0' && m_au8RxBuf[i] <= '9') //只获取数字
                                {
                                    ucDataLenArr[j++] = m_au8RxBuf[i];
                                }
                            }
                        }
                        
                    }
                    
                }
                ucRetryCnt = 0;
                gE_4G_Module_Connect_HTTP_CMD++;
            }
            else if(gE_4G_Module_Connect_HTTP_CMD == Module_FILE_DELETE)
            {
                return 0; //HTTP连接关闭成功
            }
            else if(gE_4G_Module_Connect_HTTP_CMD == Module_FILE_QFREAD)
            {
                if (FLASH_WriteData(appFlashAddr, &m_au8RxBuf[0], ulDataLen) == LL_OK) 
                {
                    appFlashAddr += ulDataLen;
                } 
                ulDataStartPosi += ulDataLen;
                gul_IAP_Upgrade_Current_Size = ulDataStartPosi; //保存当前升级文件已下载大小
                if(ulDataStartPosi >= ulDataTotalSize) //数据读取完成
                {
                    i = APP_EXIST_FLAG;
                    (void)FLASH_WriteData(APP_EXIST_FLAG_ADDR, (uint8_t *)&i, 4U);
                    gE_4G_Module_Connect_HTTP_CMD++; //进入关闭HTTP连接状态
                }
                else
                {
                    ulDataLen = (ulDataTotalSize - ulDataStartPosi > 1024) ? 1024 : (ulDataTotalSize - ulDataStartPosi);
                    gE_4G_Module_Connect_HTTP_CMD = Module_FILE_QFREAD; //继续获取数据
                }
                func_Device_Upgrade_View_Show();
                ucRetryCnt = 0;
            }
            else
            {
                ucRetryCnt = 0;
                gE_4G_Module_Connect_HTTP_CMD++;
            }
            
        }
        else
        {
            DDL_DelayMS(200);
            ucRetryCnt++;
            if(ucRetryCnt >= 5) //重复发送三次
            {
                //u8Temp = 0;
                return 3;
            }
        }
    }
    //return 1;
}

unsigned char func_4G_Up_Upgrade_Result(unsigned char ucResult)
{
    uint8_t u8Temp = 1;
    uint8_t ucRetryCnt = 0;
    uint16_t usRecvTimeOutCnt = 0;
    uint16_t usSendDataLen = 0;
    uint8_t ucRecvCheckData[50] = {0};
    uint16_t usDataPosi = 0;
    //unsigned short usPosi1 = 0;
    //uint8_t ucDataLenArr[10] = {0};
    //uint16_t i = 0;
    //uint8_t ucFlag = 0;
    //uint8_t j = 0;
    char *cTemp;

    //拉低4G模块电源引脚2s以上，让4G模块开机
    GPIO_ResetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
    //等待4G模块开机
    SysTick_Delay(2000);
    memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);
    GPIO_SetPins(EC200U_4G_MODULE_PWRKEY_PORT, EC200U_4G_MODULE_PWRKEY_PIN);
    //等待4G模块串口开始工作
    SysTick_Delay(2000);

    //等待模块启动成功主动传回"RDY\r\n"
    usRecvTimeOutCnt = 0;
    while(m_RecvFlag == 0)
    {
        SysTick_Delay(10);
        usRecvTimeOutCnt++;
        if(usRecvTimeOutCnt >= 1500)
        {
            ucResult = 1;
            break;
        }
    }
    m_RecvFlag = 0;

    if(strstr((char *)m_au8RxBuf, "RDY") != NULL)
    {
        //模块启动成功
        ucResult = 0;

    }
    else
    {
        //模块启动失败
        ucResult = 1;
    }
    m_u16RxLen = 0;
    gE_4G_Module_Init_CMD = Module_TEST_AT_CMD;
    memset(m_au8RxBuf,0,APP_FRAME_LEN_MAX);

    while(u8Temp == 1)
    {
        memset(ucSendBuf, 0, EC200U_BUF_SIZE);
        //memset(ucRecvBuf, 0, 1030);
        memset(ucRecvCheckData, 0, 50);
        m_u16RxLen = 0;
        memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);

        switch (gE_4G_Module_Init_CMD)
        {
        case Module_TEST_AT_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "AT\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_TEST_ATE0_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "ATE0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_SIM_CARD_STATE_CMD: //查询SIM卡状态
            (void)strcpy((char *)ucSendBuf, "AT+CPIN?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CPIN: READY");
            break;
        case Module_QUERY_SIGNAL_STRENGTH_CMD: //查询信号强度
            (void)strcpy((char *)ucSendBuf, "AT+CSQ\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CSQ:");
            break;
        case Module_QUERY_PS_DOMAIN_REG_STATE_CMD://查询PS域注册状态：0：未注册，1/5：注册，2：正在搜索
            //(void)strcpy((char *)ucSendBuf, "AT+CREG?\r\n");    //CS域，只使用在2G网络上
            (void)strcpy((char *)ucSendBuf, "AT+CGREG?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGREG: 0,1");
            break;
        case Module_ACTIVATE_NETWORK_CMD: //激活网络
            (void)strcpy((char *)ucSendBuf, "AT+CGATT=1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_NETWORK_ACTIVATE_STATE_CMD: //查询网络激活状态
            (void)strcpy((char *)ucSendBuf, "AT+CGATT?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGATT: 1");
            break;
        case Module_QUERY_LOCAL_DATE_TIME_CMD: //查询本地日期时间
            (void)strcpy((char *)ucSendBuf, "AT+QLTS=2\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            func_WatchDog_Refresh();
            break;
        //MQTT配置
        case Module_SET_DATA_FORMAT_CMD: //设置数据格式
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"recv/mode\",0,0,1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SET_MQTT_KEEPALIVE_TIME_CMD:    //心跳时间建议60s~300s.这里设置120s
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"keepalive\",0,120\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SET_MQTT_VERSION_CMD:   //设置MQTT 版本
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"version\",0,4\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_OPEN_MQTT_INTERFACE_CMD: //打开物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTOPEN=0,\"218.85.5.161\",7243\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "QMTOPEN");
            break;
        case Module_CONN_MQTT_INTERFACE_CMD: //连接物联网云端口
            (void)sprintf((char *)ucSendBuf, "AT+QMTCONN=0,\"%s\",\"xfgd\",\"xfgd@1234\"\r\n",gs_DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_UPGRADERESULT_CMD:    //订阅主题-升级结果
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/UpgradeResult/%s\",2\r\n",gs_DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_PUBLISH_TOPIC_UPGRADERESULT_CMD: //发布物联网云端口:发布主题为GetVersionUpdateFlag，QoS为2，消息体为Get
            usSendDataLen = func_Get_UpgradeResult_CMD(ucSendBuf,ucResult);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/UpgradeResult/%s\",%d\r\n",gs_DevicePara.cDeviceID, usSendDataLen);
            //sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/iVOw212I78/ZCJ2025042801/thing/property/post\",%d\r\n",usSendDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            COM_SendData(ucSendBuf, usSendDataLen);
            DDL_DelayMS(400);
            m_u16RxLen = 0;
            memset(m_au8RxBuf, 0, APP_FRAME_LEN_MAX);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            usSendDataLen = func_Get_UpgradeResult_CMD(ucSendBuf,ucResult);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        case Module_CLOSE_CONN_CMD: //关闭物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTCLOSE=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_DISCONNECT_MQTT_INTERFACE_CMD: //关闭物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTDISC=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_TUNS_TOPIC_CMD: //关闭物联网云端口
            (void)strcpy((char *)ucSendBuf, "AT+QMTUNS=0,1,\"topic\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        default:
            break;
        }
        m_RecvFlag = 0;
        //发送AT指令
        COM_SendData(ucSendBuf, usSendDataLen);
        //等待接收到OK
        usRecvTimeOutCnt = 0;
        while(m_RecvFlag == 0)
        {
            DDL_DelayMS(20);
            usRecvTimeOutCnt++;
            if(usRecvTimeOutCnt >= 100)
            {
                if(strlen((char*)m_au8RxBuf) > 0)
                {
                    //如果接收到数据，但没有OK，则继续等待
                    if(func_Array_Find_Str((char *)m_au8RxBuf,strlen((char*)m_au8RxBuf),(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usDataPosi) != 0)
                    {
                        //接收到数据，但没有OK
                        //u8Temp = 1;
                        break;
                    }
                }
            }
            if(usRecvTimeOutCnt >= 500)
            {
                return 2;
            }
        }
        if(m_u16RxLen > 0)
        {
            usRecvTimeOutCnt = 0;
            while(m_au8RxBuf[m_u16RxLen-1] == 0x00)
            {
                DDL_DelayMS(20);
                usRecvTimeOutCnt++;
                if(usRecvTimeOutCnt >= 500)
                {
                    return 2;
                }
            }
        }
        m_RecvFlag = 0;
        //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
        if(func_Array_Find_Str((char *)m_au8RxBuf,m_u16RxLen,(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usDataPosi) == 0) //接收到的数据中包含OK
        {
            #if 1
            if(gE_4G_Module_Init_CMD == Module_CONN_MQTT_INTERFACE_CMD)
            {
                gE_4G_Module_Init_CMD = Module_SUBSCRIBE_TOPIC_UPGRADERESULT_CMD; //连接成功后，订阅主题
            }
            else if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_UPGRADERESULT_CMD)
            {
                return 0;
            }
            else if(gE_4G_Module_Init_CMD == Module_QUERY_LOCAL_DATE_TIME_CMD)
            {
                cTemp = strchr((char*)m_au8RxBuf,'"');
                memcpy(guc_StartDateTime,&cTemp[1],19);
                guc_StartDateTime[10] = ' ';
                gE_4G_Module_Init_CMD++;
                ucRetryCnt = 0;
            }
            else
            {
                gE_4G_Module_Init_CMD++;
                ucRetryCnt = 0;
            }
        }
        else
        {
            DDL_DelayMS(200);
            ucRetryCnt++;
            if(ucRetryCnt >= 5) //重复发送三次
            {
                //u8Temp = 0;
                return 3;
            }
            
        }
        #endif
    }
    return 1;
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
