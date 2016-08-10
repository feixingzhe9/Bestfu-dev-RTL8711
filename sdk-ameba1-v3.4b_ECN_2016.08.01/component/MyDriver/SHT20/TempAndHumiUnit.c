/***************************Copyright BestFu ***********************************
**  文    件：  TempAndHumiUnit.c
**  功    能：  温湿度单元
**  版    本：  V1.1
**  编    写：  Seven
**  创建日期：  2014.09.16
**  修改日期：  2014.09.16
**  说    明：  >>
*******************************************************************************/
#include "TempAndHumiUnit.h"
#include "SHT20.H"
#include "Sensor4in1ExeAttr.h"
short gTemper = 0;                //温度有负值
short  gHumi  = 0;                    //湿度
extern void Sensor4in1_Upload(u8 level);


/*******************************************************************************
**函    数： Init_TempAndHumi()
**功    能： 温湿度传感器初始化
**参    数： void
**返    回： void
*******************************************************************************/
void Init_TempAndHumi(void)
{
	SHT20_Init();
}

/*******************************************************************************
**函    数： TempSampVal()
**功    能： 从传感器获取温度 
**参    数： void
**返    回： 温度值(数值扩大10倍) 精度0.1℃ 
*******************************************************************************/
short TempSampVal(void)
{
	float T = (SHT20_Read(TEMP)*175.72)/0x10000 - 46.85;  
	return (T*10);
}
/*******************************************************************************
**函    数： HumiSampVal()
**功    能： 从传感器获取湿度
**参    数： void
**返    回： 湿度值  精度1% RH
*******************************************************************************/
u8 HumiSampVal(void)
{
	float H;
	u16 HumiV = SHT20_Read(HUMI);       
	HumiV &= ~0x0003;                   
	H = -6.0 + (125.0* HumiV)/65536.0;  
	return (H);                     
}

/*******************************************************************************
**函    数： ABS_SUB()
**功    能： 求两数的差的绝对值   |(A-B)|
**参    数： void
**返    回： |(A-B)|
*******************************************************************************/
u16 ABS_SUB(u16 A, u16 B)
{
    if(A>=B) return (A-B);
    else return (B-A);
}

/*******************************************************************************
**函    数： TemperAndHumi_Deal()
**功    能： 温湿度处理
**参    数： void
**返    回： void
*******************************************************************************/
void TempAndHumi_Deal(void)
{
	static u8 OldMode = 0,NewMode = 0 ;
	static short TempOffset = 0 ,HumiOffset = 0 ; //偏移值
	static short OldTemp = 0 ,OldHumi = 0;     //旧值
//确保打开自动模式时可以遍历联动表	
	NewMode = gAuto_Mode.mode ; 
	if(OldMode == MODE_OFF && NewMode == MODE_ON)
	{
		OldTemp = 0;
		TempOffset = 0 ;
		OldHumi = 0;
		HumiOffset = 0 ;
	}
	OldMode = NewMode ;
	
	gTemper = TempSampVal() + gTemper_Cali.cali_val;    //读取温度值  放大10倍 加上修正值
	if(gTemper <= -468 ) gTemper = -468 ;  //最小值-46.8
	else if(gTemper >= 1250) gTemper  = 1250 ; //最大值+125
	TempOffset = TempOffset + (gTemper - OldTemp) ;
	
	
	gHumi   = HumiSampVal() + gHumi_Cali.cali_val;    //读取湿度值  放大1倍 加上修正值
	if(gHumi <= 0 ) gHumi = 0 ;  //最小值0
	else if(gHumi >= 100) gHumi  = 100 ; //最大值100
	HumiOffset = HumiOffset + (gHumi - OldHumi);	
	
	//突变超过上报门槛	
    if( (ABS_SUB(gTemper,OldTemp) >= TEMP_UPLOAD_PACE )|| ABS_SUB(gHumi , OldHumi) >= HUMI_UPLOAD_PACE )
    {       
		Sensor4in1_Upload(0);
    }
	OldTemp = gTemper ;
	OldHumi = gHumi ;
	//变化超过联动遍历门槛	
	if((ABS_SUB(TempOffset , 0) >= TEMP_LINKQUERY_PACE ))
	{
		TempOffset = 0 ;
		if(NewMode == MODE_ON && gTemper >= 0 )//温度为正数，负值不进行联动执行 2015/08/28 yanhuan adding)
		{
			if(Delate_Able)
				PropEventFifo(1, 0x21,SRCVALUE,gTemper);	
		}					
	}
	if((ABS_SUB(HumiOffset , 0) >= HUMI_LINKQUERY_PACE ))
	{
		 HumiOffset = 0 ;
		 if(NewMode == MODE_ON)
			 if(Delate_Able)
			 {
					PropEventFifo(1, 0x31,SRCVALUE,gHumi);
			 }	        			 
	}	
}

/***************************Copyright BestFu **********************************/
