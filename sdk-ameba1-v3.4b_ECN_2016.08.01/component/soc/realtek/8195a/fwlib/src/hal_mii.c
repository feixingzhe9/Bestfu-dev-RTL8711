/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */


#include "hal_mii.h"

VOID
HalMiiOpInit(
    IN VOID *Data
    )
{
    PHAL_MII_OP pHalMiiOp = (PHAL_MII_OP) Data;

    pHalMiiOp->HalMiiGmacInit          = HalMiiGmacInitRtl8195a;
    pHalMiiOp->HalMiiInit              = HalMiiInitRtl8195a;
    pHalMiiOp->HalMiiGmacReset         = HalMiiGmacResetRtl8195a;
    pHalMiiOp->HalMiiGmacEnablePhyMode = HalMiiGmacEnablePhyModeRtl8195a;
    pHalMiiOp->HalMiiGmacXmit          = HalMiiGmacXmitRtl8195a;
    pHalMiiOp->HalMiiGmacCleanTxRing   = HalMiiGmacCleanTxRingRtl8195a;
    pHalMiiOp->HalMiiGmacFillTxInfo    = HalMiiGmacFillTxInfoRtl8195a;
    pHalMiiOp->HalMiiGmacFillRxInfo    = HalMiiGmacFillRxInfoRtl8195a;
    pHalMiiOp->HalMiiGmacTx            = HalMiiGmacTxRtl8195a;
    pHalMiiOp->HalMiiGmacRx            = HalMiiGmacRxRtl8195a;
    pHalMiiOp->HalMiiGmacSetDefaultEthIoCmd   = HalMiiGmacSetDefaultEthIoCmdRtl8195a;
    pHalMiiOp->HalMiiGmacInitIrq       = HalMiiGmacInitIrqRtl8195a;
    pHalMiiOp->HalMiiGmacGetInterruptStatus   = HalMiiGmacGetInterruptStatusRtl8195a;
    pHalMiiOp->HalMiiGmacClearInterruptStatus = HalMiiGmacClearInterruptStatusRtl8195a;
#if 0
    pHalMiiOp-> = Rtl8195a;
    pHalMiiOp-> = Rtl8195a;
    pHalMiiOp-> = Rtl8195a;
    pHalMiiOp-> = Rtl8195a;
    pHalMiiOp-> = Rtl8195a;
#endif
}

