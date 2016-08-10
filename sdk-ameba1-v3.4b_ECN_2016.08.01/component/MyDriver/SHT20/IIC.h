/*****************************************************************************
	文件： IIC.h
	说明： I2C通信驱动
	编译： Keil uVision4 V4.54.0.0
	版本： v1.0
	编写： Joey
	日期： 2013.12.16    
*****************************************************************************/
#ifndef _IIC_H
#define _IIC_H

//#include "device.h"
#include "gpio_api.h"   // mbed

#define SHT_DEVICE_ADDR     //0x64      //BL5372  ADDR               //0x46    // BH1750  ADDR =0      : 0100 011x

/************************IIC Inteface STM32L151C8T6 I/O***********************/

/*******************************************************************************/

#define SHT_SCL_PIN       PC_5         
#define SHT_SDA_PIN       PC_4//PC_4//




#define SHT_SCL1        gpio_write(&gpio_sht_scl, 1)

#define SHT_SCL0        gpio_write(&gpio_sht_scl, 0)

#define SHT_SDA1        gpio_write(&gpio_sht_sda, 1)

#define SHT_SDA0        gpio_write(&gpio_sht_sda, 0)

#define SHT_SDA_IN()    {gpio_dir(&gpio_sht_sda, PIN_INPUT);gpio_mode(&gpio_sht_sda, PullUp);}
                    
                    
#define SHT_SDA_OUT()   gpio_dir(&gpio_sht_sda, PIN_OUTPUT)
#define SHT_SCL_OUT()   gpio_dir(&gpio_sht_scl, PIN_OUTPUT)
                    


#define SHT_Get_SDA()   gpio_read(&gpio_sht_sda)

/*
#define SCL_PORT    GPIO_A      //SCL位于的端口号
#define SCL_PIN     pin_3        //SCL位于的引脚号pin_10//  
													    
#define SDA_PORT    GPIO_A      //SDA位于的端口号
#define SDA_PIN     pin_4       //SDA位于的引脚号 pin_11//

#define IIC_STATUS0(port, pin)  GPIOx_Rst(port, pin) 
#define IIC_STATUS1(port, pin)  GPIOx_Set(port, pin)

        
#define SCL(f)                  IIC_STATUS##f(SCL_PORT, SCL_PIN)

#define SCL_OUT                 GPIOx_Cfg(SCL_PORT, SCL_PIN, OUT_PP_50M)
#define SCL_IN                  GPIOx_Cfg(SCL_PORT, SCL_PIN, IN_UPDOWN)
#define SCL_READ                GPIOx_Get(SCL_PORT, SCL_PIN)

#define SDA(f)                  IIC_STATUS##f(SDA_PORT, SDA_PIN)

#define SDA_OUT                 GPIOx_Cfg(SDA_PORT, SDA_PIN, OUT_PP_50M)
#define SDA_IN                  GPIOx_Cfg(SDA_PORT, SDA_PIN, IN_UPDOWN)
#define SDA_READ                GPIOx_Get(SDA_PORT, SDA_PIN)

#if     (SDA_PORT == SCL_PORT)
#define SCL_SDA(f)              IIC_STATUS##f(SDA_PORT, SDA_PIN|SCL_PIN)

#define SCL_SDA_OUT             GPIOx_Cfg(SDA_PORT, SDA_PIN|SCL_PIN, OUT_PP_50M)
#define SCL_SDA_IN              GPIOx_Cfg(SCL_PORT, SDL_PIN|SCL_PIN, IN_UPDOWN)
#define SCL_SDA_READ            GPIOx_Get(SDA_PORT, SDA_PIN|SCL_PIN)
#else 
#define SCL_SDA(f)              SCL(f);  SDA(f)

#define SCL_SDA_OUT             SCL_OUT; SDA_OUT
#define SCL_SDA_IN              SCL_IN;  SDA_IN
#define SCL_SDA_READ            SCL_READ;SDA_READ
#endif

void IIC_Init(void);
u8 IICRead_data(u8 sla, u8 suba, u8 *data, u8 len);
u8 IICWrite_data(u8 sla, u8 suba, u8 *data, u8 len);
u8 IICRead_data2(u8 sla, u8 suba, u8 *data, u8 len);
*/
#endif


