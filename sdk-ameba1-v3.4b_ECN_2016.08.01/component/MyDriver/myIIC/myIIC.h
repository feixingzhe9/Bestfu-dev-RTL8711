/******************************************************************************
**  文    件：  IIC.h
**  功    能：  <<驱动层>> 模拟IIC驱动
**  编    译：  Keil uVision5 V5.10
**  版    本：  V2.0
**  编    写：  Seven
**  创建日期：  2014.03.21
**  修改日期：  2014.08.20
**  说    明：  模拟IIC协议
**  V2.0
    >> 所有函数重新优化，删除额外操作
    >> 优化移植接口
*******************************************************************************/
#ifndef _IIC_H_
#define _IIC_H_ 

#include "device.h"
#include "gpio_api.h"   // mbed
 

   

extern gpio_t gpio_iic_1_scl;
extern gpio_t gpio_iic_1_sda;

//#define DEVICE_ADDR     0xD0    // MPU6050 PIN AD0 =0   : 1101 000x
#define DEVICE_ADDR     0x64      //BL5372  ADDR               //0x46    // BH1750  ADDR =0      : 0100 011x

/************************IIC Inteface STM32L151C8T6 I/O***********************/

/*******************************************************************************/

#define IIC_1_SCL_PIN       PB_2         
#define IIC_1_SDA_PIN       PB_3//PC_4//




#define SCL1        gpio_write(&gpio_iic_1_scl, 1)

#define SCL0        gpio_write(&gpio_iic_1_scl, 0)

#define SDA1        {gpio_dir(&gpio_iic_1_sda, PIN_OUTPUT);gpio_write(&gpio_iic_1_sda, 1);}

#define SDA0        {gpio_dir(&gpio_iic_1_sda, PIN_OUTPUT);gpio_write(&gpio_iic_1_sda, 0);}

#define SDA_IN()    {gpio_dir(&gpio_iic_1_sda, PIN_INPUT);gpio_mode(&gpio_iic_1_sda, PullUp);}
                    
                    
#define SDA_OUT()   {gpio_dir(&gpio_iic_1_sda, PIN_OUTPUT);gpio_write(&gpio_iic_1_sda, 1);}
#define SCL_OUT()   {gpio_dir(&gpio_iic_1_scl, PIN_OUTPUT);gpio_write(&gpio_iic_1_scl, 1);}
                    


#define Get_SDA()   gpio_read(&gpio_iic_1_sda)
//======================================================

/*接口函数*/
extern void  IIC_InitPort(void);
extern void  IIC_Start(void);
extern void  IIC_Stop(void);

extern void  IIC_ACK(void); 
extern void  IIC_NACK(void); 
extern u8    IIC_WaitAck(void);

extern void  IIC_SendByte(u8 dat);
extern u8    IIC_GetByte(void);

extern void  IIC_WriteByte(u8 Addr,u8 data);
extern u8    IIC_ReadByte( u8 Addr);

extern void  IIC_WriteDataN(u8 Addr,u8 *pData ,u8 len);
extern void  IIC_ReadDataN( u8 Addr,u8 *pData ,u8 len);



#endif
/********************************* END FILE ***********************************/
