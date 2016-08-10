#ifndef __WATER_H
#define __WATER_H
//#include "BF_type.h"

#include "freertos_service.h"

#define WATER_DTC_PIN       PC_2//ˮλ�������
#define WATERING_PIN        PC_0//ˮ�ÿ�������

#define WATERING_PWM_PERIOD             1.0/1000                //ˮ��PWM����
#define WATERING_PWM_PULSE_WIDTH        WATERING_PWM_PERIOD/2   //ˮ��PWM�������

#pragma pack(1)
struct watering_t       //��ˮ�ṹ��
{
    u32 watering_period;         //��ˮʱ��   ��λ��ms 
    u32 watering_interval;       //��ˮ���   ��λ��ms
    u8 cnt;             //��ˮ����
    float pwm_period;     //ˮ��PWM����
    float pwm_pulsewidth; //ˮ��PWM�������
};
#pragma pack()

extern void upload_state(void);

#endif