/*******************************************************************
文	件：SHT20.c
说	明：SHT20温湿度传感器驱动文档
编	译：Keil uVision4 V4.54.0.0
版	本：v1.0
编	写：Unarty
日	期：2014-08-09
********************************************************************/
#include "SHT20.h"
#include "IIC.H"
#include "EEPROM.h"
//#include "delay.h"

#include "device.h"
#include "PinNames.h"
#include "basic_types.h"
#include "diag.h" 
#include "osdep_api.h"
#include "i2c_api.h"
#include "pinmap.h"

#include "Kalman_Filter.h"


extern void delay_us(u32 us);

struct temp_humidity_set_t gtemp_humidity_set;
struct temp_humidity_set_t *ptemp_humidity_set;
/*******************************************************************************
º¯ Êý Ãû£º	void delay_ms(u16 nms)
¹¦ÄÜËµÃ÷£º	ÑÓÊ± n ms
²Î	  Êý£º	ms£º	ÑÓÊ±Ê±³¤µ¥Î»Îªms
·µ »Ø Öµ£º	ÎÞ
×¢	  Òâ£º	ÏµÍ³Ê±ÖÓÊÇÔÚ72MÏÂ
*******************************************************************************/								    
void delay_ms(u32 ms)
{	
	while (ms--)
	{
		delay_us(1000);
	}    
}
/*******************************************************************
函 数 名：	SHT20_Init
功能说明： 	SHT20初始化
参	  数： 	无
返 回 值：	无
注    意:   无
*******************************************************************/
void SHT20_Init(void)
{
	u8 data;
	
	SHT_IIC_Init();
	delay_ms(20);
	SHT_IICRead_data(SHT20_ADDR, SHT20_RESET, &data, 0); //SHT20复位。
	delay_ms(80);
	SHT_IICRead_data(SHT20_ADDR, SHT20_READ, &data, 1);
	if (data != 0x3A)
	{
		data = 0x3A; //工作模式，温度：14位，湿度：12位，不加热
		SHT_IICWrite_data(SHT20_ADDR, SHT20_READ, &data, 1);
	}
}

/*******************************************************************
函 数 名：	SHT20_Read
功能说明： 	读取温湿度传感器值
参	  数： 	mode:	T(温度)/H(湿度)
返 回 值：	读取的实际寄存器值
*******************************************************************/
u16 SHT20_Read(SHT20_t mode)
{
	u8 data[3];
	
	if (SHT_IICRead_data2(SHT20_ADDR, mode, data, 3))
	{
		return ((data[0]<<8)|data[1]);
	}
	return FALSE;
}
		


/*******************************************************************************
**函    数： TempSampVal()
**功    能： 从传感器获取温度 
**参    数： void
**返    回： 温度值(数值扩大10倍) 精度0.1℃ 
*******************************************************************************/
int TempSampVal(void)
{
	int T = (((u32)SHT20_Read(TEMP)*17572)>>16)/10 - 468; 
	return T;         //温度补偿2.5度
}

/*******************************************************************************
**函    数： HumiSampVal()
**功    能： 从传感器获取湿度
**参    数： void
**返    回： 湿度值  精度1% RH
*******************************************************************************/
u8 HumiSampVal(void)
{ 
	
	u8 H  = (((u32)SHT20_Read(HUMI) & (~0X0003))*125>>16)-6;
 	return H;         //湿度补偿6%  
}


#define FILTER_LENGTH   (5)
struct sht20_t sht20;
extern xSemaphoreHandle sht20_handle;
void get_temp_humidity_task(void *param)
{
    int new_temp = 0;
    int last_temp = 0;
    u8 new_humidity = 0;
    u8 last_humidity = 0;
    u8 flag = FALSE;
    
    Kalman_State_t kalman_temp;
    Kalman_State_t kalman_humidity;
    
    u8 cnt = 0;;
    int temp_data[FILTER_LENGTH] = {0};
    u8 humidity_data[FILTER_LENGTH] = {0};
    
    SHT20_Init();
    vTaskDelay(100);
    new_temp = TempSampVal();
    new_humidity = HumiSampVal();
    
    kalman_init(&kalman_temp, new_temp, 0.78);
    kalman_init(&kalman_humidity, new_humidity, 0.78);
    
    for(;;)
    {
        last_temp = new_temp;
        last_humidity = new_humidity;
        
        vTaskDelay(200);
        new_temp = TempSampVal();
        new_humidity = HumiSampVal();
        
        xSemaphoreTake(sht20_handle,portMAX_DELAY);
        {
            if(abs(last_temp - new_temp) < 20)
            {
                sht20.temp = (int)kalman_filter(&kalman_temp, new_temp);
            }
            
            if(abs(last_humidity - new_humidity) < 10)
            {
                sht20.humidity = (int)kalman_filter(&kalman_humidity, new_humidity);
            }
         
        }
        xSemaphoreGive(sht20_handle);
        
    }
    
#if 0
    while(1)
    {
        last_temp = new_temp;
        vTaskDelay(200);
        new_temp = TempSampVal();
        if(abs(new_temp - last_temp) < 10)
        { 
            temp_data[cnt] = new_temp;
            if(cnt == FILTER_LENGTH -1)
            {
                flag = TRUE;
            }
            cnt = (cnt + 1) % FILTER_LENGTH;
            if(flag == TRUE)
            {
                u8 i;
                int tmp = 0;
                for(i = 0; i < FILTER_LENGTH; i++)
                {
                    tmp += temp_data[i];
                }
                sht20.temp = tmp/FILTER_LENGTH;
                
            }
            
        }
    }
#endif
    
    

}

void example_shtc20_test(void *param)
{
	
	int temperature = 0;
	int humidity = 0;
        
        SHT20_Init();
               
	vTaskDelay(100);
	
	while(1){
		
                temperature = TempSampVal();
                humidity = HumiSampVal();
	
		rtl_printf("temp=%d, humidity=%d \n", temperature, humidity);              
		vTaskDelay(1000);
	}
}



/*******************************************************************************
**函    数： temp_humidity_set
**功    能： 设置温湿度阈值
**参    数： struct temp_humidity_set_t *temp_humidity_set：温湿度阈值结构体
**返    回： 无
*******************************************************************************/
u8 temp_humidity_set(struct temp_humidity_set_t *temp_humidity_set)
{
    memcpy(ptemp_humidity_set,temp_humidity_set,sizeof(struct temp_humidity_set_t));
    return WriteDataToEEPROM(TEMP_HUMIDITY_SET_START_ADDR,TEMP_HUMIDITY_SET_SPACE,ptemp_humidity_set);
}


/*******************************************************************************
**函    数： temp_humidity_set
**功    能： 设置温湿度阈值
**参    数： struct temp_humidity_set_t *temp_humidity_set：温湿度阈值结构体
**返    回： 无
*******************************************************************************/
u8 temp_humidity_set_init(void)
{    
    ptemp_humidity_set->humidity_max = HUMIDITY_MAX_DEFAULT_VALUE;
    ptemp_humidity_set->humidity_min = HUMIDITY_MIN_DEFAULT_VALUE;
    ptemp_humidity_set->temp_max = TEMP_MAX_DEFALUT_VALUE;
    ptemp_humidity_set->temp_min = TEMP_MIN_DEFAULT_VALUE;
    
    return WriteDataToEEPROM(TEMP_HUMIDITY_SET_START_ADDR,TEMP_HUMIDITY_SET_SPACE,ptemp_humidity_set);
}
