/* mbed Microcontroller Library
 *******************************************************************************
 * Copyright (c) 2014, Realtek Semiconductor Corp.
 * All rights reserved.
 *
 * This module is a confidential and proprietary property of RealTek and
 * possession or use of this module requires written permission of RealTek.
 *******************************************************************************
 */
#include "objects.h"
#include "port_api.h"
#include "pinmap.h"
#include "gpio_api.h"
#include "PinNames.h"
//#include "mbed_error.h"

#if CONFIG_GPIO_EN

#if DEVICE_PORTIN || DEVICE_PORTOUT

#define GPIO_PORT_NUM       2
#define GPIO_PORT_WIDTH     8
#define GPIO_PORT_WIDTH_MAX     16

const u8 Default_Port_PinDef[GPIO_PORT_NUM][GPIO_PORT_WIDTH+1] = {
    // Port 0
    {PA_6, PA_7, PA_5, PD_4,
     PD_5, PA_4, PA_3, PA_2,
     0xFF},

    // Port 1
    {PB_4, PB_5, PC_0, PC_2,
     PC_3, PC_1, PB_3, PB_2,
     0xFF}
};

extern  VOID HAL_GPIO_Init(HAL_GPIO_PIN  *GPIO_Pin);
extern u32 HAL_GPIO_GetPinName(u32 chip_pin);

// high nibble = port number (0=A, 1=B, 2=C, 3=D, 4=E, 5=F, ...)
// low nibble  = pin number
PinName port_pin(PortName port, int pin_n) {
    return (PinName)(pin_n + (port << 4));
}

void port_init(port_t *obj, PortName port, int mask, PinDirection dir) 
{
    u32 i;

    if (port >= GPIO_PORT_NUM) {
        DBG_GPIO_ERR("port_init: Invalid port num(%d), max port num is %d\r\n", \
            port, (GPIO_PORT_NUM-1));
    }
    
    // Fill PORT object structure for future use
    obj->port      = port;
    obj->mask      = mask;
    obj->direction = dir;

//    if (obj->pin_def == NULL) {
        obj->pin_def = (uint8_t*)&Default_Port_PinDef[port][0];
//    }

    i=0;
    while (obj->pin_def[i] != 0xff) {        
        i++;
        if (i == GPIO_PORT_WIDTH_MAX) {
            break;
        }
    }

    obj->mask &= ((1<<i) - 1);
    port_dir(obj, dir);
}

void port_dir(port_t *obj, PinDirection dir) 
{
    uint32_t i;
    HAL_GPIO_PIN GPIO_Pin;
    
    obj->direction = dir;
    for (i = 0; i < GPIO_PORT_WIDTH_MAX; i++) { // Process all pins
        if (obj->mask & (1 << i)) { // If the pin is used

            GPIO_Pin.pin_name = HAL_GPIO_GetPinName(obj->pin_def[i]); // get the IP pin name
            
            if (dir == PIN_OUTPUT) {
                GPIO_Pin.pin_mode = DOUT_PUSH_PULL;
            } else { // PIN_INPUT
                GPIO_Pin.pin_mode = DIN_PULL_NONE;
            }
            HAL_GPIO_Init(&GPIO_Pin);
        }
    }
}

void port_mode(port_t *obj, PinMode mode) 
{
    uint32_t i;
    
    for (i = 0; i < GPIO_PORT_WIDTH_MAX; i++) { // Process all pins
        if (obj->mask & (1 << i)) { // If the pin is used
            pin_mode(obj->pin_def[i], mode);
        }
    }
}

void port_write(port_t *obj, int value) 
{
    uint32_t i;
    HAL_GPIO_PIN GPIO_Pin;
    
    for (i = 0; i < GPIO_PORT_WIDTH_MAX; i++) { // Process all pins
        if (obj->mask & (1 << i)) { // If the pin is used
            GPIO_Pin.pin_name = HAL_GPIO_GetPinName(obj->pin_def[i]); // get the IP pin name
            GPIO_Pin.pin_mode = DOUT_PUSH_PULL;
            HAL_GPIO_WritePin(&GPIO_Pin, ((value>>i) & 0x01));
        }
    }
}

int port_read(port_t *obj) 
{
    int value=0;
    u32 i;
    HAL_GPIO_PIN_MODE pin_mode;
    HAL_GPIO_PIN GPIO_Pin;

    if (obj->direction == PIN_OUTPUT) {
        pin_mode = DOUT_PUSH_PULL;
    } else { // PIN_INPUT
        pin_mode = DIN_PULL_NONE;
    }

    for (i = 0; i < GPIO_PORT_WIDTH_MAX; i++) { // Process all pins
        if (obj->mask & (1 << i)) { // If the pin is used
            GPIO_Pin.pin_name = HAL_GPIO_GetPinName(obj->pin_def[i]); // get the IP pin name
            GPIO_Pin.pin_mode = pin_mode;
            if (HAL_GPIO_ReadPin(&GPIO_Pin)) {
                value |= (1<<i);
            }
        }
    }

    return value;
}

#endif
#endif
