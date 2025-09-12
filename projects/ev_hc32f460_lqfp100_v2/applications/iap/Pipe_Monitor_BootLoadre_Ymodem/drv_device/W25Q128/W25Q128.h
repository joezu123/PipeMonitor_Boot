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
#ifndef __W25Q128_H__
#define __W25Q128_H__

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
#include "hc32_ll_spi.h"
#include "hc32_ll_utility.h"
#include "com.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#ifdef HW_VERSION_V1_1
#define BSP_SPI_CS_PORT                 (GPIO_PORT_A)
#define BSP_SPI_CS_PIN                  (GPIO_PIN_00)
#else
#define BSP_SPI_CS_PORT                 (GPIO_PORT_A)
#define BSP_SPI_CS_PIN                  (GPIO_PIN_01)
#endif
#define BSP_SPI_CS_ACTIVE()             GPIO_ResetPins(BSP_SPI_CS_PORT, BSP_SPI_CS_PIN)
#define BSP_SPI_CS_INACTIVE()           GPIO_SetPins(BSP_SPI_CS_PORT, BSP_SPI_CS_PIN)

#ifdef HW_VERSION_V1_1
#define BSP_SPI_SCK_PORT                (GPIO_PORT_A)
#define BSP_SPI_SCK_PIN                 (GPIO_PIN_02)
#else
#define BSP_SPI_SCK_PORT                (GPIO_PORT_A)
#define BSP_SPI_SCK_PIN                 (GPIO_PIN_03)
#endif
#define BSP_SPI_SCK_PIN_FUNC            (GPIO_FUNC_43)          /*!< SPI1 SCK */

#ifdef HW_VERSION_V1_1
#define BSP_SPI_MOSI_PORT               (GPIO_PORT_A)           /*!< W25Qxx IO0 */
#define BSP_SPI_MOSI_PIN                (GPIO_PIN_03)
#else
#define BSP_SPI_MOSI_PORT               (GPIO_PORT_A)           /*!< W25Qxx IO0 */
#define BSP_SPI_MOSI_PIN                (GPIO_PIN_02)
#endif
#define BSP_SPI_MOSI_PIN_FUNC           (GPIO_FUNC_40)          /*!< SPI1 MOSI */

#ifdef HW_VERSION_V1_1
#define BSP_SPI_MISO_PORT               (GPIO_PORT_A)           /*!< W25Qxx IO1 */
#define BSP_SPI_MISO_PIN                (GPIO_PIN_01)
#else
#define BSP_SPI_MISO_PORT               (GPIO_PORT_A)           /*!< W25Qxx IO1 */
#define BSP_SPI_MISO_PIN                (GPIO_PIN_00)
#endif
#define BSP_SPI_MISO_PIN_FUNC           (GPIO_FUNC_41)          /*!< SPI3 MISO */
/**
 * @}
 */

/**
 * @defgroup W25Qxx_SPI_Instance W25Qxx SPI Instance
 * @{
 */
#define BSP_SPI_UNIT                    CM_SPI1
#define BSP_SPI_PERIPH_CLK              FCG1_PERIPH_SPI1
/**
 * @}
 */

/**
 * @defgroup W25Qxx_SPI_Timeout W25Qxx SPI Timeout
 * @{
 */
#define BSP_SPI_TIMEOUT                 (100000)
/**
 * @}
 */

/**
 * @defgroup W25Qxx_Size W25Qxx Size
 * @{
 */
#define W25Q128_PAGE_SIZE                (256UL)
#define W25Q128_SECTOR_SIZE              (1024UL * 4UL)
#define W25Q128_BLOCK_SIZE               (1024UL * 64UL)
#define W25Q128_PAGE_PER_SECTOR          (W25Q128_SECTOR_SIZE / W25Q128_PAGE_SIZE)
#define W25Q128_MAX_ADDR                 (0x1000000UL)
/**
 * @}
 */

/**
 * @}
 */
typedef struct {
  void (*Delay)(uint32_t);
  void (*Init)(void);
  void (*DeInit)(void);
  void (*Active)(void);
  void (*Inactive)(void);
  int32_t (*Trans)(const uint8_t *, uint32_t);
  int32_t (*Receive)(uint8_t *, uint32_t);
} stc_w25qxx_ll_t;

/**
* @}
*/

/*******************************************************************************
* Global pre-processor symbols/macros ('#define')
******************************************************************************/
/**
* @defgroup W25QXX_Global_Macros W25QXX Global Macros
* @{
*/

/**
* @defgroup W25QXX_ID W25QXX ID
* @{
*/
#define W25Q80                  (0xEF13U)
#define W25Q16                  (0xEF14U)
#define W25Q32                  (0xEF15U)
#define W25Q64                  (0xEF16U)
#define W25Q128                 (0xEF17U)
/**
* @}
*/

/**
* @defgroup W25QXX_Command W25QXX Command
* @{
*/
#define W25QXX_WRITE_ENABLE                     (0x06U)
#define W25QXX_VOLATILE_SR_WRITE_ENABLE         (0x50U)
#define W25QXX_WRITE_DISABLE                    (0x04U)
#define W25QXX_RELEASE_POWER_DOWN_ID            (0xABU)
#define W25QXX_MANUFACTURER_DEVICE_ID           (0x90U)
#define W25QXX_JEDEC_ID                         (0x9FU)
#define W25QXX_READ_UNIQUE_ID                   (0x4BU)
#define W25QXX_READ_DATA                        (0x03U)
#define W25QXX_FAST_READ                        (0x0BU)
#define W25QXX_PAGE_PROGRAM                     (0x02U)
#define W25QXX_SECTOR_ERASE                     (0x20U)
#define W25QXX_BLOCK_ERASE_32KB                 (0x52U)
#define W25QXX_BLOCK_ERASE_64KB                 (0xD8U)
#define W25QXX_CHIP_ERASE                       (0xC7U)
#define W25QXX_READ_STATUS_REGISTER_1           (0x05U)
#define W25QXX_WRITE_STATUS_REGISTER_1          (0x01U)
#define W25QXX_READ_STATUS_REGISTER_2           (0x35U)
#define W25QXX_WRITE_STATUS_REGISTER_2          (0x31U)
#define W25QXX_READ_STATUS_REGISTER_3           (0x15U)
#define W25QXX_WRITE_STATUS_REGISTER_3          (0x11U)
#define W25QXX_READ_SFDP_REGISTER               (0x5AU)
#define W25QXX_ERASE_SECURITY_REGISTER          (0x44U)
#define W25QXX_PROGRAM_SECURITY_REGISTER        (0x42U)
#define W25QXX_READ_SECURITY_REGISTER           (0x48U)
#define W25QXX_GLOBAL_BLOCK_LOCK                (0x7EU)
#define W25QXX_GLOBAL_BLOCK_UNLOCK              (0x98U)
#define W25QXX_READ_BLOCK_LOCK                  (0x3DU)
#define W25QXX_INDIVIDUAL_BLOCK_LOCK            (0x36U)
#define W25QXX_INDIVIDUAL_BLOCK_UNLOCK          (0x39U)
#define W25QXX_ERASE_PROGRAM_SUSPEND            (0x75U)
#define W25QXX_ERASE_PROGRAM_RESUME             (0x7AU)
#define W25QXX_POWER_DOWN                       (0xB9U)
#define W25QXX_ENABLE_RESET                     (0x66U)
#define W25QXX_RESET_DEVICE                     (0x99U)
/**
* @}
*/

/**
* @defgroup W25QXX_Timeout_Value W25QXX Timeout Value
* @{
*/
#define W25QXX_TIMEOUT                          (100000UL)

#define SYSTEM_PARA_ADDR  0x0000  //系统配置参数保存地址，写以一扇区为单位4096Bytes
/**
* @}
*/

/**
* @}
*/
/*设备设置参数-服务器上发参数*/
typedef struct _ServerPara
{
	int nChannelID;	//通道序号
	char cIPAddr[20];	//IP字符串，支持域名
	int nPort;			//端口
	char cProcArr[10];	//UPD/TCP/COAP/MQTT
	int nAppProc;		//1:本协议
	int nDeviceUploadRecordCnt;	//设备设置的上传记录时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDelayTime;		//延时时间，单位min
}ServerParaSt;	//50Bytes

/*设备设置参数-以太网参数 */
typedef struct _EthPara
{
	char cAutoIP;	//0->关闭自动获取IP; 1->自动获取IP
	char cLocAddrArr[20];	//本地地址
	char cMaskArr[20];		//子网掩码
	char cGatewayArr[20];	//网关地址
	char cDNSArr[20];		//DNS服务器
}EthParaSt;	//81Bytes

/*设备外接传感器类型 */
typedef enum _MeasSensorType
{
	Meas_Integrated_Conductivity = 1,	//一体式电导率
	Meas_WaterQuality_COD,				//水质COD传感器	
	Meas_WaterLevel_Radar,				//雷达液位计
	Meas_WaterLevel_Pressure,			//压力液位计
	Meas_Flowmeter,					//流量计
	Meas_Max
}EMeasSensorType;

/*设备参数数据结构体*/
#pragma pack(1)
typedef struct _SysDevicePara
{
	char cDeviceID[17];	//设备ID:MQ+4位厂商代码+4位客户端类型+6位自定义字符串
	char cDeviceIdenFlag;	//设备是否经过NFC认证标志位	0->未验证
	char cDeviceRegisterFlag;	//设备是否在服务器通讯上注册标志位	0->未注册
	char cDeviceIMSI[16];	//设备IMSI号；是一个用于在全球范围内唯一地识别移动用户的标识符。IMSI存储在手机的SIM卡中，用于在蜂窝网络中进行用户身份验证和位置管理。
	char cDeviceIMEI[16];	//设备IMEI号；通常所说的手机序列号、手机“串号”，用于在移动电话网络中识别每一部独立的手机等移动通信设备，相当于移动电话的身份证。序列号共有15~17位数字
	char cDeviceHWVersion[10];	//设备硬件版本号
	char cDeviceSWVersion[10];	//设备软件版本号
	char cDevicePDDate[10];		//设备生产日期: YY-MM-DD
	int nDeviceSampleGapCnt;	//设备设置的采样间隔时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDeviceSaveRecordCnt;	//设备设置的保存记录时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDeviceUploadCnt;		//设备设置的上传间隔时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDeviceRecordCnt;	//设备历史数据记录计数
	//ServerParaSt esServerPara;	//服务器上发参数
	char cMQTT_UserNameArr[20];	//登录用户名
	char cMQTT_UserPWDArr[20];	//登录密码
	char cHeartGap;				//心跳间隔，单位s
	char cDps;					//离散估长	单位s
	//char cAPNArr[20];			//接入点名称
	//char cVPDNUserNameArr[20];	//VPDN用户名
	//char cVPDNUserPWDArr[20];	//VPDN密码
	//EthParaSt esEthPara;		//以太网参数
	char cParaVersion;			//设备参数版本号
	char cMeasSensorEnableFlag[2];	//两路485使能开关
	char cMeasSensorCount[2];		//设备两路外接测量传感器数量
	EMeasSensorType eMeasSensor[2][10];	//设备两路485外接测量传感器设备型号,可配置
	float fTotal_Volume;	//设备工作累计流量，单位m3
	float fInit_Height;		//设备安装时井深，用于计算液位高度，单位m，该值减去雷达液位计的空高值即为液位高度
	unsigned char ucUploadStatusGap;	//设备状态上传间隔时间，单位h 
	char cServerIP[2][16];	//设备连接物联网平台IP地址，共2组
	unsigned short usServerPort[2];	//设备连接物联网平台端口，共2组
	char cDebugModel;	//设备调试模式; 0->正常模式； 1->调试模式
	char cSensorBaudRate;	//设备485波特率; 0->9600; 1->2400; 2->4800; 3->19200; 4->38400; 5->115200
	char cBackUpArr[50];	//备用数据数组，长度50字节
	short sEEP_Version;			//存储版本号
}SysDeviceParaSt;	//325Bytes
#pragma pack()

extern SysDeviceParaSt gs_DevicePara;	//设备参数数据结构体
/*******************************************************************************
* Global variable definitions ('extern')
******************************************************************************/

/*******************************************************************************
Global function prototypes (definition in C source)
******************************************************************************/
/**
* @addtogroup W25QXX_Global_Functions W25QXX Global Functions
* @{
*/

int32_t W25QXX_Init(const stc_w25qxx_ll_t *pstcW25qxxLL);
int32_t W25QXX_DeInit(const stc_w25qxx_ll_t *pstcW25qxxLL);
int32_t W25QXX_GetManDeviceId(const stc_w25qxx_ll_t *pstcW25qxxLL, uint16_t *pu16ID);
int32_t W25QXX_GetUniqueId(const stc_w25qxx_ll_t *pstcW25qxxLL, uint8_t *pu8UniqueId);
int32_t W25QXX_ReadStatus(const stc_w25qxx_ll_t *pstcW25qxxLL, uint8_t u8SrRdCmd, uint8_t *pu8Status);
int32_t W25QXX_WriteStatus(const stc_w25qxx_ll_t *pstcW25qxxLL, uint8_t u8SrWtCmd, uint8_t u8Value);
int32_t W25QXX_PowerDown(const stc_w25qxx_ll_t *pstcW25qxxLL);
int32_t W25QXX_ReleasePowerDown(const stc_w25qxx_ll_t *pstcW25qxxLL);
int32_t W25QXX_EraseChip(const stc_w25qxx_ll_t *pstcW25qxxLL);
int32_t W25QXX_EraseSector(const stc_w25qxx_ll_t *pstcW25qxxLL, uint32_t u32Addr);
int32_t W25QXX_ReadData(const stc_w25qxx_ll_t *pstcW25qxxLL, uint32_t u32Addr, uint8_t *pu8ReadBuf, uint32_t u32NumByteToRead);
int32_t W25QXX_PageProgram(const stc_w25qxx_ll_t *pstcW25qxxLL, uint32_t u32Addr, const uint8_t *pu8Data, uint32_t u32NumByteToProgram);
/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup EV_HC32F460_LQFP100_V2_W25QXX_Global_Functions
 * @{
 */

void BSP_W25QXX_Init(void);
void BSP_W25QXX_DeInit(void);
int32_t BSP_W25QXX_Write(uint32_t u32Addr, const uint8_t *pu8Data, uint32_t u32NumByteToWrite);
int32_t BSP_W25QXX_Read(uint32_t u32Addr, uint8_t *pu8Data, uint32_t u32NumByteToRead);
int32_t BSP_W25QXX_EraseSector(uint32_t u32Addr);
int32_t BSP_W25QXX_EraseChip(void);

#ifdef __cplusplus
}
#endif

#endif /* __COM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
