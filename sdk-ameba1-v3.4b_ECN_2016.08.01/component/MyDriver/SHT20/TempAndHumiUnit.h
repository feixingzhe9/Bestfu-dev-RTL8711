/***************************Copyright BestFu ***********************************
**  文    件：  TempAndHumiUnit.c
**  功    能：  温湿度单元
**  版    本：  V1.1
**  编    写：  Seven
**  创建日期：  2014.09.16
**  修改日期：  2014.09.16
**  说    明：  >>
*******************************************************************************/
#ifndef _TEMP_UNIT_H_
#define _TEMP_UNIT_H_

#include "BF_type.h"
#include "UnitCfg.h"

#define TEMP_UPLOAD_PACE      10          //温度变化上报 差值1℃
#define HUMI_UPLOAD_PACE      5           //湿度变化上报 差值5%

#define TEMP_LINKQUERY_PACE      5          //温度变化上报 差值0.5℃
#define HUMI_LINKQUERY_PACE      2          //湿度变化上报 差值2%

extern short gTemper;
extern short  gHumi;
extern short OldTemp;
extern short OldHumi;                
short TempSampVal(void);
u8   HumiSampVal(void);
void Init_TempAndHumi(void);	   //初始化	
void TempAndHumi_Deal(void);

#endif

/***************************Copyright BestFu **********************************/

