/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */


#include "rtl8195a.h"
#include "rtl8195a_uart.h"
#include "hal_uart.h"
#include "hal_gdma.h"

#ifndef CONFIG_CHIP_E_CUT
// Pre-Defined Supported Baud Rate Table for CPU 166 MHz
const u32 DEF_BAUDRATE_TABLE[] = {
        110,     300,     600,    1200,
       2400,    4800,    9600,   14400,
      19200,   28800,   38400,   57600,
      76800,  115200,  128000,  153600,
     230400,  380400,  460800,  500000,
     921600, 1000000, 1382400, 1444400,
    1500000, 1843200, 2000000, 2100000,
    2764800, 3000000, 3250000, 3692300,
    3750000, 4000000, 6000000,

      56000,  256000,
    
    // For UART to IR Carrier
      66000,   72000,   73400,   76000,
      80000,  112000,

    // End of the table
    0xffffffff
};

const u16 ovsr_adj_table_10bit[10] = {
    0x000, 0x020, 0x044, 0x124, 0x294, 0x2AA, 0x16B, 0x2DB, 0x3BB, 0x3EF
};

const u16 ovsr_adj_table_9bit[9] = {
    0x000, 0x010, 0x044, 0x92, 0xAA, 0x155, 0x1B6, 0x1BB, 0x1EF
};

const u16 ovsr_adj_table_8bit[8] = {
    0x000, 0x010, 0x044, 0x92, 0xAA, 0xB5, 0xBB, 0xEF
};

#if 0   // Old format
#if defined(CONFIG_CHIP_A_CUT) || defined(CONFIG_CHIP_B_CUT)

const u8 DEF_OVSR_166[] = {
    10, 10, 12, 14,
    10, 10, 10, 11,
    14, 11, 14, 11,
    14, 10, 11, 14,
    18, 17, 17, 18,
    17, 13, 19, 18,
    10, 11, 13, 19,
    14, 13, 12, 11,
    10, 10, 13,

    20, 18,

    // For UART to IR Carrier
    13, 13, 18, 15,
    20, 12,
};

const u16 DEF_DIV_166[] = {
    74272, 27233, 11347,  4863,  
     3404,  1702,   851,   516,
      304,   258,   152,   129,
       76,    71,    58,    38,
       19,    12,    10,     9,
        5,     6,     3,     3,
        5,     4,     3,     2,
        2,     2,     2,     2,
        2,     2,     1,        

       73,    17,

    // For UART to IR Carrier
       97,    89,    63,    73,
       52,    62,
};


const u16 DEF_OVSR_ADJ_166[] = {
    0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000,
    0x2AA, 0x3BB, 0x1B6, 0x010,
    0x1B6, 0x2AA, 0x1B6, 0x2DB,
    0x3BB, 0x000, 0x2AA, 0x294,
    0x2DB, 0x2AA, 0x2AA, 0x000 ,
    0x3BB, 0x088, 0x2AA,

    0x000, 0x2DB,
    
    // For UART to IR Carrier
    0x000, 0x000, 0x000, 0x000,
    0x000, 0x000
};
#endif  //#if defined(CONFIG_CHIP_A_CUT) || defined(CONFIG_CHIP_B_CUT)

#ifdef CONFIG_CHIP_C_CUT

const u8 DEF_OVSR_166[] = {
    13, 12, 12, 12,
    18, 10, 10, 11,
    10, 11, 10, 20,
    20, 20, 20, 20,
    20, 18, 20, 12,
    15, 16, 20, 19,
    18, 15, 10, 13,
    15, 13, 12, 11,
    11, 10, 13, 

    16, 18,
    
    // For UART to IR Carrier
    13, 13, 18, 15,
    20, 12,
};

const u16 DEF_DIV_166[] = {
    58275, 23148, 11574,  5787,
     1929,  1736,   868,   526,
      434,   263,   217,    72,
       54,    36,    32,    27,
       18,    12,     9,    13,
        6,     5,     3,     3,
        3,     3,     4,     3,
        2,     2,     2,     2,
        2,     2,     1,     

       93,    18,

    // For UART to IR Carrier    
       97,    89,    63,    73,
       52,    62,
};

const u16 DEF_OVSR_ADJ_166[] = {
    0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x010,
    0x010, 0x010, 0x124, 0x010,
    0x010, 0x088, 0x010, 0x2DB,
    0x000, 0x16B, 0x010, 0x088,
    0x2AA, 0x000, 0x294, 0x088,
    0x000, 0x3BB, 0x3BB, 0x088,
    0x010, 0x294, 0x3BB, 

    0x000, 0x010,

    // For UART to IR Carrier
    0x000, 0x000, 0x000, 0x000,
    0x000, 0x000
};

#endif  // #ifdef CONFIG_CHIP_C_CUT
#endif  // end of #if 0   // Old format

#if defined(CONFIG_CHIP_A_CUT) || defined(CONFIG_CHIP_B_CUT)
const u8 DEF_OVSR_B_CUT[] = {
    20, 20, 20, 20,
    20, 20, 15, 18,
    13, 15, 18, 13,
    18, 12, 11, 10,
    16, 15, 16, 18,
    11, 20, 19, 14,
    18, 11, 20, 19,
    14, 13, 12, 11,
    21, 20, 13,
    
    18, 11,
    
    // For UART to IR Carrier
    13, 13, 18, 15,
    20, 12
    
};

const u16 DEF_DIV_B_CUT[] = {
    37202, 13616,  6808,  3404, 
     1702,   851,   567,   315,  
      327,   189,   118,   109,  
       59,    59,    58,    53,   
       22,    14,    11,     9,    
        8,     4,     3,     4,    
        3,     4,     2,     2,    
        2,     2,     2,     2,    
        1,     1,     1,    
        
       81,    29,

    // For UART to IR Carrier    
       97,    89,   63,   73,   
       52,    62   
};

const u8 DEF_OVSR_ADJ_BITS_B_CUT_10B[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 3, 1, 2,
    1, 4, 7, 1,
    2, 1, 4, 5,
    8, 6, 6, 1,
    8, 4, 6, 
    
    0, 0,

    // For UART to IR Carrier
    0, 0, 0, 0,
    0, 0

};

const u8 DEF_OVSR_ADJ_BITS_B_CUT_9B[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 3, 1, 1,
    1, 4, 6, 1,
    1, 1, 4, 4,
    7, 6, 5, 1,
    7, 4, 6, 
    
    0, 0,

    // For UART to IR Carrier
    0, 0, 0, 0,
    0, 0

};

const u8 DEF_OVSR_ADJ_BITS_B_CUT_8B[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 3, 1, 1,
    1, 4, 6, 1,
    1, 1, 4, 4,
    6, 5, 5, 1,
    6, 4, 5, 
    
    0, 0,

    // For UART to IR Carrier
    0, 0, 0, 0,
    0, 0
};

#endif  // #if defined(CONFIG_CHIP_A_CUT) || defined(CONFIG_CHIP_B_CUT)

const u8 DEF_OVSR_C_CUT[] = {
    20, 20, 20, 20,
    20, 20, 20, 14,
    20, 12, 14, 19,
    19, 19, 13, 20,
    19, 18, 20, 15,
    18, 20, 20, 19,
    11, 15, 20, 13,
    15, 13, 12, 11,
    11, 20, 13, 

    16, 13, 
    
    // For UART to IR Carrier
    13, 13, 18, 15,
    20, 12    
};

const u16 DEF_DIV_C_CUT[] = {
    37878, 13888, 6944, 3472, 
     1736,   868,  434,  413,  
      217,   241,  155,   76,   
       57,    38,   50,   27,   
       19,    12,    9,   11,   
        5,     4,    3,    3,    
        5,     3,    2,    3,    
        2,     2,    2,    2,    
        2,     1,    1,     
        
       93,    25,   
       
    // For UART to IR Carrier    
       97,    89,   63,   73,   
       52,    62   
};

const u8 DEF_OVSR_ADJ_BITS_C_CUT_10B[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 1,
    0, 3, 1, 2,
    1, 8, 1, 2,
    1, 1, 8, 2,
    1, 9, 8, 3,
    1, 8, 9, 

    0, 0,

    // For UART to IR Carrier
    0, 0, 0, 0,
    0, 0

};

const u8 DEF_OVSR_ADJ_BITS_C_CUT_9B[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 1,
    0, 2, 1, 1,
    1, 8, 1, 2,
    1, 1, 8, 2,
    1, 8, 8, 3,
    1, 8, 8,

    0, 0,

    // For UART to IR Carrier
    0, 0, 0, 0,
    0, 0

};

const u8 DEF_OVSR_ADJ_BITS_C_CUT_8B[] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 1,
    0, 2, 1, 1,
    1, 7, 1, 2,
    1, 1, 7, 2,
    1, 7, 7, 2,
    1, 7, 7, 

    0, 0,

    // For UART to IR Carrier
    0, 0, 0, 0,
    0, 0
};

#endif  // #if !(CONFIG_CHIP_E_CUT)

extern u32 _UartIrqHandle(VOID *Data);

extern HAL_Status 
HalRuartInitRtl8195a_Patch(
        IN VOID *Data  ///< RUART Adapter
        );

#if (CONFIG_CHIP_C_CUT)
extern _LONG_CALL_ HAL_Status
HalRuartInitRtl8195aV02(
        IN VOID *Data  ///< RUART Adapter
        );
#endif

extern u8 HalRuartGetChipVerRtl8195a(VOID);

const HAL_GDMA_CHNL Uart2_TX_GDMA_Chnl_Option[] = { 
    {0,0,GDMA0_CHANNEL0_IRQ,0},
    {0,1,GDMA0_CHANNEL1_IRQ,0},
    {0,2,GDMA0_CHANNEL2_IRQ,0},
    {0,3,GDMA0_CHANNEL3_IRQ,0},
    {0,4,GDMA0_CHANNEL4_IRQ,0},
    {0,5,GDMA0_CHANNEL5_IRQ,0},
    
    {0xff,0,0,0}    // end
};

const HAL_GDMA_CHNL Uart2_RX_GDMA_Chnl_Option[] = { 
    {1,0,GDMA1_CHANNEL0_IRQ,0},
    {1,1,GDMA1_CHANNEL1_IRQ,0},
    {1,2,GDMA1_CHANNEL2_IRQ,0},
    {1,3,GDMA1_CHANNEL3_IRQ,0},
    {1,4,GDMA1_CHANNEL4_IRQ,0},
    {1,5,GDMA1_CHANNEL5_IRQ,0},
    
    {0xff,0,0,0}    // end
};

VOID
HalRuartOpInit(
        IN VOID *Data
)
{
    PHAL_RUART_OP pHalRuartOp = (PHAL_RUART_OP) Data;

    pHalRuartOp->HalRuartAdapterLoadDef     = HalRuartAdapterLoadDefRtl8195a;
    pHalRuartOp->HalRuartTxGdmaLoadDef      = HalRuartTxGdmaLoadDefRtl8195a;
    pHalRuartOp->HalRuartRxGdmaLoadDef      = HalRuartRxGdmaLoadDefRtl8195a;
    pHalRuartOp->HalRuartResetRxFifo        = HalRuartResetRxFifoRtl8195a;
#if CONFIG_CHIP_E_CUT
    pHalRuartOp->HalRuartInit               = HalRuartInitRtl8195a_V04;
#else
    pHalRuartOp->HalRuartInit               = HalRuartInitRtl8195a_Patch;         // Hardware Init ROM code patch 
#endif
    pHalRuartOp->HalRuartDeInit             = HalRuartDeInitRtl8195a;         // Hardware Init
    pHalRuartOp->HalRuartPutC               = HalRuartPutCRtl8195a;         // Send a byte
    pHalRuartOp->HalRuartSend               = HalRuartSendRtl8195a;         // Polling mode Tx
    pHalRuartOp->HalRuartIntSend            = HalRuartIntSendRtl8195a;      // Interrupt mode Tx
#if CONFIG_CHIP_E_CUT
    pHalRuartOp->HalRuartDmaSend            = HalRuartDmaSendRtl8195a_V04;      // DMA mode Tx
    pHalRuartOp->HalRuartStopSend           = HalRuartStopSendRtl8195a_V04;     // Stop non-blocking TX
#else
    pHalRuartOp->HalRuartDmaSend            = HalRuartDmaSendRtl8195a_Patch;      // DMA mode Tx
    pHalRuartOp->HalRuartStopSend           = HalRuartStopSendRtl8195a_Patch;     // Stop non-blocking TX
#endif    
    pHalRuartOp->HalRuartGetC               = HalRuartGetCRtl8195a;           // get a byte
    pHalRuartOp->HalRuartRecv               = HalRuartRecvRtl8195a;         // Polling mode Rx
    pHalRuartOp->HalRuartIntRecv            = HalRuartIntRecvRtl8195a;      // Interrupt mode Rx
    pHalRuartOp->HalRuartDmaRecv            = HalRuartDmaRecvRtl8195a;      // DMA mode Rx
#if CONFIG_CHIP_E_CUT
    pHalRuartOp->HalRuartStopRecv           = HalRuartStopRecvRtl8195a_V04;     // Stop non-blocking Rx
#else
    pHalRuartOp->HalRuartStopRecv           = HalRuartStopRecvRtl8195a_Patch; // Stop non-blocking Rx
#endif    
    pHalRuartOp->HalRuartGetIMR             = HalRuartGetIMRRtl8195a;
    pHalRuartOp->HalRuartSetIMR             = HalRuartSetIMRRtl8195a;
    pHalRuartOp->HalRuartGetDebugValue      = HalRuartGetDebugValueRtl8195a;
    pHalRuartOp->HalRuartDmaInit            = HalRuartDmaInitRtl8195a;
    pHalRuartOp->HalRuartRTSCtrl            = HalRuartRTSCtrlRtl8195a;
    pHalRuartOp->HalRuartRegIrq             = HalRuartRegIrqRtl8195a;
    pHalRuartOp->HalRuartIntEnable          = HalRuartIntEnableRtl8195a;
    pHalRuartOp->HalRuartIntDisable         = HalRuartIntDisableRtl8195a;
}

/**
 * Load UART HAL default setting
 *
 * Call this function to load the default setting for UART HAL adapter
 *
 *
 */
VOID
HalRuartAdapterInit(
    PRUART_ADAPTER pRuartAdapter,
    u8 UartIdx
)
{
    PHAL_RUART_OP pHalRuartOp;
    PHAL_RUART_ADAPTER pHalRuartAdapter;
    
    if (NULL == pRuartAdapter) {
        return;
    }
    
    pHalRuartOp = pRuartAdapter->pHalRuartOp;
    pHalRuartAdapter = pRuartAdapter->pHalRuartAdapter;

    if ((NULL == pHalRuartOp) || (NULL == pHalRuartAdapter)) {
        return;
    }

    // Load default setting
    if (pHalRuartOp->HalRuartAdapterLoadDef != NULL) {
        pHalRuartOp->HalRuartAdapterLoadDef (pHalRuartAdapter, UartIdx);
        pHalRuartAdapter->IrqHandle.Priority = 6;
    }
    else {
        // Initial your UART HAL adapter here
    }

    // Start to modify the defualt setting
    pHalRuartAdapter->PinmuxSelect = RUART0_MUX_TO_GPIOC;
    pHalRuartAdapter->BaudRate = 38400;

//    pHalRuartAdapter->IrqHandle.IrqFun = (IRQ_FUN)_UartIrqHandle;
//    pHalRuartAdapter->IrqHandle.Data = (void *)pHalRuartAdapter;

    // Register IRQ
    InterruptRegister(&pHalRuartAdapter->IrqHandle);
    
}

/**
 * Load UART HAL GDMA default setting
 *
 * Call this function to load the default setting for UART GDMA
 *
 *
 */
HAL_Status
HalRuartTxGdmaInit(
    PHAL_RUART_OP pHalRuartOp,
    PHAL_RUART_ADAPTER pHalRuartAdapter,
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL *pgdma_chnl;
    
    if ((NULL == pHalRuartOp) || (NULL == pHalRuartAdapter) || (NULL == pUartGdmaConfig)) {
        return HAL_ERR_PARA;
    }

    // Load default setting
    if (pHalRuartOp->HalRuartTxGdmaLoadDef != NULL) {
        pHalRuartOp->HalRuartTxGdmaLoadDef (pHalRuartAdapter, pUartGdmaConfig);
        pUartGdmaConfig->TxGdmaIrqHandle.Priority = 6;
    }
    else {
        // Initial your GDMA setting here
    }

    // Start to patch the default setting
    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pTxHalGdmaAdapter;
    if (HalGdmaChnlRegister(pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum) != HAL_OK) {
        // The default GDMA Channel is not available, try others
        if (pHalRuartAdapter->UartIndex == 2) {
            // UART2 TX Only can use GDMA 0
            pgdma_chnl = HalGdmaChnlAlloc((HAL_GDMA_CHNL*)Uart2_TX_GDMA_Chnl_Option);
        }
        else {
            pgdma_chnl = HalGdmaChnlAlloc(NULL);
        }

        if (pgdma_chnl == NULL) {
            // No Available DMA channel
            return HAL_BUSY;
        }
        else {
            pHalGdmaAdapter->GdmaIndex   = pgdma_chnl->GdmaIndx;
            pHalGdmaAdapter->ChNum       = pgdma_chnl->GdmaChnl;
            pHalGdmaAdapter->ChEn        = 0x0101 << pgdma_chnl->GdmaChnl;
            pUartGdmaConfig->TxGdmaIrqHandle.IrqNum = pgdma_chnl->IrqNum;            
        }
    }

    // User can assign a Interrupt Handler here
//    pUartGdmaConfig->TxGdmaIrqHandle.Data = pHalRuartAdapter;
//    pUartGdmaConfig->TxGdmaIrqHandle.IrqFun = (IRQ_FUN)_UartTxDmaIrqHandle
//    pUartGdmaConfig->TxGdmaIrqHandle.Priority = 0x20;

    pHalRuartOp->HalRuartDmaInit (pHalRuartAdapter);
    InterruptRegister(&pUartGdmaConfig->TxGdmaIrqHandle);
    InterruptEn(&pUartGdmaConfig->TxGdmaIrqHandle);

    return HAL_OK;
}

VOID
HalRuartTxGdmaDeInit(
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL GdmaChnl;

    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pTxHalGdmaAdapter;
    GdmaChnl.GdmaIndx = pHalGdmaAdapter->GdmaIndex;
    GdmaChnl.GdmaChnl = pHalGdmaAdapter->ChNum;
    GdmaChnl.IrqNum = pUartGdmaConfig->TxGdmaIrqHandle.IrqNum;
    HalGdmaChnlFree(&GdmaChnl);
}

/**
 * Load UART HAL GDMA default setting
 *
 * Call this function to load the default setting for UART GDMA
 *
 *
 */
HAL_Status
HalRuartRxGdmaInit(
    PHAL_RUART_OP pHalRuartOp,
    PHAL_RUART_ADAPTER pHalRuartAdapter,
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL *pgdma_chnl;
    
    if ((NULL == pHalRuartOp) || (NULL == pHalRuartAdapter) || (NULL == pUartGdmaConfig)) {
        return HAL_ERR_PARA;
    }

    // Load default setting
    if (pHalRuartOp->HalRuartRxGdmaLoadDef != NULL) {
        pHalRuartOp->HalRuartRxGdmaLoadDef (pHalRuartAdapter, pUartGdmaConfig);
        pUartGdmaConfig->RxGdmaIrqHandle.Priority = 6;
    }
    else {
        // Initial your GDMA setting here
    }

    // Start to patch the default setting
    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pRxHalGdmaAdapter;
    if (HalGdmaChnlRegister(pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum) != HAL_OK) {
        // The default GDMA Channel is not available, try others
        if (pHalRuartAdapter->UartIndex == 2) {
            // UART2 RX Only can use GDMA 1
            pgdma_chnl = HalGdmaChnlAlloc((HAL_GDMA_CHNL*)Uart2_RX_GDMA_Chnl_Option);
        }
        else {
            pgdma_chnl = HalGdmaChnlAlloc(NULL);
        }

        if (pgdma_chnl == NULL) {
            // No Available DMA channel
            return HAL_BUSY;
        }
        else {
            pHalGdmaAdapter->GdmaIndex   = pgdma_chnl->GdmaIndx;
            pHalGdmaAdapter->ChNum       = pgdma_chnl->GdmaChnl;
            pHalGdmaAdapter->ChEn        = 0x0101 << pgdma_chnl->GdmaChnl;
            pUartGdmaConfig->RxGdmaIrqHandle.IrqNum = pgdma_chnl->IrqNum;            
        }
    }

//    pUartGdmaConfig->RxGdmaIrqHandle.Data = pHalRuartAdapter;
//    pUartGdmaConfig->RxGdmaIrqHandle.IrqFun = (IRQ_FUN)_UartTxDmaIrqHandle;
//    pUartGdmaConfig->RxGdmaIrqHandle.Priority = 0x20;

    pHalRuartOp->HalRuartDmaInit (pHalRuartAdapter);
    InterruptRegister(&pUartGdmaConfig->RxGdmaIrqHandle);
    InterruptEn(&pUartGdmaConfig->RxGdmaIrqHandle);

    return HAL_OK;
}

VOID
HalRuartRxGdmaDeInit(
    PUART_DMA_CONFIG pUartGdmaConfig
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL GdmaChnl;

    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pUartGdmaConfig->pRxHalGdmaAdapter;
    GdmaChnl.GdmaIndx = pHalGdmaAdapter->GdmaIndex;
    GdmaChnl.GdmaChnl = pHalGdmaAdapter->ChNum;
    GdmaChnl.IrqNum = pUartGdmaConfig->RxGdmaIrqHandle.IrqNum;
    HalGdmaChnlFree(&GdmaChnl);
}

/**
 * Hook a RX indication callback
 *
 * To hook a callback function which will be called when a got a RX byte
 *
 *
 */
VOID
HalRuartRxIndHook(
    PRUART_ADAPTER pRuartAdapter,
    VOID *pCallback,
    VOID *pPara
)
{
    PHAL_RUART_ADAPTER pHalRuartAdapter = pRuartAdapter->pHalRuartAdapter;

    pHalRuartAdapter->RxDRCallback = (void (*)(void*))pCallback;
    pHalRuartAdapter->RxDRCbPara = pPara;

    // enable RX data ready interrupt
    pHalRuartAdapter->Interrupts |= RUART_IER_ERBI | RUART_IER_ELSI;
    pRuartAdapter->pHalRuartOp->HalRuartSetIMR(pHalRuartAdapter);    
}


HAL_Status
HalRuartResetTxFifo(
    IN VOID *Data
)
{
    return (HalRuartResetTxFifoRtl8195a(Data));
}

HAL_Status 
HalRuartSetBaudRate(
        IN VOID *Data
)
{
#if CONFIG_CHIP_E_CUT
    return HalRuartSetBaudRateRtl8195a_V04(Data);
#else
    return HalRuartSetBaudRateRtl8195a(Data);
#endif
}

HAL_Status 
HalRuartInit(
    IN VOID *Data
)
{
    HAL_Status ret;
    PHAL_RUART_ADAPTER pHalRuartAdapter = (PHAL_RUART_ADAPTER) Data;
#ifdef CONFIG_SOC_PS_MODULE
    REG_POWER_STATE UartPwrState;
#endif
#if CONFIG_CHIP_E_CUT
    pHalRuartAdapter->pDefaultBaudRateTbl = (uint32_t*)BAUDRATE_166_ROM_V04;
    pHalRuartAdapter->pDefaultOvsrRTbl = (uint8_t*)OVSR_166_ROM_V04;
    pHalRuartAdapter->pDefaultOvsrAdjTbl = (uint16_t*)OVSR_ADJ_166_ROM_V04;
    pHalRuartAdapter->pDefaultDivTbl = (uint16_t*)DIV_166_ROM_V04;
    ret = HalRuartInitRtl8195a_V04(Data);
#else
    pHalRuartAdapter->pDefaultBaudRateTbl = (uint32_t*)DEF_BAUDRATE_TABLE;
#if defined(CONFIG_CHIP_A_CUT) || defined(CONFIG_CHIP_B_CUT)
    u8 chip_ver;

    chip_ver = HalRuartGetChipVerRtl8195a();
    if (chip_ver < 2) {
        pHalRuartAdapter->pDefaultOvsrRTbl = (uint8_t*)DEF_OVSR_B_CUT;
        pHalRuartAdapter->pDefaultDivTbl = (uint16_t*)DEF_DIV_B_CUT;
        pHalRuartAdapter->pDefOvsrAdjBitTbl_10 = (uint8_t*)DEF_OVSR_ADJ_BITS_B_CUT_10B;
        pHalRuartAdapter->pDefOvsrAdjBitTbl_9 = (uint8_t*)DEF_OVSR_ADJ_BITS_B_CUT_9B;
        pHalRuartAdapter->pDefOvsrAdjBitTbl_8 = (uint8_t*)DEF_OVSR_ADJ_BITS_B_CUT_8B;
    } 
    else 
#endif
    {
        pHalRuartAdapter->pDefaultOvsrRTbl = (uint8_t*)DEF_OVSR_C_CUT;
        pHalRuartAdapter->pDefaultDivTbl = (uint16_t*)DEF_DIV_C_CUT;
        pHalRuartAdapter->pDefOvsrAdjBitTbl_10 = (uint8_t*)DEF_OVSR_ADJ_BITS_C_CUT_10B;
        pHalRuartAdapter->pDefOvsrAdjBitTbl_9 = (uint8_t*)DEF_OVSR_ADJ_BITS_C_CUT_9B;
        pHalRuartAdapter->pDefOvsrAdjBitTbl_8 = (uint8_t*)DEF_OVSR_ADJ_BITS_C_CUT_8B;
    }
    pHalRuartAdapter->pDefOvsrAdjTbl_10 = (uint16_t*)ovsr_adj_table_10bit;
    pHalRuartAdapter->pDefOvsrAdjTbl_9 = (uint16_t*)ovsr_adj_table_9bit;
    pHalRuartAdapter->pDefOvsrAdjTbl_8 = (uint16_t*)ovsr_adj_table_8bit;
    
    ret = HalRuartInitRtl8195a_Patch(Data);
#endif    

#ifdef CONFIG_SOC_PS_MODULE
    if(ret == HAL_OK) {
        // To register a new peripheral device power state
        UartPwrState.FuncIdx = UART0 + pHalRuartAdapter->UartIndex;
        UartPwrState.PwrState = ACT;
        RegPowerState(UartPwrState);
    }
#endif
    return ret;
}

VOID 
HalRuartDeInit(
    IN VOID *Data
)
{
#ifdef CONFIG_SOC_PS_MODULE
    REG_POWER_STATE UartPwrState;
    PHAL_RUART_ADAPTER pHalRuartAdapter = (PHAL_RUART_ADAPTER) Data;
    u8 HwState;

    UartPwrState.FuncIdx = UART0 + pHalRuartAdapter->UartIndex;
    QueryRegPwrState(UartPwrState.FuncIdx, &(UartPwrState.PwrState), &HwState);

    // if the power state isn't ACT, then switch the power state back to ACT first
    if ((UartPwrState.PwrState != ACT) && (UartPwrState.PwrState != INACT)) {
        HalRuartEnable(Data);
        QueryRegPwrState(UartPwrState.FuncIdx, &(UartPwrState.PwrState), &HwState);
    }

    if (UartPwrState.PwrState == ACT) {
        UartPwrState.PwrState = INACT;
        RegPowerState(UartPwrState);
    }    
#endif

    HalRuartDeInitRtl8195a(Data);    
}

HAL_Status 
HalRuartDisable(
    IN VOID *Data
)
{
    HAL_Status ret;
#ifdef CONFIG_SOC_PS_MODULE
    REG_POWER_STATE UartPwrState;
    PHAL_RUART_ADAPTER pHalRuartAdapter = (PHAL_RUART_ADAPTER) Data;
#endif

#if CONFIG_CHIP_E_CUT
    ret = HalRuartDisableRtl8195a_V04(Data);
#else
    ret = HalRuartDisableRtl8195a(Data);
#endif
#ifdef CONFIG_SOC_PS_MODULE
    if (ret == HAL_OK) {
        UartPwrState.FuncIdx = UART0 + pHalRuartAdapter->UartIndex;
        UartPwrState.PwrState = SLPCG;
        RegPowerState(UartPwrState);
    }
#endif
    return ret;
}

HAL_Status 
HalRuartEnable(
    IN VOID *Data
)
{
    HAL_Status ret;
#ifdef CONFIG_SOC_PS_MODULE
    REG_POWER_STATE UartPwrState;
    PHAL_RUART_ADAPTER pHalRuartAdapter = (PHAL_RUART_ADAPTER) Data;
#endif

#if CONFIG_CHIP_E_CUT
    ret = HalRuartEnableRtl8195a_V04(Data);
#else
    ret = HalRuartEnableRtl8195a(Data);
#endif
#ifdef CONFIG_SOC_PS_MODULE
    if (ret == HAL_OK) {
        UartPwrState.FuncIdx = UART0 + pHalRuartAdapter->UartIndex;
        UartPwrState.PwrState = ACT;
        RegPowerState(UartPwrState);
    }
#endif
    return ret;
}

HAL_Status 
HalRuartFlowCtrl(
    IN VOID *Data
)
{
    HAL_Status ret;

#if CONFIG_CHIP_E_CUT
    ret = HalRuartFlowCtrlRtl8195a_V04((VOID *)Data);
#else
    ret = HalRuartFlowCtrlRtl8195a((VOID *)Data);
#endif
    return ret;
}

VOID
HalRuartEnterCritical(
        IN VOID *Data
)
{
#if CONFIG_CHIP_E_CUT
    HalRuartEnterCriticalRtl8195a_V04(Data);
#else
    HalRuartEnterCriticalRtl8195a(Data);
#endif
}

VOID
HalRuartExitCritical(
        IN VOID *Data
)
{
#if CONFIG_CHIP_E_CUT
    HalRuartExitCriticalRtl8195a_V04(Data);
#else
    HalRuartExitCriticalRtl8195a(Data);
#endif
}

