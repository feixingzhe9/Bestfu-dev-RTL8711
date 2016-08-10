/***************************Copyright BestFu ***********************************
**  文    件：  IIC.c
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
#include "myIIC.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio_api.h"   


#include "PinNames.h"
#include "basic_types.h"
#include "diag.h"
#include <osdep_api.h>

#include "i2c_api.h"
#include "pinmap.h"
#include "ex_api.h"




gpio_t gpio_iic_1_scl;
gpio_t gpio_iic_1_sda;

void delay_us(u32 us)
{
    u8 i = 0;
    while(us--){
        i = 50;//16;
        while(i--);
    }
        
}


                                                                  
/*******************************************************************************
**函    数： IIC_delay
**功    能： IIC时钟延时  -- 决定IIC传输速度
**参    数： void
**返    回： void
**说    明： 1bit cost double delay
**           if delay is 5us , IIC Speed is 100Kbps
*******************************************************************************/
static void IIC_delay(void) 
{
    delay_us(10);    
}




#if 1
/*******************************************************************************
**函    数： IIC_InitPort
**功    能： IIC接口初始化
**参    数： void
**返    回： void
*******************************************************************************/
void IIC_InitPort(void)
{                        

    
    printf("\n BL5372 IIC Port Init");
    // Init IIC_1_SCL_PIN
      gpio_init(&gpio_iic_1_scl, IIC_1_SCL_PIN);
      gpio_dir(&gpio_iic_1_scl, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_iic_1_scl, PullNone);     // No pull
      
      gpio_write(&gpio_iic_1_scl, 1);


      // Init IIC_1_SDA_PIN
      gpio_init(&gpio_iic_1_sda, IIC_1_SDA_PIN);
      gpio_dir(&gpio_iic_1_sda, PIN_OUTPUT);    // Direction: Output
      gpio_mode(&gpio_iic_1_sda, PullNone);     // No pull
    
      gpio_write(&gpio_iic_1_sda, 1);
      
      SDA_OUT();
      SCL_OUT();
      SCL1;
      SDA1;
}

/*******************************************************************************
**函    数： IIC_Start
**功    能： IIC开始信号
**说    明： When SCL is high , SDA change form high to low.
*******************************************************************************/
void IIC_Start(void)
{
    SDA1;
    SCL1;
    IIC_delay();
    SDA0;
    IIC_delay();
    SCL0;
    IIC_delay();
}

/*******************************************************************************
**函    数： IIC_Stop
**功    能： IIC停止信号
**说    明： When SCL is high , SDA change form low to high.
*******************************************************************************/
void IIC_Stop(void)
{
   SDA0;
   IIC_delay();
   SCL1;
   IIC_delay();
   SDA1;
   IIC_delay();
}

/*******************************************************************************
**函    数： IIC_WaitAck
**功    能： IIC等待ACK信号
**参    数： void
**返    回： 0：正常       1：等待超时
**说    明： 该函数加入等待超时处理，防止死等
*******************************************************************************/
u8 IIC_WaitAck(void)
{
    u16 undead = 0;//10000;

    SDA_IN();       //输入
    IIC_delay();  
    SCL1; 
    IIC_delay();
    
    while(Get_SDA())
    {   
        if(++undead>1000)
        {           
            SDA_OUT();                  
            return 0;
        } 
    }
    SCL0; 
    IIC_delay(); 
    SDA_OUT();
    IIC_delay(); 
    return 1;  
    
}

/*******************************************************************************
**函    数： IIC_ACK
**功    能： IIC发送ACK信号
*******************************************************************************/
void IIC_ACK(void)                       
{
    SDA0;         
    SCL1;   
    IIC_delay();
    SCL0;
    SDA0;
    IIC_delay();
}

/*******************************************************************************
**函    数： IIC_NACK
**功    能： IIC发送NACK信号
*******************************************************************************/
void IIC_NACK(void)                       
{
    SDA1;          
    SCL1;   
    IIC_delay();
    SCL0;
    SDA0;
    IIC_delay();
}

/*******************************************************************************
**函    数： IIC_SendByte
**功    能： IIC发送一个字节
**参    数： dat       --数据
**返    回： void
**说    明： 高位先发
*******************************************************************************/
void IIC_SendByte(u8 dat)
{
    u8 i;
    
//    SCL0;//xjx.add
        
    for(i=0;i<8;i++)
    {   
        if(dat&0x80)
        {
            SDA1;
        }   
        else  
        {
            SDA0;
        }
            
        dat<<=1;
        SCL1;
        IIC_delay();
        SCL0;
        IIC_delay();    
    }
    
//    SDA1;//xjx.add
}

/*******************************************************************************
**函    数： IIC_GetByte
**功    能： IIC读取一个字节
**参    数： void
**返    回： dat       --数据
**说    明： 高位先收
*******************************************************************************/
u8 IIC_GetByte(void)
{   
    u8 i,dat=0;
    SDA_IN();
    
//    SCL0;//xjx.add
    
    for(i=0;i<8;i++)
    {   
       SCL1;
       IIC_delay();
       dat=dat<<1;
       if(Get_SDA()) 
       {
          dat |= 0x01;
       }
           
       SCL0;
       IIC_delay();
    }   
    SDA_OUT();   
    SDA1;   
    return dat;     
}

/*******************************************************************************
**函    数： IIC_WriteByte
**功    能： IIC往指定地址写一字节数据
**参    数： Addr    --地址
**           data    --发送的数据
**返    回： void
*******************************************************************************/
void IIC_WriteByte(u8 Addr,u8 data)   
{
    IIC_Start();
    IIC_SendByte(DEVICE_ADDR);  //WriteDeviceAddr
    IIC_WaitAck();
    IIC_SendByte(Addr);
    IIC_WaitAck();
    IIC_SendByte(data);   
    IIC_WaitAck();  
    IIC_Stop(); 
}

/*******************************************************************************
**函    数： IIC_ReadByte
**功    能： IIC读指定地址字节数据
**参    数： Addr    --地址    
**返    回： 接收的数据
*******************************************************************************/
u8 IIC_ReadByte(u8 Addr)         
{ 
    u8 data;
   
    IIC_Start();
    IIC_SendByte(DEVICE_ADDR);
    IIC_WaitAck();
    IIC_SendByte(Addr); 
    IIC_WaitAck();  
    IIC_Start();
    IIC_SendByte(DEVICE_ADDR+1);    //ReadDeviceAddr
    IIC_WaitAck();
    data=IIC_GetByte();
    IIC_NACK();
    IIC_Stop();
 
    return data;  
}

/*******************************************************************************
**函    数： IIC_WriteDataN
**功    能： IIC往指定地址连续写N个数据
**参    数： Addr    --地址   
**           pData   --发送数据指针
**           len     --发送数据长度
**返    回： void
*******************************************************************************/
void IIC_WriteDataN(u8 Addr , u8 *pData , u8 len)
{
    u8 i,*pTemp= pData;
    IIC_Start();
    IIC_SendByte(DEVICE_ADDR);
    IIC_WaitAck();
    IIC_SendByte(Addr);
    IIC_WaitAck();
    for(i=0;i<len;i++)  
    {
        IIC_SendByte(*pTemp);
        IIC_WaitAck();
        pTemp++ ;
    }
    IIC_Stop(); 
}
                                                                
/*******************************************************************************
**函    数： IIC_ReadDataN
**功    能： IIC在指定地址连续读N个数据
**参    数： Addr    --地址   
**           pData   --接收数据指针
**           len     --接收数据长度
**返    回： void
*******************************************************************************/
void IIC_ReadDataN(u8 Addr,u8 *pData,u8 len)         
{      
    u8 i,*pTemp= pData;
    IIC_Start();
    IIC_SendByte(DEVICE_ADDR);
    IIC_WaitAck();  
    IIC_SendByte(Addr);
    IIC_WaitAck();  
    IIC_Start();
    IIC_SendByte(DEVICE_ADDR+1);
    IIC_WaitAck();
    for(i=0;i<len-1;i++)  
    {    
        *(pTemp++) = IIC_GetByte();
        IIC_ACK();  
    }
    *(pTemp++) = IIC_GetByte();
    IIC_NACK();
    IIC_Stop();
}
#endif






u8 TimeGet(u8 *year,u8 *month, u8 *day,u8 *week, u8 *hour, u8 *min, u8 *sec);
void TimeSet();
    

   

/*******************************************************************************
函 数 名:  	TimeSet
功能说明:  	设置时间
参    数:  		
返 回 值:  	无
注    意：	只做测试用
*******************************************************************************/
void TimeSet()
{
//    u8 buf[7] = {0x16,0x06,0x16,0x04,0x08,0x00,0x00};//年月日周时分秒
    u8 buf2[7] = {0x00,0x00,0x08,0x05,0x17,0x06,0x16};//秒分时周日月年
    
    IIC_WriteDataN(0<<4,buf2,7);

}

    
    
    
/*******************************************************************************
函 数 名:  	TimeGet
功能说明:  	读取时间
参    数:  		
返 回 值:  	无
注    意：	只做测试用
*******************************************************************************/
u8 TimeGet(u8 *year,u8 *month, u8 *day,u8 *week, u8 *hour, u8 *min, u8 *sec)
{
//    u8 buf[6] = {0};
//    IIC_Read(u8 sla, u16 suba, u8 *data, u8 len);

    u8 secTmp = 0;
    u8 minTmp = 0;
    u8 hourTmp = 0;
    u8 weekTmp = 0;
    u8 dayTmp = 0;
    u8 montTmp = 0;
    u8 yearTmp = 0;
    
    u8 data[7] = {0};
    
    IIC_ReadDataN(0<<4,data,1);

  
//    secTmp = IIC_ReadByte(0<<4);
//    minTmp = IIC_ReadByte(1<<4);
//    hourTmp = IIC_ReadByte(2<<4);
//    weekTmp = IIC_ReadByte(3<<4);
//    dayTmp = IIC_ReadByte(4<<4);
//    montTmp = IIC_ReadByte(5<<4);
//    yearTmp = IIC_ReadByte(6<<4);
    
    *sec = data[0];// & 0x0f + (data[0] & 0xf0) * 10;
    *min = data[1];// & 0x0f + (data[1] & 0xf0) * 10;
    *hour = data[2];// & 0x0f + (data[2] & 0xf0) * 10;
    *week = data[3];// & 0x0f + (data[3] & 0xf0) * 10;
    *day = data[4];// & 0x0f + (data[4] & 0xf0) * 10;
   // *month = data[5];// & 0x0f + (data[5] & 0xf0) * 10;
    //*year = data[6];// & 0x0f + (data[6] & 0xf0) * 10;

    printf("sec is %d\n ",*sec);
   // printf("min is %d\n ",*min);
  //  printf("hour is %d\n ",*hour);
  //  printf("week is %d\n ",*week);
  //  printf("day is %d\n ",*day);
  //  printf("month is %d\n ",*month);
  //  printf("year is %d\n ",*year);
    return TRUE;
}


void TimeDelayTest(void *param)
{
    u8 year, month, day, week,hour, min, sec;
    u8 FH_Value = 0;
    
    u8 *temp = NULL;
    u8 *humi = NULL;
    
    IIC_InitPort();
  
    TimeSet();
    vTaskDelay(1000);
    while(1)
    {
             
        TimeGet(&year, &month, &day, &week,&hour, &min, &sec);
        
        vTaskDelay(2000);

    }                    
}



/********************************* END FILE ***********************************/
