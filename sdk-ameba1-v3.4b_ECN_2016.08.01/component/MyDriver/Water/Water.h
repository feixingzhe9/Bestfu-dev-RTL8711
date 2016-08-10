#ifndef __WATER_H
#define __WATER_H
//#include "BF_type.h"

#include "freertos_service.h"

#define WATER_DTC_PIN       PC_2//水位检测引脚
#define WATERING_PIN        PC_0//水泵控制引脚

#define WATERING_PWM_PERIOD             1.0/1000                //水泵PWM周期
#define WATERING_PWM_PULSE_WIDTH        WATERING_PWM_PERIOD/2   //水泵PWM脉冲宽度

#pragma pack(1)
struct watering_t       //浇水结构体
{
    u32 watering_period;         //浇水时间   单位：ms 
    u32 watering_interval;       //浇水间隔   单位：ms
    u8 cnt;             //浇水次数
    float pwm_period;     //水泵PWM周期
    float pwm_pulsewidth; //水泵PWM脉冲宽度
};
#pragma pack()

extern void upload_state(void);

#endif