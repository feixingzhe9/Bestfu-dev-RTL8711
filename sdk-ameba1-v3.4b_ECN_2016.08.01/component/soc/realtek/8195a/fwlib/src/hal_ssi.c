/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include "rtl8195a.h"
#include "hal_ssi.h"

const HAL_GDMA_CHNL Ssi2_TX_GDMA_Chnl_Option[] = { 
    {0,4,GDMA0_CHANNEL4_IRQ,0},
    {0,5,GDMA0_CHANNEL5_IRQ,0},
    {0,3,GDMA0_CHANNEL3_IRQ,0},
    {0,0,GDMA0_CHANNEL0_IRQ,0},
    {0,1,GDMA0_CHANNEL1_IRQ,0},
    {0,2,GDMA0_CHANNEL2_IRQ,0},
    
    {0xff,0,0,0}    // end
};

const HAL_GDMA_CHNL Ssi2_RX_GDMA_Chnl_Option[] = { 
    {1,4,GDMA1_CHANNEL4_IRQ,0},
    {1,5,GDMA1_CHANNEL5_IRQ,0},
    {1,3,GDMA1_CHANNEL3_IRQ,0},
    {1,0,GDMA1_CHANNEL0_IRQ,0},
    {1,1,GDMA1_CHANNEL1_IRQ,0},
    {1,2,GDMA1_CHANNEL2_IRQ,0},
    
    {0xff,0,0,0}    // end
};

//TODO: Load default Setting: It should be loaded from external setting file.
const DW_SSI_DEFAULT_SETTING SpiDefaultSetting =
{
    .RxCompCallback    = NULL,
    .RxCompCbPara      = NULL,
    .RxData            = NULL,
    .TxCompCallback    = NULL,
    .TxCompCbPara      = NULL,
    .TxData            = NULL,
    .DmaRxDataLevel    =    7,  // RX FIFO stored bytes > (DMARDLR(7) + 1) then trigger DMA transfer
    .DmaTxDataLevel    =   48,  // TX FIFO free space > (FIFO_SPACE(64)-DMATDLR(48)) then trigger DMA transfer
    .InterruptPriority = 0x20,
    .RxLength          =    0,
    .RxLengthRemainder =    0,
    .RxThresholdLevel  =    7,  // if number of entries in th RX FIFO >= (RxThresholdLevel+1), RX interrupt asserted
    .TxLength          =    0,
    .TxThresholdLevel  =    8,  // if number of entries in th TX FIFO <= TxThresholdLevel, TX interrupt asserted
    .SlaveSelectEnable =    0,
    .ClockDivider      = SSI_CLK_SPI0_2/1000000,    // SCLK=1M
    .DataFrameNumber   =    0,
    .ControlFrameSize  = CFS_1_BIT,
    .DataFrameFormat   = FRF_MOTOROLA_SPI,
    .DataFrameSize     = DFS_8_BITS,
    .DmaControl        = 0, // default DMA is disable
    .InterruptMask     =  0x0,
    .MicrowireDirection    = MW_DIRECTION_MASTER_TO_SLAVE,
    .MicrowireHandshaking  = MW_HANDSHAKE_DISABLE,
    .MicrowireTransferMode = MW_TMOD_NONSEQUENTIAL,
    .SclkPhase             = SCPH_TOGGLES_AT_START,
    .SclkPolarity          = SCPOL_INACTIVE_IS_HIGH,
    .SlaveOutputEnable     = SLV_TXD_ENABLE,  // Slave
    .TransferMode          = TMOD_TR,
    .TransferMechanism     = SSI_DTM_INTERRUPT
};

extern HAL_Status HalSsiInitRtl8195a_Patch(VOID *Adaptor);
extern HAL_Status HalSsiPinmuxEnableRtl8195a_Patch(VOID *Adaptor);
extern HAL_Status HalSsiPinmuxDisableRtl8195a(VOID *Adaptor);
extern HAL_Status HalSsiDeInitRtl8195a(VOID * Adapter);
extern HAL_Status HalSsiClockOffRtl8195a(VOID * Adapter);
extern HAL_Status HalSsiClockOnRtl8195a(VOID * Adapter);
extern HAL_Status HalSsiIntReadRtl8195a(VOID *Adapter, VOID *RxData, u32 Length);
extern HAL_Status HalSsiIntWriteRtl8195a(VOID *Adapter, u8 *pTxData, u32 Length);
extern VOID HalSsiSetSclkRtl8195a(VOID *Adapter, u32 ClkRate);
#ifdef CONFIG_GDMA_EN
extern VOID HalSsiDmaInitRtl8195a(VOID *Adapter);
#endif

VOID HalSsiOpInit(VOID *Adaptor)
{
    PHAL_SSI_OP pHalSsiOp = (PHAL_SSI_OP) Adaptor;

//    pHalSsiOp->HalSsiPinmuxEnable            = HalSsiPinmuxEnableRtl8195a;
    pHalSsiOp->HalSsiPinmuxEnable            = HalSsiPinmuxEnableRtl8195a_Patch;
    pHalSsiOp->HalSsiPinmuxDisable           = HalSsiPinmuxDisableRtl8195a;
    pHalSsiOp->HalSsiEnable                  = HalSsiEnableRtl8195a;
    pHalSsiOp->HalSsiDisable                 = HalSsiDisableRtl8195a;
//    pHalSsiOp->HalSsiInit                    = HalSsiInitRtl8195a;
    pHalSsiOp->HalSsiInit                    = HalSsiInitRtl8195a_Patch;
    pHalSsiOp->HalSsiSetSclkPolarity         = HalSsiSetSclkPolarityRtl8195a;
    pHalSsiOp->HalSsiSetSclkPhase            = HalSsiSetSclkPhaseRtl8195a;
    pHalSsiOp->HalSsiWrite                   = HalSsiWriteRtl8195a;
    pHalSsiOp->HalSsiRead                    = HalSsiReadRtl8195a;
    pHalSsiOp->HalSsiGetRxFifoLevel          = HalSsiGetRxFifoLevelRtl8195a;
    pHalSsiOp->HalSsiGetTxFifoLevel          = HalSsiGetTxFifoLevelRtl8195a;
    pHalSsiOp->HalSsiGetStatus               = HalSsiGetStatusRtl8195a;
    pHalSsiOp->HalSsiGetInterruptStatus      = HalSsiGetInterruptStatusRtl8195a;
    pHalSsiOp->HalSsiLoadSetting             = HalSsiLoadSettingRtl8195a;
    pHalSsiOp->HalSsiSetInterruptMask        = HalSsiSetInterruptMaskRtl8195a;
    pHalSsiOp->HalSsiGetInterruptMask        = HalSsiGetInterruptMaskRtl8195a;
    pHalSsiOp->HalSsiSetDeviceRole           = HalSsiSetDeviceRoleRtl8195a;
    pHalSsiOp->HalSsiWriteable               = HalSsiWriteableRtl8195a;
    pHalSsiOp->HalSsiReadable                = HalSsiReadableRtl8195a;
    pHalSsiOp->HalSsiBusy                    = HalSsiBusyRtl8195a;
    pHalSsiOp->HalSsiInterruptEnable         = HalSsiInterruptEnableRtl8195a;
    pHalSsiOp->HalSsiInterruptDisable        = HalSsiInterruptDisableRtl8195a;
//    pHalSsiOp->HalSsiReadInterrupt           = HalSsiReadInterruptRtl8195a;
    pHalSsiOp->HalSsiReadInterrupt           = HalSsiIntReadRtl8195a;
    pHalSsiOp->HalSsiSetRxFifoThresholdLevel = HalSsiSetRxFifoThresholdLevelRtl8195a;
    pHalSsiOp->HalSsiSetTxFifoThresholdLevel = HalSsiSetTxFifoThresholdLevelRtl8195a;
//    pHalSsiOp->HalSsiWriteInterrupt          = HalSsiWriteInterruptRtl8195a;
    pHalSsiOp->HalSsiWriteInterrupt          = HalSsiIntWriteRtl8195a;
    pHalSsiOp->HalSsiGetRawInterruptStatus   = HalSsiGetRawInterruptStatusRtl8195a;
    pHalSsiOp->HalSsiGetSlaveEnableRegister  = HalSsiGetSlaveEnableRegisterRtl8195a;
    pHalSsiOp->HalSsiSetSlaveEnableRegister  = HalSsiSetSlaveEnableRegisterRtl8195a;
}


#ifdef CONFIG_GDMA_EN    

HAL_Status
HalSsiTxGdmaInit(
    IN PHAL_SSI_OP pHalSsiOp,
    IN PHAL_SSI_ADAPTOR pHalSsiAdapter
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    PSSI_DMA_CONFIG pDmaConfig;    
    HAL_GDMA_CHNL *pgdma_chnl;
    PHAL_GDMA_OP pHalGdmaOp;
    
    if ((NULL == pHalSsiOp) || (NULL == pHalSsiAdapter)) {
        return HAL_ERR_PARA;
    }

    pDmaConfig = &pHalSsiAdapter->DmaConfig;

    // Load default setting
    HalSsiTxGdmaLoadDefRtl8195a((void*)pHalSsiAdapter);

    // Start to patch the default setting
    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pDmaConfig->pTxHalGdmaAdapter;
    if (HalGdmaChnlRegister(pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum) != HAL_OK) {
        // The default GDMA Channel is not available, try others
        if (pHalSsiAdapter->Index == 2) {
            // SSI2 TX Only can use GDMA 0
            pgdma_chnl = HalGdmaChnlAlloc((HAL_GDMA_CHNL*)Ssi2_TX_GDMA_Chnl_Option);
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
            pDmaConfig->TxGdmaIrqHandle.IrqNum = pgdma_chnl->IrqNum;            
        }
    }

    DBG_SSI_INFO("HalSsiTxGdmaInit: GdmaIndex=%d ChNum=%d \r\n", pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum);
    pHalGdmaOp = (PHAL_GDMA_OP)pDmaConfig->pHalGdmaOp;
    pHalGdmaOp->HalGdmaOnOff((VOID*)(pHalGdmaAdapter));
    pHalGdmaOp->HalGdmaChIsrEnAndDis((VOID*)(pHalGdmaAdapter));

    HalSsiDmaInit(pHalSsiAdapter);
    InterruptRegister(&pDmaConfig->TxGdmaIrqHandle);
    InterruptEn(&pDmaConfig->TxGdmaIrqHandle);

    return HAL_OK;
}

VOID
HalSsiTxGdmaDeInit(
    IN PHAL_SSI_ADAPTOR pHalSsiAdapter
)
{
    PSSI_DMA_CONFIG pDmaConfig;    
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL GdmaChnl;
    
    if (NULL == pHalSsiAdapter) {
        return;
    }

    pDmaConfig = &pHalSsiAdapter->DmaConfig;

    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pDmaConfig->pTxHalGdmaAdapter;
    GdmaChnl.GdmaIndx = pHalGdmaAdapter->GdmaIndex;
    GdmaChnl.GdmaChnl = pHalGdmaAdapter->ChNum;
    GdmaChnl.IrqNum = pDmaConfig->TxGdmaIrqHandle.IrqNum;
    HalGdmaChnlFree(&GdmaChnl);
}


HAL_Status
HalSsiRxGdmaInit(
    IN PHAL_SSI_OP pHalSsiOp,
    IN PHAL_SSI_ADAPTOR pHalSsiAdapter
)
{
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    PSSI_DMA_CONFIG pDmaConfig;    
    HAL_GDMA_CHNL *pgdma_chnl;
    PHAL_GDMA_OP pHalGdmaOp;
    
    if ((NULL == pHalSsiOp) || (NULL == pHalSsiAdapter)) {
        return HAL_ERR_PARA;
    }

    pDmaConfig = &pHalSsiAdapter->DmaConfig;
    // Load default setting
    HalSsiRxGdmaLoadDefRtl8195a((void*)pHalSsiAdapter);

    // Start to patch the default setting
    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pDmaConfig->pRxHalGdmaAdapter;
    if (HalGdmaChnlRegister(pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum) != HAL_OK) {
        // The default GDMA Channel is not available, try others
        if (pHalSsiAdapter->Index == 2) {
            // SSI2 RX Only can use GDMA 1
            pgdma_chnl = HalGdmaChnlAlloc((HAL_GDMA_CHNL*)Ssi2_RX_GDMA_Chnl_Option);
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
            pDmaConfig->RxGdmaIrqHandle.IrqNum = pgdma_chnl->IrqNum;            
        }
    }

    DBG_SSI_INFO("HalSsiRxGdmaInit: GdmaIndex=%d ChNum=%d \r\n", pHalGdmaAdapter->GdmaIndex, pHalGdmaAdapter->ChNum);
    pHalGdmaOp = (PHAL_GDMA_OP)pDmaConfig->pHalGdmaOp;
    pHalGdmaOp->HalGdmaOnOff((VOID*)(pHalGdmaAdapter));
    pHalGdmaOp->HalGdmaChIsrEnAndDis((VOID*)(pHalGdmaAdapter));

    HalSsiDmaInit(pHalSsiAdapter);
    InterruptRegister(&pDmaConfig->RxGdmaIrqHandle);
    InterruptEn(&pDmaConfig->RxGdmaIrqHandle);

    return HAL_OK;
}

VOID
HalSsiRxGdmaDeInit(
    IN PHAL_SSI_ADAPTOR pHalSsiAdapter
)
{
    PSSI_DMA_CONFIG pDmaConfig;    
    PHAL_GDMA_ADAPTER pHalGdmaAdapter;
    HAL_GDMA_CHNL GdmaChnl;
    
    if (NULL == pHalSsiAdapter) {
        return;
    }

    pDmaConfig = &pHalSsiAdapter->DmaConfig;

    pHalGdmaAdapter = (PHAL_GDMA_ADAPTER)pDmaConfig->pRxHalGdmaAdapter;
    GdmaChnl.GdmaIndx = pHalGdmaAdapter->GdmaIndex;
    GdmaChnl.GdmaChnl = pHalGdmaAdapter->ChNum;
    GdmaChnl.IrqNum = pDmaConfig->RxGdmaIrqHandle.IrqNum;
    HalGdmaChnlFree(&GdmaChnl);
}

#endif  // end of "#ifdef CONFIG_GDMA_EN"

HAL_Status
HalSsiInit(VOID *Data)
{
    HAL_Status ret;
    PHAL_SSI_ADAPTOR pHalSsiAdapter = (PHAL_SSI_ADAPTOR) Data;
#ifdef CONFIG_SOC_PS_MODULE
        REG_POWER_STATE SsiPwrState;
#endif
    ret = HalSsiInitRtl8195a_Patch(pHalSsiAdapter);
#ifdef CONFIG_SOC_PS_MODULE
        if(ret == HAL_OK) {
            // To register a new peripheral device power state
            SsiPwrState.FuncIdx = SPI0+ pHalSsiAdapter->Index;
            SsiPwrState.PwrState = ACT;
            RegPowerState(SsiPwrState);
        }
#endif

    return ret;   
}

HAL_Status
HalSsiDeInit(VOID *Data)
{
    HAL_Status ret;
    PHAL_SSI_ADAPTOR pHalSsiAdapter = (PHAL_SSI_ADAPTOR) Data;
#ifdef CONFIG_SOC_PS_MODULE
        REG_POWER_STATE SsiPwrState;
        u8 HardwareState;

        SsiPwrState.FuncIdx= SPI0+ pHalSsiAdapter->Index;
        QueryRegPwrState(SsiPwrState.FuncIdx, &(SsiPwrState.PwrState), &HardwareState);

        if(SsiPwrState.PwrState != HardwareState){
            DBG_SSI_ERR("Registered State is not the Hardware State");
            return HAL_ERR_UNKNOWN;
        }
        else{
            if((SsiPwrState.PwrState != INACT) && (SsiPwrState.PwrState !=ACT)){
                    DBG_SSI_INFO("Return to ACT state before DeInit");
                    HalSsiEnable(pHalSsiAdapter);
                    QueryRegPwrState(SsiPwrState.FuncIdx, &(SsiPwrState.PwrState), &HardwareState);
            }
            if(SsiPwrState.PwrState == ACT){
                SsiPwrState.PwrState = INACT;
                RegPowerState(SsiPwrState);
            }          
        }
#endif
    ret = HalSsiDeInitRtl8195a(pHalSsiAdapter);
    return ret;
}


HAL_Status
HalSsiEnable(VOID *Data)
{
    HAL_Status ret;
    PHAL_SSI_ADAPTOR pHalSsiAdapter = (PHAL_SSI_ADAPTOR) Data;
#ifdef CONFIG_SOC_PS_MODULE
        REG_POWER_STATE SsiPwrState;
#endif
    ret = HalSsiClockOnRtl8195a(pHalSsiAdapter);
#ifdef CONFIG_SOC_PS_MODULE
        if(ret == HAL_OK) {
            // To register a new peripheral device power state
            SsiPwrState.FuncIdx = SPI0+ pHalSsiAdapter->Index;
            SsiPwrState.PwrState = ACT;
            RegPowerState(SsiPwrState);
        }
#endif

    return ret;   
}

HAL_Status
HalSsiDisable(VOID *Data)
{
    HAL_Status ret;
    PHAL_SSI_ADAPTOR pHalSsiAdapter = (PHAL_SSI_ADAPTOR) Data;
#ifdef CONFIG_SOC_PS_MODULE
        REG_POWER_STATE SsiPwrState;
#endif
    ret = HalSsiClockOffRtl8195a(pHalSsiAdapter);
#ifdef CONFIG_SOC_PS_MODULE
        if(ret == HAL_OK) {
            // To register a new peripheral device power state
            SsiPwrState.FuncIdx = SPI0+ pHalSsiAdapter->Index;
            SsiPwrState.PwrState = SLPCG;
            RegPowerState(SsiPwrState);
        }
#endif

    return ret;   
}