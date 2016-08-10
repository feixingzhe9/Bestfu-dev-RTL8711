/*****************************************************************************
	文件： IIC.c
	说明： IIC模式通信驱动
	编译： Keil uVision4 V4.54.0.0
	版本： v1.0
	编写： Joey
	日期： 2013.12.16    
*****************************************************************************/
#include "IIC.h"


/*
static void Delay_nNOP(u16 n);
static void IIC_Start(void);
static void  IIC_Stop(void);
static void  IIC_ACK(u8 ack);
static u8 IIC_WaitAck(void);
static u8 IIC_SendB_Ack(u8 data);
static u8 IIC_RecvB_Ack(u8 ack);
*/


gpio_t gpio_sht_scl;
gpio_t gpio_sht_sda;

/*****************************************************************************
s  函数名:  void Delay_nNOP(u16 n)	
功能说明:  延时n个空操作
	参数:  n-空操作的个数
  返回值:  void
*****************************************************************************/
static void Delay_nNOP(u16 n)
{
  u16 i=0,j;
  for(i=0;i<n;i++)
	for(j=0;j<20;j++);
}


/*******************************************************************************
**函    数： IIC_InitPort
**功    能： IIC接口初始化
**参    数： void
**返    回： void
*******************************************************************************/
void SHT_IIC_InitPort(void)
{                        

    
      printf("\n IIC_InitPort");
    // Init IIC_1_SCL_PIN
      gpio_init(&gpio_sht_scl, SHT_SCL_PIN);
      gpio_dir(&gpio_sht_scl, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_sht_scl, PullNone);     // No pull
      
      gpio_write(&gpio_sht_scl, 1);


      // Init IIC_1_SDA_PIN
      gpio_init(&gpio_sht_sda, SHT_SDA_PIN);
      gpio_dir(&gpio_sht_sda, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_sht_sda, PullNone);     // No pull
    
      gpio_write(&gpio_sht_sda, 1);
}

/*****************************************************************************
  函数名:  void IIC_Init(void)
功能说明:  IIC的初始化
    参数:  void
  返回值:  void
*****************************************************************************/
void SHT_IIC_Init(void)
{
    printf("\n SHT_IIC_Init");
    
    SHT_IIC_InitPort();
    
    SHT_SDA_OUT();
    SHT_SCL_OUT();

    SHT_SDA1;
    SHT_SCL1;
}

/*****************************************************************************
  函数名:  void IIC_Start(void)  
功能说明:  IIC的开始
    参数:  void
  返回值:  void
*****************************************************************************/
static void SHT_IIC_Start(void)
{
    SHT_SDA1;         
    SHT_SCL1;         
    Delay_nNOP(5);
    SHT_SDA0;
    Delay_nNOP(10);
    SHT_SCL0;
}

/*****************************************************************************
  函数名:  void IIC_Stop(void)
功能说明:  IIC的停止
    参数:  void
  返回值:  void
*****************************************************************************/
static void SHT_IIC_Stop(void)
{
    SHT_SDA0;
    Delay_nNOP(5);
    SHT_SCL1;
    Delay_nNOP(10);
    SHT_SDA1;
    Delay_nNOP(10);
}

/*****************************************************************************
  函数名:  void IIC_ACK(u8 ack)
功能说明:  IIC主动回应
    参数:  ack-回应标志，0表示继续读取，1表示读取结束
  返回值:  void
*****************************************************************************/
static void SHT_IIC_ACK(u8 ack)
{
    SHT_SCL0;
    Delay_nNOP(10);
    if (ack) 
    {
        SHT_SDA1;
    }
    else 
    {
        SHT_SDA0;
    }
    Delay_nNOP(5);
    SHT_SCL1;
    Delay_nNOP(10);
    SHT_SCL0;
}

/*****************************************************************************
  函数名:  u8 IIC_WaitAck(void)
功能说明:  IIC等待从器件回应
    参数:  void
  返回值:  u8-回应数据，1表示有回应，0表示超时或无回应
*****************************************************************************/
static u8 SHT_IIC_WaitAck(void)
{
	u16 OutTime=0;
	SHT_SDA_IN();       //SDA设置为输入  
	
    Delay_nNOP(2);	   
	SHT_SCL1;
    Delay_nNOP(5); 
	while(SHT_Get_SDA())
	{
		OutTime++;
		if(OutTime > 500)
		{
			SHT_SDA_OUT();
			return 0;
		}
	}
	SHT_SCL0;
    Delay_nNOP(5); 
	SHT_SDA_OUT();
	Delay_nNOP(5);
	return 1;
}

/*****************************************************************************
  函数名:  u8 IIC_SendB_Ack(u8 data) 
功能说明:  IIC发送一个字节数据并且等待回应
    参数:  data-将要发送的字节
  返回值:  u8-回应数据，1表示有回应，0表示超时或无回应
*****************************************************************************/
static u8 SHT_IIC_SendB_Ack(u8 data)
{
	u8 i,flag;
	SHT_SCL0;
	Delay_nNOP(5);
	for (i = 0; i < 8; i++)
	{
		if(data&0x80)
		  SHT_SDA1;
		else
		  SHT_SDA0;
		data <<= 1;
		Delay_nNOP(2);
		SHT_SCL1;
		Delay_nNOP(5);
		SHT_SCL0;
		Delay_nNOP(2);
	}

	flag = SHT_IIC_WaitAck();
	Delay_nNOP(10);
	return (flag);
}

/*****************************************************************************
  函数名:  u8 IIC_RecvB_Ack(u8 ack)
功能说明:  IIC接收一个字节数据并且回应ack
    参数:  ack-回应信息，0表示继续读取，1-表示停止读取
  返回值:  u8-收到的数据
*****************************************************************************/
static u8 SHT_IIC_RecvB_Ack(u8 ack)
{
	u8 i,rcvdata=0;
	SHT_SDA_IN(); 	  //SDA设置为输入  
	for(i=0; i<8; i++)
	{
		SHT_SCL0;
		Delay_nNOP(3);
		SHT_SCL1;
		Delay_nNOP(3);
		rcvdata <<= 1;

		if(SHT_Get_SDA())
		{
			rcvdata++;
		}
		Delay_nNOP(3);
	}
	SHT_SCL0;
	Delay_nNOP(3);
	SHT_SDA_OUT();
	Delay_nNOP(3);
	SHT_IIC_ACK(ack);
	return (rcvdata);
}

/*****************************************************************************
  函数名:  u8 IICReadEx
功能说明:  把数据串写入到EEPROM中的指定位置
    参数:  sla-从器件地址
           suba-数据首地址
           data-读取到的数据存放于该指针
           len-长度，最多不能超过一页大小
  返回值:  1表示成功，0表示失败
*****************************************************************************/
u8 SHT_IICRead_data(u8 sla, u8 suba, u8 *data, u8 len)
{
	u8 i, Flag = 0;
	SHT_IIC_Start();
	if(SHT_IIC_SendB_Ack(sla))
	{	
		if(SHT_IIC_SendB_Ack(suba))
		{
			SHT_IIC_Start();
			if(SHT_IIC_SendB_Ack(sla | 0x01))
			{
				Delay_nNOP(50000);
				for(i = len; i > 1; i--)
				{
					*data++ = SHT_IIC_RecvB_Ack(0);
				}
				*data = SHT_IIC_RecvB_Ack(1);
				Flag = 1;
			}
		}
	}
	SHT_IIC_Stop();
	Delay_nNOP(20);
	return Flag;
}
/*****************************************************************************
  函数名:  u8 IICReadEx
功能说明:  把数据串写入到EEPROM中的指定位置
    参数:  sla-从器件地址
           suba-数据首地址
           data-读取到的数据存放于该指针
           len-长度，最多不能超过一页大小
  返回值:  1表示成功，0表示失败
*****************************************************************************/
u8 SHT_IICRead_data2(u8 sla, u8 suba, u8 *data, u8 len)
{
	u8 i, Flag = 0;
	u32 overtime;
	SHT_IIC_Start();
	if(SHT_IIC_SendB_Ack(sla))
	{	
		if(SHT_IIC_SendB_Ack(suba))
		{
			for (overtime = 0; overtime < 0xffff; overtime++)
			{
				Delay_nNOP(50);
				SHT_IIC_Start();
				if(SHT_IIC_SendB_Ack(sla | 0x01))
				{
					break;
				}
			}
			if (overtime < 0xffff) //未超时
			{
				for(i = len; i > 1; i--)
				{
					*data++ = SHT_IIC_RecvB_Ack(0);
				}
				*data = SHT_IIC_RecvB_Ack(1);
				Flag = 1;
			}
		}
	}
	SHT_IIC_Stop();
	Delay_nNOP(20);
	return Flag;
}
/*****************************************************************************
  函数名:  u8 IICWriteEx1s
功能说明:  把数据串写入到EEPROM中的指定位置
    参数:  sla-从器件地址
           suba-数据首地址
           data-要写入的数据存放于该指针
           len-长度，最多不能超过一页大小
  返回值:  1表示成功，0表示失败
*****************************************************************************/
u8 SHT_IICWrite_data(u8 sla, u8 suba, u8 *data, u8 len)
{
	u8 i;
	SHT_IIC_Start();
	if(SHT_IIC_SendB_Ack(sla))
	{
		if(SHT_IIC_SendB_Ack(suba))
		{
			for(i=0; i<len; i++)
			{
				if(!SHT_IIC_SendB_Ack(*data++))
				{
					return FALSE;
				}
			}
		}
    }
	SHT_IIC_Stop();
	Delay_nNOP(500);
	return TRUE;
}



