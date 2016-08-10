/***************************Copyright BestFu 2016-03-03*************************
文 件：    kalman_filter.h
说 明：    卡尔曼滤波处理函数
编 译：    Keil uVision V5.12.0.0
版 本：    v1.0
编 写：    jay
日 期：    2016-03-03
修  改:    无
*******************************************************************************/
#include "Kalman_Filter.h"



/*******************************************************************************
函 数 名：  void kalman_init(kalman_state_t *state, float init_x, float init_p)
功能说明：  Init fields of structure @kalman1_state.
参   数：   state	:Klaman filter structure
			init_x	:initial x state value
			init_p	:initial estimated error convariance
返 回 值：  无
说    明：  在实际运用中,请根据实际工程改变A,H,q,r的初始值.
*******************************************************************************/
void kalman_init(Kalman_State_t *state, float init_x, float init_p)
{
    state->x = init_x;
    state->p = init_p;
    state->A = 1;
    state->H = 0.96;
    state->q = 2e2;//10e-6;  /* predict noise convariance */
    state->r = 5e2;//10e-5;  /* measure error convariance */
}

/*******************************************************************************
函 数 名：  float kalman_filter(kalman_state_t *state, float z_measure)
功能说明：  1维卡尔曼滤波函数
参   数：   state		:Klaman filter structure
			z_measure	:Measure value
返 回 值：  估算结果
说    明：  无
*******************************************************************************/
float kalman_filter(Kalman_State_t *state, float z_measure)
{
    /* Predict */
    state->x = state->A * state->x;
    state->p = state->A * state->A * state->p + state->q;  /* p(n|n-1)=A^2*p(n-1|n-1)+q */

    /* Measurement */
    state->gain = state->p * state->H / (state->p * state->H * state->H + state->r);
    state->x = state->x + state->gain * (z_measure - state->H * state->x);
    state->p = (1 - state->gain * state->H) * state->p;

    return state->x;
}
/***********************************End of File*******************************/
