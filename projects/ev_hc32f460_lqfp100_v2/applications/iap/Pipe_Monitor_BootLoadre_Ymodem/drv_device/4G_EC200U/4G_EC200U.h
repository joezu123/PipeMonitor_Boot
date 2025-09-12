/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\drivers\device_drv\4G_EC200U\4G_EC200U.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-12       Joe             First version
 @endverbatim

 */
#ifndef __4G_EC200U_H__
#define __4G_EC200U_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_fcg.h"
#include "hc32_ll_gpio.h"
#include "hc32_ll_usart.h"
#include "hc32_ll_utility.h"
#include "com.h"
#include "W25Q128.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EC200U_4G_MODULE_PWRKEY_PIN            (GPIO_PIN_02)
#define EC200U_4G_MODULE_PWRKEY_PORT           (GPIO_PORT_E)

#define EC200U_4G_MODULE_RST_PIN               (GPIO_PIN_03)
#define EC200U_4G_MODULE_RST_PORT              (GPIO_PORT_E)


/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/
typedef enum _4G_Module_Init_State
{
	Module_TEST_AT_CMD = 0,	//测试AT指令
	Module_TEST_ATE0_CMD,	//关闭回显
	Module_QUERY_SIM_CARD_STATE_CMD,	//查询SIM卡状态
	Module_QUERY_SIGNAL_STRENGTH_CMD,	//查询信号强度
	Module_QUERY_PS_DOMAIN_REG_STATE_CMD,	//查询PS域注册状态
	Module_ACTIVATE_NETWORK_CMD,			//激活网络
	Module_QUERY_NETWORK_ACTIVATE_STATE_CMD,	//查询网络激活状态
	//Module_QUERY_IMSI_CMD,	//查询IMSI号
	//Module_QUERY_IMEI_CMD,	//查询IMEI号
	Module_QUERY_LOCAL_DATE_TIME_CMD,	//查询本地日期时间
	Module_SET_DATA_FORMAT_CMD,	//设置数据格式
	Module_SET_MQTT_KEEPALIVE_TIME_CMD,	//设置MQTT心跳时间
	Module_SET_MQTT_VERSION_CMD,	//设置MQTT协议版本; 4->3.1.1； 3->3.1
	//Module_Check_MQTT_OPEN_CMD1,	//检查MQTT是否打开
	Module_OPEN_MQTT_INTERFACE_CMD,	//打开物联网云端口
	//Module_Check_MQTT_OPEN_CMD2,	//检查MQTT是否打开
	//Module_Check_MQTT_CONN_CMD1,	//检查MQTT连接状态
	Module_CONN_MQTT_INTERFACE_CMD,	//连接物联网云端口
	//Module_Check_MQTT_CONN_CMD2,	//检查MQTT连接状态
	//Module_CONNECT_CMD,	//连接MQTT服务器
	Module_SUBSCRIBE_TOPIC_UPGRADECHECK_CMD,	//订阅主题-UpgradeCheck
	//Module_SUBSCRIBE_TOPIC_DATAPT_CMD,			//订阅主题-DataPt
	//Module_CHECKPUBEX_CMD,	//检查MQTT订阅状态
	Module_PUBLISH_TOPIC_UPGRADECHECK_CMD,	//发布主题-UpgradeCheck
	Module_SUBSCRIBE_TOPIC_UPGRADERESULT_CMD,	//订阅主题-UpgradeResult
	Module_PUBLISH_TOPIC_UPGRADERESULT_CMD,	//发布主题-UpgradeResult
	
	Module_DISCONNECT_MQTT_INTERFACE_CMD,	//断开MQTT服务器
	Module_CLOSE_CONN_CMD,	//关闭MQTT客户端网络
	Module_TUNS_TOPIC_CMD,	//退订主题
	Module_INIT_STATE_MAX
}en_4G_Module_Init_State;

typedef enum _4G_Mudule_Connect_HTTP_State
{
	Module_HTTP_ATE0_CMD = 0,	//关闭回显
	Module_HTTP_CFG,        //配置PDP上下文
	Module_HTTP_CFG1,		//配置PDP上下文1
	Module_HTTP_ICSGP,          //配置APN
	//Module_HTTP_CGDCONT,		//查询APN
	Module_HTTP_IACT,       // 激活PDP上下文
	Module_HTTP_URL_DATALEN,		//设置要访问的URL域名长度
	Module_HTTP_URL,			//设置要访问的URL域名
	Module_HTTP_GETEX,		//发送GET请求
	Module_HTTP_READ,		//读取HTTP响应
	Module_HTTP_CLOSE,		//关闭HTTP连接
	
	Module_FILE_QFLDS,		//获取存储器空间大小
	Module_FILE_QFLST,		//获取存储器文件列表
	Module_FILE_QFOPEN,		//打开文件
	Module_FILE_BASICPOSI,	//定位初始地址
	Module_FILE_QFREAD,		//读取文件
	//Module_FILE_QFSEEK,		//设置文件指针位置
	Module_FILE_QFCLOSE,	//关闭文件
	Module_FILE_DELETE,		//删除文件
	Module_Connect_HTTP_State_Max
}en_4G_Mudule_Connect_HTTP_State;
/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/

void EC200U_4G_Module_GPIO_Init(void);
uint8_t EC200U_4G_Module_Configuration_Init(void);
uint8_t EC200U_4G_Module_Init(void);
extern uint8_t func_Publish_Topic_DataPt_Cmd(void);
extern unsigned char func_4G_Module_Connect_HTTP(unsigned char* ucURLArr, uint16_t usURLLen, uint32_t ulDataTotalSize);
extern unsigned char func_4G_Up_Upgrade_Result(unsigned char ucResult);


extern uint8_t guc_URLArr[200];	//用于存储URL地址
extern uint16_t gus_URLArrLen; //URL网址链接长度
extern uint32_t gul_UpdateFileSize;	//升级文件大小
#ifdef __cplusplus
}
#endif

#endif /* __COM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
