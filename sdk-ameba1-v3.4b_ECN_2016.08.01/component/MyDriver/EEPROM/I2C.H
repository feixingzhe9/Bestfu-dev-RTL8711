/***************************Copyright BestFu 2014-05-14*************************
文	件：	I2C.h
说	明：	I2C模式通信驱动
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.05.04  
修　改：	暂无
*******************************************************************************/
#ifndef _I2C_H
#define _I2C_H



     
#define SCL(f)                  STATUS##f(SCL_PORT, SCL_PIN)

#define SCL_OUT                 GPIOx_Cfg(SCL_PORT, SCL_PIN, OUT_PP_50M
#define SCL_IN                  GPIOx_Cfg(SCL_PORT, SCL_PIN, IN_UPDOWN); \
								GPIOx_Set(SCL_PORT, SCL_PIN)
#define SCL_READ                GPIOx_Get(SCL_PORT, SCL_PIN)

#define SDA(f)                  STATUS##f(SDA_PORT, SDA_PIN)

#define SDA_OUT                 GPIOx_Cfg(SDA_PORT, SDA_PIN, OUT_PP_50M);\
								GPIOx_Set(SDA_PORT, SDA_PIN)
#define SDA_IN                  GPIOx_Cfg(SDA_PORT, SDA_PIN, IN_UPDOWN); \
								GPIOx_Set(SDA_PORT, SDA_PIN)
#define SDA_READ                GPIOx_Get(SDA_PORT, SDA_PIN)

#if     (SDA_PORT == SCL_PORT)
	#define SCL_SDA(f)              STATUS##f(SDA_PORT, SDA_PIN|SCL_PIN)

	#define SCL_SDA_OUT             GPIOx_Cfg(SDA_PORT, SDA_PIN|SCL_PIN, OUT_PP_50M)
	#define SCL_SDA_IN              GPIOx_Cfg(SCL_PORT, SDA_PIN|SCL_PIN, IN_UPDOWN); \
									GPIOx_Set(SCL_PORT, SDA_PIN|SCL_PIN)
	#define SCL_SDA_READ            GPIOx_Get(SDA_PORT, SDA_PIN|SCL_PIN)
#else 
	#define SCL_SDA(f)              SCL(f);  SDA(f)

	#define SCL_SDA_OUT             SCL_OUT; SDA_OUT
	#define SCL_SDA_IN              SCL_IN;  SDA_IN
	#define SCL_SDA_READ            SCL_READ;SDA_READ
#endif

#ifdef	WP_PORT		//如果定义写保护端口
	#define WP_INIT					GPIOx_Cfg(WP_PORT, WP_PIN, OUT_PP_50M);\
									WP_EN
	#define WP_UN					GPIOx_Rst(WP_PORT, WP_PIN)
	#define WP_EN					GPIOx_Set(WP_PORT, WP_PIN)
#else
	#define WP_INIT	
	#define WP_UN
	#define WP_EN
#endif



void I2C_Init(void);
u8 IIC_Read(u8 sla, u16 suba, u8 *data, u8 len);
u8 IIC_Write(u8 sla, u16 suba, u8 *data, u8 len);
u8 IIC_WriteSame(u8 sla, u16 suba, u8 data, u8 len);
u8 IIC_GetState(u8 sla, u32 overTime);


#endif
/**************************Copyright BestFu 2014-05-14*************************/
