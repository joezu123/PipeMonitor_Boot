/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\source\com.c
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
#include "com.h"

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

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/* DMA definition */
#define RX_DMA_UNIT                     (CM_DMA1)
#define RX_DMA_CH                       (DMA_CH0)
#define RX_DMA_FCG_ENABLE()             (FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DMA1, ENABLE))
#define RX_DMA_TRIG_SEL                 (AOS_DMA1_0)
#define RX_DMA_TRIG_EVT_SRC             (EVT_SRC_USART1_RI)
#define RX_DMA_RECONF_TRIG_SEL          (AOS_DMA_RC)
#define RX_DMA_RECONF_TRIG_EVT_SRC      (EVT_SRC_AOS_STRG)
#define RX_DMA_TC_INT                   (DMA_INT_TC_CH0)
#define RX_DMA_TC_FLAG                  (DMA_FLAG_TC_CH0)
#define RX_DMA_TC_IRQn                  (INT000_IRQn)
#define RX_DMA_TC_INT_SRC               (INT_SRC_DMA1_TC0)

/* Timer0 unit & channel definition */
#define TMR0_UNIT                       (CM_TMR0_1)
#define TMR0_CH                         (TMR0_CH_A)
#define TMR0_FCG_ENABLE()               (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR0_1, ENABLE))

/* USART RX/TX pin definition */
#define USART_RX_PORT                   (GPIO_PORT_C)   /* PC0: USART1_RX */
#define USART_RX_PIN                    (GPIO_PIN_00)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_33)

#define USART_TX_PORT                   (GPIO_PORT_C)   /* PC1: USART1_TX */
#define USART_TX_PIN                    (GPIO_PIN_01)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_32)

/* USART unit definition */
#define USART_UNIT                      (CM_USART1)
#define USART_FCG_ENABLE()              (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART1, ENABLE))

/* USART baudrate definition */
#define USART_BAUDRATE                  (115200UL)

/* USART timeout bits definition */
#define USART_TIMEOUT_BITS              (1000U)

/* USART interrupt definition */
//#define USART_TX_CPLT_IRQn              (INT002_IRQn)
//#define USART_TX_CPLT_INT_SRC           (INT_SRC_USART1_TCI)

#define USART_RX_ERR_IRQn               (INT003_IRQn)
#define USART_RX_ERR_INT_SRC            (INT_SRC_USART1_EI)

#define USART_RX_TIMEOUT_IRQn           (INT004_IRQn)
#define USART_RX_TIMEOUT_INT_SRC        (INT_SRC_USART1_RTO)



/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static __IO en_flag_status_t m_enRxFrameEnd;
uint16_t m_u16RxLen = 0;
uint8_t m_RecvFlag = 0;
uint8_t m_au8RxBuf[APP_FRAME_LEN_MAX] = {0};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  DMA transfer complete IRQ callback function.
 * @param  None
 * @retval None
 */
static void RX_DMA_TC_IrqCallback(void)
{
    m_enRxFrameEnd = SET;
    m_RecvFlag = 1;
    m_u16RxLen = APP_FRAME_LEN_MAX;

    USART_FuncCmd(USART_UNIT, USART_RX_TIMEOUT, DISABLE);

    DMA_ClearTransCompleteStatus(RX_DMA_UNIT, RX_DMA_TC_FLAG);
}

/**
 * @brief  Initialize DMA.
 * @param  None
 * @retval int32_t:
 *           - LL_OK:                   Initialize successfully.
 *           - LL_ERR_INVD_PARAM:       Initialization parameters is invalid.
 */
static int32_t DMA_Config(void)
{
    int32_t i32Ret;
    stc_dma_init_t stcDmaInit;
    stc_dma_llp_init_t stcDmaLlpInit;
    stc_irq_signin_config_t stcIrqSignConfig;
    static stc_dma_llp_descriptor_t stcLlpDesc;

    /* DMA&AOS FCG enable */
    RX_DMA_FCG_ENABLE();
    //TX_DMA_FCG_ENABLE();
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

    /* USART_RX_DMA */
    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32IntEn = DMA_INT_ENABLE;
    stcDmaInit.u32BlockSize = 1UL;
    stcDmaInit.u32TransCount = ARRAY_SZ(m_au8RxBuf);
    stcDmaInit.u32DataWidth = DMA_DATAWIDTH_8BIT;
    stcDmaInit.u32DestAddr = (uint32_t)m_au8RxBuf;
    stcDmaInit.u32SrcAddr = (uint32_t)(&USART_UNIT->RDR);
    stcDmaInit.u32SrcAddrInc = DMA_SRC_ADDR_FIX;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;
    i32Ret = DMA_Init(RX_DMA_UNIT, RX_DMA_CH, &stcDmaInit);
    if (LL_OK == i32Ret) {
        (void)DMA_LlpStructInit(&stcDmaLlpInit);
        stcDmaLlpInit.u32State = DMA_LLP_ENABLE;
        stcDmaLlpInit.u32Mode  = DMA_LLP_WAIT;
        stcDmaLlpInit.u32Addr  = (uint32_t)&stcLlpDesc;
        (void)DMA_LlpInit(RX_DMA_UNIT, RX_DMA_CH, &stcDmaLlpInit);

        stcLlpDesc.SARx   = stcDmaInit.u32SrcAddr;
        stcLlpDesc.DARx   = stcDmaInit.u32DestAddr;
        stcLlpDesc.DTCTLx = (stcDmaInit.u32TransCount << DMA_DTCTL_CNT_POS) | (stcDmaInit.u32BlockSize << DMA_DTCTL_BLKSIZE_POS);;
        stcLlpDesc.LLPx   = (uint32_t)&stcLlpDesc;
        stcLlpDesc.CHCTLx = stcDmaInit.u32SrcAddrInc | stcDmaInit.u32DestAddrInc | stcDmaInit.u32DataWidth |  \
                            stcDmaInit.u32IntEn      | stcDmaLlpInit.u32State    | stcDmaLlpInit.u32Mode;

        DMA_ReconfigLlpCmd(RX_DMA_UNIT, RX_DMA_CH, ENABLE);
        DMA_ReconfigCmd(RX_DMA_UNIT, ENABLE);
        AOS_SetTriggerEventSrc(RX_DMA_RECONF_TRIG_SEL, RX_DMA_RECONF_TRIG_EVT_SRC);

        stcIrqSignConfig.enIntSrc = RX_DMA_TC_INT_SRC;
        stcIrqSignConfig.enIRQn  = RX_DMA_TC_IRQn;
        stcIrqSignConfig.pfnCallback = &RX_DMA_TC_IrqCallback;
        (void)INTC_IrqSignIn(&stcIrqSignConfig);
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        AOS_SetTriggerEventSrc(RX_DMA_TRIG_SEL, RX_DMA_TRIG_EVT_SRC);

        DMA_Cmd(RX_DMA_UNIT, ENABLE);
        DMA_TransCompleteIntCmd(RX_DMA_UNIT, RX_DMA_TC_INT, ENABLE);
        (void)DMA_ChCmd(RX_DMA_UNIT, RX_DMA_CH, ENABLE);
    }

    return i32Ret;
}

/**
 * @brief  Configure TMR0.
 * @param  [in] u16TimeoutBits:         Timeout bits
 * @retval None
 */
static void TMR0_Config(uint16_t u16TimeoutBits)
{
    uint16_t u16Div;
    uint16_t u16Delay;
    uint16_t u16CompareValue;
    stc_tmr0_init_t stcTmr0Init;

    TMR0_FCG_ENABLE();

    /* Initialize TMR0 base function. */
    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC_XTAL32;
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV8;
    stcTmr0Init.u32Func     = TMR0_FUNC_CMP;
    if (TMR0_CLK_DIV1 == stcTmr0Init.u32ClockDiv) {
        u16Delay = 7U;
    } else if (TMR0_CLK_DIV2 == stcTmr0Init.u32ClockDiv) {
        u16Delay = 5U;
    } else if ((TMR0_CLK_DIV4 == stcTmr0Init.u32ClockDiv) || \
               (TMR0_CLK_DIV8 == stcTmr0Init.u32ClockDiv) || \
               (TMR0_CLK_DIV16 == stcTmr0Init.u32ClockDiv)) {
        u16Delay = 1U;
    } else {
        u16Delay = 2U;
    }

    u16Div = (uint16_t)1U << (stcTmr0Init.u32ClockDiv >> TMR0_BCONR_CKDIVA_POS);
    u16CompareValue = ((u16TimeoutBits + u16Div - 1U) / u16Div) - u16Delay;
    stcTmr0Init.u16CompareValue = u16CompareValue;
    (void)TMR0_Init(TMR0_UNIT, TMR0_CH, &stcTmr0Init);

    TMR0_HWStartCondCmd(TMR0_UNIT, TMR0_CH, ENABLE);
    TMR0_HWClearCondCmd(TMR0_UNIT, TMR0_CH, ENABLE);
}

/**
 * @brief  Stop timeout timer.
 * @param  [in]  TMR0x                  Pointer to TMR0 instance register base.
 *                                      This parameter can be a value of the following:
 *   @arg  CM_TMR0_x or CM_TMR0
 * @param  [in]  u32Ch                  TMR0 channel.
 *                                      This parameter can be a value @ref TMR0_Channel
 */
static void USART_StopTimeoutTimer(CM_TMR0_TypeDef *TMR0x, uint32_t u32Ch)
{
    uint32_t u32ClrMask;
    uint32_t u32SetMask;
    uint32_t u32BitOffset;

    u32BitOffset = 16UL * u32Ch;

    /* Set: TMR0_BCONR.SYNCLKA<B>=1, TMR0_BCONR.SYNA<B>=0 */
    u32ClrMask = (TMR0_BCONR_SYNCLKA | TMR0_BCONR_SYNSA) << u32BitOffset;
    u32SetMask = TMR0_BCONR_SYNCLKA << u32BitOffset;
    MODIFY_REG32(TMR0x->BCONR, u32ClrMask, u32SetMask);

    /* Set: TMR0_BCONR.CSTA<B>=0, TMR0_BCONR.SYNCLKA<B>=0, TMR0_BCONR.SYNSA<B>=1 */
    u32ClrMask = (TMR0_BCONR_SYNCLKA | TMR0_BCONR_SYNSA | TMR0_BCONR_CSTA) << u32BitOffset;
    u32SetMask = TMR0_BCONR_SYNSA << u32BitOffset;
    MODIFY_REG32(TMR0x->BCONR, u32ClrMask, u32SetMask);
}

/**
 * @brief  USART RX timeout IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxTimeout_IrqCallback(void)
{
    if (m_enRxFrameEnd != SET) {
        m_enRxFrameEnd = SET;
        m_RecvFlag = 1;
        m_u16RxLen = APP_FRAME_LEN_MAX - (uint16_t)DMA_GetTransCount(RX_DMA_UNIT, RX_DMA_CH);

        /* Trigger for re-config USART RX DMA */
        AOS_SW_Trigger();
    }

    USART_StopTimeoutTimer(TMR0_UNIT, TMR0_CH);

    USART_ClearStatus(USART_UNIT, USART_FLAG_RX_TIMEOUT);
}

/**
 * @brief  USART RX error IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxError_IrqCallback(void)
{
    (void)USART_ReadData(USART_UNIT);

    USART_ClearStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR | USART_FLAG_OVERRUN));
}


/**
 * @brief  COM De-Initialize.
 * @param  None
 * @retval None
 */
void COM_DeInit(void)
{
    USART_DeInit(MODEM_USART_UNIT);
    /* Disable USART clock */
    MODEM_USART_CLK_CTRL(DISABLE);
    /* Configure USART RX/TX pin */
    GPIO_SetFunc(MODEM_USART_RX_PORT, MODEM_USART_RX_PIN, GPIO_FUNC_0);
    GPIO_SetFunc(MODEM_USART_TX_PORT, MODEM_USART_TX_PIN, GPIO_FUNC_0);
    NVIC_DisableIRQ(RX_DMA_TC_IRQn);
    NVIC_DisableIRQ(USART_RX_ERR_IRQn);
    NVIC_DisableIRQ(USART_RX_TIMEOUT_IRQn);
}


/**
 * @brief  COM Initialize.
 * @param  None
 * @retval None
 */
void COM_Init(void)
{
    stc_usart_uart_init_t stcUartInit;
    stc_irq_signin_config_t stcIrqSigninConfig;

    /* Initialize DMA. */
    (void)DMA_Config();

    /* Initialize TMR0. */
    TMR0_Config(USART_TIMEOUT_BITS);

    /* Configure USART RX/TX pin */
    GPIO_SetFunc(MODEM_USART_RX_PORT, MODEM_USART_RX_PIN, MODEM_USART_RX_FUNC);
    GPIO_SetFunc(MODEM_USART_TX_PORT, MODEM_USART_TX_PIN, MODEM_USART_TX_FUNC);
    /* Enable USART Clock. */
    MODEM_USART_CLK_CTRL(ENABLE);
    /* Initialize UART */
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockDiv      = USART_CLK_DIV64;//USART_CLK_DIV4;
    stcUartInit.u32CKOutput = USART_CK_OUTPUT_ENABLE;
    stcUartInit.u32Baudrate      = MODEM_USART_BAUD_RATE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;
    USART_UART_Init(MODEM_USART_UNIT, &stcUartInit, NULL);

    /* Register RX error IRQ handler. */
    stcIrqSigninConfig.enIRQn = USART_RX_ERR_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_ERR_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxError_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    /* Register RX timeout IRQ handler. */
    stcIrqSigninConfig.enIRQn = USART_RX_TIMEOUT_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_TIMEOUT_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxTimeout_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    /* Enable RX/TX function */
    //USART_FuncCmd(MODEM_USART_UNIT, (USART_RX | USART_TX), ENABLE);
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX | USART_RX_TIMEOUT | \
        USART_INT_RX_TIMEOUT | USART_TX ), ENABLE);
}

/**
 * @brief  COM send data.
 * @param  [in] pu8Buff                 Pointer to the buffer to be sent
 * @param  [in] u16Len                  Send buffer length
 * @retval None
 */
void COM_SendData(uint8_t *pu8Buff, uint16_t u16Len)
{
    USART_UART_Trans(MODEM_USART_UNIT, pu8Buff, u16Len, USART_MAX_TIMEOUT);
    m_enRxFrameEnd = RESET;
}

/**
 * @brief  COM receive data.
 * @param  [out] pu8Buff                Pointer to the buffer to be sent
 * @param  [in]  u16Len                 Receive data length
 * @param  [in]  u32Timeout             Receive timeout(ms)
 * @retval int32_t:
 *           - LL_OK: Receive data finished
 *           - LL_ERR: Receive error
 *           - LL_ERR_INVD_PARAM: u32Len value is 0 or the pointer pvBuf value is NULL.
 */
int32_t COM_RecvData(uint8_t *pu8Buff, uint16_t u16Len, uint32_t u32Timeout)
{
    if ((NULL == pu8Buff) || (0U == u16Len)) {
        return LL_ERR_INVD_PARAM;
    }

    /* Clear the RX error */
    if (RESET != USART_GetStatus(MODEM_USART_UNIT, (USART_FLAG_OVERRUN | USART_FLAG_FRAME_ERR | USART_FLAG_PARITY_ERR))) {
        USART_ClearStatus(MODEM_USART_UNIT, (USART_FLAG_OVERRUN | USART_FLAG_FRAME_ERR | USART_FLAG_PARITY_ERR));
    }
    if (LL_OK != USART_UART_Receive(MODEM_USART_UNIT, pu8Buff, u16Len, u32Timeout * 1000U)) {
        return LL_ERR;
    }
    return LL_OK;
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
