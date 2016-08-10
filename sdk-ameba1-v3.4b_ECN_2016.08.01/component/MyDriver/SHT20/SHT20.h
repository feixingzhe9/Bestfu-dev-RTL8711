/*******************************************************************
文	件：SHT20.h
说	明：SHT20温湿度传感器驱动文档
编	译：Keil uVision4 V4.54.0.0
版	本：v1.0
编	写：Unarty
日	期：2014-08-09
********************************************************************/
#ifndef __SHT20_H
#define __SHT20_H

#include "freertos_service.h"

typedef enum
{
	TEMP = 0xF3,
	HUMI = 0xF5,
}SHT20_t;

#define SHT20_ADDR 	(0x80)
#define SHT20_READ	(0xE7)
#define SHT20_WRITE	(0xE6)
#define SHT20_RESET	(0xFE)

void SHT20_Init(void);
unsigned short SHT20_Read(SHT20_t mode);
unsigned short SHT20_Read2(SHT20_t mode);

#pragma pack(1)
struct temp_humidity_set_t
{
    u8 temp_min;        //温度下限
    u8 temp_max;        //温度上限
    u8 humidity_min;    //湿度下限
    u8 humidity_max;    //湿度上限
};

struct sht20_t
{
    int temp;
    u8 humidity;
};


#pragma pack()



#define TEMP_MIN_DEFAULT_VALUE          (10)
#define TEMP_MAX_DEFALUT_VALUE          (45)

#define HUMIDITY_MIN_DEFAULT_VALUE      (10)
#define HUMIDITY_MAX_DEFAULT_VALUE      (90)


extern struct sht20_t sht20;
extern struct temp_humidity_set_t gtemp_humidity_set;
extern struct temp_humidity_set_t *ptemp_humidity_set;

#endif //SHT20.h end
