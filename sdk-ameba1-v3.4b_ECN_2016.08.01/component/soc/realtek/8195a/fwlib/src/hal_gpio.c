/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include "rtl8195a.h"

#ifdef CONFIG_GPIO_EN

HAL_GPIO_DATA_SECTION HAL_GPIO_ADAPTER gHAL_Gpio_Adapter;
extern PHAL_GPIO_ADAPTER _pHAL_Gpio_Adapter;

extern VOID GPIO_PullCtrl_8195a(u32 chip_pin, u8 pull_type);

/**
  * @brief  To get the GPIO IP Pin name for the given chip pin name
  *
  * @param  chip_pin: The chip pin name.
  *
  * @retval The gotten GPIO IP pin name
  */
HAL_GPIO_TEXT_SECTION u32 
HAL_GPIO_GetPinName(
    u32 chip_pin
)
{
    return HAL_GPIO_GetIPPinName_8195a((u32)chip_pin);
}

/**
  * @brief  Set the GPIO pad Pull type
  *
  *  @param    pin: The pin for pull type control.
  *  @param    mode: the pull type for the pin.
  *  @return   None
  */
VOID 
HAL_GPIO_PullCtrl(
    u32 pin,
    u32 mode    
)
{
    u8 pull_type;

    DBG_GPIO_INFO("%s: pin=0x%x mode=%d\n ", __FUNCTION__, (u32)pin, (u32)mode);

    switch (mode) {
        case hal_PullNone:
            pull_type = DIN_PULL_NONE;
            break;
            
        case hal_PullDown:
            pull_type = DIN_PULL_LOW;
            break;
            
        case hal_PullUp:
            pull_type = DIN_PULL_HIGH;
            break;
            
        case hal_OpenDrain:
        default:
            pull_type = DIN_PULL_NONE;
            break;
    }

//    HAL_GPIO_PullCtrl_8195a (pin, pull_type);
    GPIO_PullCtrl_8195a (pin, pull_type);
}


/**
  * @brief  Initializes a GPIO Pin by the GPIO_Pin parameters.
  *
  * @param  GPIO_Pin: The data structer which contains the parameters for the GPIO Pin initialization.
  *
  * @retval HAL_Status
  */
HAL_GPIO_TEXT_SECTION VOID 
HAL_GPIO_Init(
    HAL_GPIO_PIN  *GPIO_Pin
)
{
    if (_pHAL_Gpio_Adapter == NULL) {
        _pHAL_Gpio_Adapter = &gHAL_Gpio_Adapter;
        DBG_GPIO_INFO("%s: Initial GPIO Adapter\n ", __FUNCTION__);
    }

    HAL_GPIO_Init_8195a(GPIO_Pin);
}

/**
  * @brief  Initializes a GPIO Pin as a interrupt signal
  *
  * @param  GPIO_Pin: The data structer which contains the parameters for the GPIO Pin initialization.
  *
  * @retval HAL_Status
  */
VOID 
HAL_GPIO_Irq_Init(
    HAL_GPIO_PIN  *GPIO_Pin
)
{
    if (_pHAL_Gpio_Adapter == NULL) {
        _pHAL_Gpio_Adapter = &gHAL_Gpio_Adapter;
        DBG_GPIO_INFO("%s: Initial GPIO Adapter\n ", __FUNCTION__);
    }

    if (_pHAL_Gpio_Adapter->IrqHandle.IrqFun == NULL) {
        _pHAL_Gpio_Adapter->IrqHandle.IrqFun = HAL_GPIO_MbedIrqHandler_8195a;
        _pHAL_Gpio_Adapter->IrqHandle.Priority = 8;
        HAL_GPIO_RegIrq_8195a(&_pHAL_Gpio_Adapter->IrqHandle);        
        InterruptEn(&_pHAL_Gpio_Adapter->IrqHandle);
        DBG_GPIO_INFO("%s: Initial GPIO IRQ Adapter\n ", __FUNCTION__);
    }

    DBG_GPIO_INFO("%s: GPIO(name=0x%x)(mode=%d)\n ", __FUNCTION__, GPIO_Pin->pin_name, 
        GPIO_Pin->pin_mode);
    HAL_GPIO_MaskIrq_8195a(GPIO_Pin);
    HAL_GPIO_Init_8195a(GPIO_Pin);
}

/**
  * @brief  UnInitial GPIO Adapter
  *
  *
  * @retval HAL_Status
  */
VOID
HAL_GPIO_IP_DeInit(
    VOID
)
{
    if (_pHAL_Gpio_Adapter != NULL) {
        InterruptDis(&_pHAL_Gpio_Adapter->IrqHandle);
        HAL_GPIO_UnRegIrq_8195a(&_pHAL_Gpio_Adapter->IrqHandle);                
        _pHAL_Gpio_Adapter = NULL;
    }
    
}

#endif // CONFIG_GPIO_EN
