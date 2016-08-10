/***************************Copyright BestFu 2014-05-14*************************
文	件：	Eeprom.h
说	明：	对Eeprom操作,包括全局参数与EEPROM映射关系，独立读写EEPROM入口头文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Joey
日	期：	2013.7.23   
修  改:     暂无
*******************************************************************************/
#ifndef _EEPROM_H_
#define _EEPROM_H_
//#include "BF_type.h"
#include "freertos_service.h"
#include "SHT20.h"


#define  A24C256               (256*1024/8)                 // 24C256大小
#define  EEPROMSIZE            (A24C256-EEPROMSTARTADDR)    // EEPROM尺寸
#define  PAGESIZE              (64)                         // 页尺寸
#define  I2CBUFFLEN            (PAGESIZE)                   // 写缓冲区大小
#define  EEPROMSTARTADDR       (100)                        // EEPROM器件首地址
#define  EEPROMENDADDR         (A24C256)                    // EEPROM器件尾地址
#define  EEPROMSLAVEADDR       (0xA0)                       // EEPROM从器件地址

#define  EEPROMINITED          (0xA5)


///////
#define RTCSLAVEADDR           (0x64)
///////


u8 ReadDataFromEEPROM(u32 addr, u32 len, u8 *data);
u8 WriteDataToEEPROM(u32 addr, u32 len, u8 *data);
u8 WriteDataToEEPROMEx(u32 addr, u32 len, u8 data);



///////////////////EEPROM存储结构/////////////////////////////
#define TEMP_HUMIDITY_SET_START_ADDR            (0x0040)
#define TEMP_HUMIDITY_SET_SPACE                 sizeof(struct temp_humidity_set_t)
//////////////////////////////////////////////////////////////

#endif
/**************************Copyright BestFu 2014-05-14*************************/
