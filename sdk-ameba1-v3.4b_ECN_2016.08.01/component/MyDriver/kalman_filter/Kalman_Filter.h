/***************************Copyright BestFu 2016-03-03*************************
文 件：    kalman_filter.h
说 明：    卡尔曼滤波处理函数
编 译：    Keil uVision V5.12.0.0
版 本：    v1.0
编 写：    jay
日 期：    2016-03-03
修  改:    无
*******************************************************************************/
#ifndef  _KALMAN_FILTER_H
#define  _KALMAN_FILTER_H

/**************************1维卡尔曼滤波数据结构定义**************************/
typedef struct {
    float x;  /* state */
    float A;  /* x(n)=A*x(n-1)+u(n),u(n)~N(0,q) */
    float H;  /* z(n)=H*x(n)+w(n),w(n)~N(0,r)   */
    float q;  /* process(predict) noise convariance */
    float r;  /* measure noise convariance */
    float p;  /* estimated error convariance */
    float gain;
}Kalman_State_t;

extern Kalman_State_t Kalman_Temp;

/***************************1维卡尔曼滤波函数声明*****************************/
extern void kalman_init(Kalman_State_t *state, float init_x, float init_p);
extern float kalman_filter(Kalman_State_t *state, float z_measure);

#endif  /*_KALMAN_FILTER_H*/
/********************************End of File***********************************/
