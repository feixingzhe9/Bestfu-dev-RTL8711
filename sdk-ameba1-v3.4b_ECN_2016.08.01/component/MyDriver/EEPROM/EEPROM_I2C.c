/***************************Copyright BestFu 2014-05-14*************************
文	件：	I2C.c
说	明：	I2C模式通信驱动
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.05.04  
修　改：	暂无
*******************************************************************************/
#include "EEPROM_I2C.h"
#include "gpio_api.h"   // mbed

//Private macro define
#define WaitTime	3//2		//等待时间 
#define NOP			5		//空闲时间

//Private function declaration
static void Delay_nNOP(u16 n);
static void EEPROM_I2C_Start(void);
static void  EEPROM_I2C_Stop(void);
static void  EEPROM_I2C_ACK(u8 ack);
static u8 EEPROM_I2C_WaitAck(void);
static u8 EEPROM_I2C_SendB_Ack(u8 data);
static u8 EEPROM_I2C_RecvB_Ack(u8 ack);

gpio_t gpio_eeprom_scl;
gpio_t gpio_eeprom_sda;

/*******************************************************************************
函 数 名：	Delay_nNOP
功能说明： 	延时n个空操作
参	  数： 	n-空操作的个数
返 回 值：	void
*******************************************************************************/
static void Delay_nNOP(u16 n)
{
	u16 i = 0;

	for ( ; n > 0; n--)
	{
		for(i = NOP; i > 0; i--)
		{}
	}
}


/*******************************************************************************
**函    数： IIC_InitPort
**功    能： IIC接口初始化
**参    数： void
**返    回： void
*******************************************************************************/
void EEPROM_IIC_InitPort(void)
{                        

    
      printf("\n IIC_InitPort");
    // Init IIC_1_SCL_PIN
      gpio_init(&gpio_eeprom_scl, EEPROM_SCL_PIN);
      gpio_dir(&gpio_eeprom_scl, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_eeprom_scl, PullNone);     // No pull
      
      gpio_write(&gpio_eeprom_scl, 1);


      // Init IIC_1_SDA_PIN
      gpio_init(&gpio_eeprom_sda, EEPROM_SDA_PIN);
      gpio_dir(&gpio_eeprom_sda, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_eeprom_sda, PullNone);     // No pull
    
      gpio_write(&gpio_eeprom_sda, 1);
}


/*******************************************************************************
函 数 名：	I2C_Init
功能说明： 	I2C的初始化
参	  数： 	void
返 回 值：	void
*******************************************************************************/
void EEPROM_I2C_Init(void)
{
    EEPROM_IIC_InitPort();

    EEPROM_SDA_OUT;
    EEPROM_SCL_OUT;
    
    EEPROM_SCL1;
    EEPROM_SDA1;
}

/*******************************************************************************
函 数 名：	I2C_Start
功能说明： 	I2C的开始
参	  数： 	void
返 回 值：	void
*******************************************************************************/
static void EEPROM_I2C_Start(void)
{
    EEPROM_SDA1;         
    EEPROM_SCL1;         
    Delay_nNOP(WaitTime);
    EEPROM_SDA0;
    Delay_nNOP(WaitTime);
    EEPROM_SCL0;
}

/*******************************************************************************
函 数 名：	I2C_Stop
功能说明： 	I2C的停止
参	  数： 	void
返 回 值：	void
*******************************************************************************/
static void EEPROM_I2C_Stop(void)
{
    EEPROM_SDA0;
    Delay_nNOP(WaitTime);
    EEPROM_SCL1;
    Delay_nNOP(WaitTime);
    EEPROM_SDA1;
    Delay_nNOP(WaitTime);
}

/*******************************************************************************
函 数 名：	I2C_ACK
功能说明： 	I2C主动回应
参	  数： 	ack-回应标志，0表示继续读取，1表示读取结束
返 回 值：	void
*******************************************************************************/
static void EEPROM_I2C_ACK(u8 ack)
{
    EEPROM_SCL0;
    Delay_nNOP(WaitTime);
    if (ack) 
    {
        EEPROM_SDA1;
    }
    else 
    {
        EEPROM_SDA0;
    }
    Delay_nNOP(WaitTime);
    EEPROM_SCL1;
    Delay_nNOP(WaitTime);
    EEPROM_SCL0;
}

/*******************************************************************************
函 数 名：	I2C_WaitAck
功能说明： 	I2C等待从器件回应
参	  数： 	void
返 回 值：	u8-回应数据，1表示有回应，0表示超时或无回应
*******************************************************************************/
static u8 EEPROM_I2C_WaitAck(void)
{
	u16 OutTime=0;
	
	EEPROM_SDA_IN;       //SDA设置为输入  
	
        Delay_nNOP(WaitTime);	   
	EEPROM_SCL1;
        Delay_nNOP(WaitTime); 
	while (EEPROM_Get_SDA)
	{
		if ((++OutTime) > 500)
		{
			EEPROM_SDA_OUT;
			return FALSE;
		}
	}
	EEPROM_SCL0;
        Delay_nNOP(WaitTime); 
	EEPROM_SDA_OUT;
	Delay_nNOP(WaitTime);
	
	return TRUE;
}

/*******************************************************************************
函 数 名：	I2C_SendB_Ack
功能说明： 	I2C发送一个字节数据并且等待回应
参	  数： 	data-将要发送的字节
返 回 值：	u8-回应数据，1表示有回应，0表示超时或无回应
*******************************************************************************/
static u8 EEPROM_I2C_SendB_Ack(u8 data)
{
	u8 i;
	
	EEPROM_SCL0;
	Delay_nNOP(WaitTime);
	for (i = 0; i < 8; i++)
	{
		if(data&0x80)
		  EEPROM_SDA1;
		else
		  EEPROM_SDA0;
		data <<= 1;
		Delay_nNOP(WaitTime);
		EEPROM_SCL1;
		Delay_nNOP(WaitTime);
		EEPROM_SCL0;
		Delay_nNOP(WaitTime);
	}

	return EEPROM_I2C_WaitAck();
}

/*******************************************************************************
函 数 名：	I2C_RecvB_Ack
功能说明： 	I2C接收一个字节数据并且回应ack
参	  数： 	ack-回应信息，0表示继续读取，1-表示停止读取
返 回 值：	u8-收到的数据
*******************************************************************************/
static u8 EEPROM_I2C_RecvB_Ack(u8 ack)
{
	u8 i,rcvdata=0;
	
	EEPROM_SDA_IN; 	  //SDA设置为输入  
	for (i = 0; i < 8; i++)
	{
		EEPROM_SCL0;
		Delay_nNOP(WaitTime);
		EEPROM_SCL1;
		Delay_nNOP(WaitTime);
		rcvdata <<= 1;

		if(EEPROM_Get_SDA)
		{
			rcvdata++;
		}
		Delay_nNOP(WaitTime);
	}
	EEPROM_SCL0;
	Delay_nNOP(WaitTime);
	EEPROM_SDA_OUT;
	Delay_nNOP(WaitTime);
	EEPROM_I2C_ACK(ack);
	
	return (rcvdata);
}

/*******************************************************************************
函 数 名：  IIC_GetState
功能说明：  IIC通信设备状态
参	  数：  add:	设备地址
			overtime:等待超时时间
  返回值:  TRUE(非忙)/FALSE(忙)
*******************************************************************************/
u8 EEPROM_IIC_GetState(u8 sla, u32 overTime)
{
	do 
	{	
		EEPROM_I2C_Start();
	}while((!EEPROM_I2C_SendB_Ack(sla)) && --overTime);
	
	EEPROM_I2C_Stop();
	
	return (overTime ?  TRUE : FALSE);
}

/*******************************************************************************
函 数 名：	IIC_Read
功能说明： 	IIC数据读取
参	  数： 	sla-从器件地址
            suba-数据首地址
            data-读取到的数据存放于该指针
            len-长度，最多不能超过一页大小
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 EEPROM_IIC_Read(u8 sla, u16 suba, u8 *data, u8 len)
{
	EEPROM_I2C_Start();
	if (EEPROM_I2C_SendB_Ack(sla))
	{	
		if (EEPROM_I2C_SendB_Ack((u8)(suba>>8)))
		{
			if (EEPROM_I2C_SendB_Ack((u8)(suba)))
			{
				EEPROM_I2C_Start();
				if (EEPROM_I2C_SendB_Ack(sla | 0x01))
				{
					for (; len > 1; len--)
					{
						*data++ = EEPROM_I2C_RecvB_Ack(0);
					}
					len--;
					*data = EEPROM_I2C_RecvB_Ack(1);
				}
			}
		}
	}
	EEPROM_I2C_Stop();
	
	return (len ? FALSE : TRUE);
}

/*******************************************************************************
函 数 名：	I2C_Write
功能说明： 	把一串数据写入到EEPROM中的指定位置
参	  数： 	sla-从器件地址
            suba-数据首地址
            data-要写入的数据存放于该指针
            len-长度，最多不能超过一页大小
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 EEPROM_IIC_Write(u8 sla, u16 suba, u8 *data, u8 len)
{
//	WP_UN;
	EEPROM_I2C_Start();
	if(EEPROM_I2C_SendB_Ack(sla))
	{
		if (EEPROM_I2C_SendB_Ack((u8)(suba>>8)))
		{
			if (EEPROM_I2C_SendB_Ack((u8)(suba)))
			{
				for (; len > 0; len--)
				{
					if (!EEPROM_I2C_SendB_Ack(*data++))
					{
						break;
					}
				}
			}
		}
    }
	EEPROM_I2C_Stop();
//	WP_EN;
	
	return (len ? FALSE : TRUE);
}

/*******************************************************************************
函 数 名：	I2C_WriteSame
功能说明： 	把一串相同的数据，写入到EEPROM中的指定位置
参	  数： 	sla-从器件地址
            suba-数据首地址
            data-要写入的数据
            len-长度，最多不能超过一页大小
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 EEPROM_IIC_WriteSame(u8 sla, u16 suba, u8 data, u8 len)
{
//	WP_UN;
	EEPROM_I2C_Start();
	if (EEPROM_I2C_SendB_Ack(sla))
	{
		if (EEPROM_I2C_SendB_Ack((u8)(suba>>8)))
		{
			if (EEPROM_I2C_SendB_Ack((u8)(suba)))
			{
				for	(; len > 0; len--)
				{
					if (!EEPROM_I2C_SendB_Ack(data))
					{
						break;
					}
				}
			}
		}
    }
	EEPROM_I2C_Stop();
//	WP_EN;
	
	return (len ? FALSE : TRUE);
}
/**************************Copyright BestFu 2014-05-14*************************/
