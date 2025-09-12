/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\source\com.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-12       Joe             First version
 @endverbatim

 */
#ifndef __COM_H__
#define __COM_H__

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
#include "hc32f460.h"
#include "hc32_ll_dma.h"
#include "hc32_ll_aos.h"
#include "hc32_ll_tmr0.h"
#include "hc32_ll_interrupts.h"
/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define HW_VERSION_V1_1	1	//硬件V1.1版本

#define MODEM_USART_UNIT                (CM_USART1)
#define MODEM_USART_CLK_CTRL(state)     FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART1, state)
#define MODEM_USART_BAUD_RATE           (115200UL)

#ifdef HW_VERSION_V1_1
#define MODEM_USART_RX_PORT             (GPIO_PORT_A)
#define MODEM_USART_RX_PIN              (GPIO_PIN_07)
#define MODEM_USART_RX_FUNC             (GPIO_FUNC_32)  //32->USART1_TX; 33->USART1_RX 
#else
#define MODEM_USART_RX_PORT             (GPIO_PORT_C)
#define MODEM_USART_RX_PIN              (GPIO_PIN_00)
#define MODEM_USART_RX_FUNC             (GPIO_FUNC_33)
#endif


#ifdef HW_VERSION_V1_1
#define MODEM_USART_TX_PORT             (GPIO_PORT_C)
#define MODEM_USART_TX_PIN              (GPIO_PIN_04)
#define MODEM_USART_TX_FUNC             (GPIO_FUNC_33)
#else
#define MODEM_USART_TX_PORT             (GPIO_PORT_C)
#define MODEM_USART_TX_PIN              (GPIO_PIN_01)
#define MODEM_USART_TX_FUNC             (GPIO_FUNC_32)
#endif


/* Application frame length max definition */
#define APP_FRAME_LEN_MAX               (1050U)

extern uint16_t m_u16RxLen;
extern uint8_t m_RecvFlag;
extern uint8_t m_au8RxBuf[APP_FRAME_LEN_MAX];

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
void COM_DeInit(void);
void COM_Init(void);
void COM_SendData(uint8_t *pu8Buff, uint16_t u16Len);
int32_t COM_RecvData(uint8_t *pu8Buff, uint16_t u16Len, uint32_t u32Timeout);

#ifdef __cplusplus
}
#endif

#endif /* __COM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
