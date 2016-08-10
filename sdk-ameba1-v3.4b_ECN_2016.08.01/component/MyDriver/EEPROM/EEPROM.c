/***************************Copyright BestFu 2014-05-14*************************
文	件：	Eeprom.c
说	明：	对Eeprom操作,包括全局参数与EEPROM映射关系，独立读写EEPROM入口源代码
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Joey
日	期：	2013.7.23   
修  改:     Unart(2014.04.12增加换页写入时的忙检测)
*******************************************************************************/
#include "EEPROM.h"
#include "EEPROM_I2C.h"

/*******************************************************************************
函 数 名：	ReadDataFromEEPROM
功能说明：	把对应EEPROM中数据读取到RAM中
参	  数：	addr-EEPROM中的实际地址
            len-长度
            data-读取到的数据存放于该指针
返 回 值：	1表示成功，0表示失败
*******************************************************************************/ 
u8 ReadDataFromEEPROM(u32 addr, u32 len, u8 *data)
{
	u32 count;
	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
			
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;	
		if (!EEPROM_IIC_Read(EEPROMSLAVEADDR, addr, data, count))
		{
			break;
		}
		
		addr += count;
		len  -= count;
		data += count;
		if (!EEPROM_IIC_GetState(EEPROMSLAVEADDR, 0xfffffFFF)) //??EEPROM???? 
		{
			break;
		}
	}

	return TRUE;
		
}

/*******************************************************************************
函 数 名：	WriteDataToEEPROM
功能说明：	把RAM中的数据写入到对应的EEPROM中
参	  数：	addr-EEPROM中的实际地址
            len-长度
            data-读取到的数据存放于该指针
返 回 值：	1表示成功，0表示失败
*******************************************************************************/ 
u8 WriteDataToEEPROM(u32 addr, u32 len, u8 *data)
{
	u32 count;

	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
		
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;		
		if(!EEPROM_IIC_Write(EEPROMSLAVEADDR, addr, data, count))
		{
			break;
		}

		addr += count;
		len  -= count;
		data += count;
		if (!EEPROM_IIC_GetState(EEPROMSLAVEADDR, 0xfffffFF)) //??EEPROM???? 
		{
			break;
		}
	}

	return TRUE;
	
}

/*******************************************************************************
函 数 名：	WriteDataToEEPROMEx
功能说明：	把相同数据写入到对应的EEPROM中
参	  数：	addr-EEPROM中的实际地址
            len-长度
            data-要写入的相同数据
返 回 值：	1表示成功，0表示失败
*******************************************************************************/ 
u8 WriteDataToEEPROMEx(u32 addr, u32 len, u8 data)
{
	u32 count;

	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
		
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;		
		if(!EEPROM_IIC_WriteSame(EEPROMSLAVEADDR, addr, data, count))
		{
			break;
		}

		addr += count;
		len  -= count;
		if (!EEPROM_IIC_GetState(EEPROMSLAVEADDR, 0xfffffFF)) //??EEPROM???? 
		{
			break;
		}
	}

	return TRUE;
	
}


#define EEPROM_TEST_ADDR     0x0000

#define EEPROM_TEST_DATA_NUM   6

void example_EEPROM_test(void *param)
{
	
	u8 data[EEPROM_TEST_DATA_NUM];
        u8 i;
        u8 buf[EEPROM_TEST_DATA_NUM] = {0};
        static u32 rcvCnt = 0;
        static u32 errorCnt = 0;
        
        EEPROM_I2C_Init();
        
       

	vTaskDelay(1000);
        
        for(i = 0; i < EEPROM_TEST_DATA_NUM; i++)
        {
            data[i] = i;
        }
	
	while(1){
            
                
  
                WriteDataToEEPROM(EEPROM_TEST_ADDR,EEPROM_TEST_DATA_NUM,data);
                vTaskDelay(2000);
                ReadDataFromEEPROM(EEPROM_TEST_ADDR, EEPROM_TEST_DATA_NUM, buf);
                
                
                for(i = 0; i < EEPROM_TEST_DATA_NUM; i++)
                {
                    rcvCnt++;
                    if(buf[i] == i)
                    {
                        printf("buf[%d] = %d \n",i, buf[i]);   
                    }
                    else
                    {
                        errorCnt++;
                        printf("buf[%d] is error,it should be %d  not %d, \n",i,i,buf[i]);
                    }
                       
                }
                printf("receive num is %d, error num is %d \n",rcvCnt,errorCnt);
                
                

		vTaskDelay(10000);
	}
}




/*******************************************************************************
函 数 名：	ReadDataFromRTC
功能说明：	把对应RTC中数据读取到RAM中
参	  数：	addr-RTC中的实际地址
            len-长度
            data-读取到的数据存放于该指针
返 回 值：	1表示成功，0表示失败
*******************************************************************************/ 
u8 ReadDataFromRTC(u32 addr, u32 len, u8 *data)
{
	u32 count;
	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
			
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;	
		if (!EEPROM_IIC_Read(RTCSLAVEADDR, addr, data, count))
		{
			break;
		}
		
		addr += count;
		len  -= count;
		data += count;
		if (!EEPROM_IIC_GetState(RTCSLAVEADDR, 0xfffffFFF)) //??EEPROM???? 
		{
			break;
		}
	}

	return TRUE;
		
}

/*******************************************************************************
函 数 名：	WriteDataToRTC
功能说明：	把RAM中的数据写入到对应的EEPROM中
参	  数：	addr-EEPROM中的实际地址
            len-长度
            data-读取到的数据存放于该指针
返 回 值：	1表示成功，0表示失败
*******************************************************************************/ 
u8 WriteDataToRTC(u32 addr, u32 len, u8 *data)
{
	u32 count;

	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
		
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;		
		if(!EEPROM_IIC_Write(RTCSLAVEADDR, addr, data, count))
		{
			break;
		}

		addr += count;
		len  -= count;
		data += count;
		if (!EEPROM_IIC_GetState(RTCSLAVEADDR, 0xfffffFF)) //??EEPROM???? 
		{
			break;
		}
	}

	return TRUE;
	
}

/*******************************************************************************
函 数 名：	WriteDataToEEPROMEx
功能说明：	把相同数据写入到对应的EEPROM中
参	  数：	addr-EEPROM中的实际地址
            len-长度
            data-要写入的相同数据
返 回 值：	1表示成功，0表示失败
*******************************************************************************/ 
u8 WriteDataToRTCEx(u32 addr, u32 len, u8 data)
{
	u32 count;

	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
		
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;		
		if(!EEPROM_IIC_WriteSame(RTCSLAVEADDR, addr, data, count))
		{
			break;
		}

		addr += count;
		len  -= count;
		if (!EEPROM_IIC_GetState(RTCSLAVEADDR, 0xfffffFF)) //??EEPROM???? 
		{
			break;
		}
	}

	return TRUE;
	
}


#define RTC_TEST_ADDR          0
void Example_RTC_test(void *param)
{
  
    u8 timeSet[7] = {0x00,0x00,0x08,0x05,0x17,0x06,0x16};//秒分时周日月年
    u8 timeGet[7] = {0};
    u8 i = 0;
    
    EEPROM_I2C_Init();
    vTaskDelay(500);
      
    WriteDataToRTC(RTC_TEST_ADDR,7,timeSet);
    vTaskDelay(1000);
    
    while(1)
    {
        ReadDataFromRTC(RTC_TEST_ADDR, 7, timeGet);
      
        for(i = 0; i < EEPROM_TEST_DATA_NUM; i++)
        {
            printf("timeGet[%d] = %d \n",i, timeGet[i]);   
       
        }
        
        vTaskDelay(2500);
    }
    
    
}
/**************************Copyright BestFu 2014-05-14*************************/
