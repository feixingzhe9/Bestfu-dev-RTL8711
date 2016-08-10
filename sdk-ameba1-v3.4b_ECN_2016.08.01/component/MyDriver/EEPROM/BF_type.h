/***************************Copyright BestFu 2014-05-14*************************
文	件：	BF_type.h
说	明：	公共头文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Joey
日	期：	2013.8.9 
修  改:     Unary (2014.08.15, 删除了大部分定义，只留下了最简定义)
*******************************************************************************/
#ifndef _BF_TYPE_H_
#define _BF_TYPE_H_



#define TRUE                      		(1)
#define FALSE                     		(0)

#define NULL                      		((void*)(0))
#define CLEAR                       	(0xFF)

#define offsetof(TYPE, MEMBER) 			((u32) &((TYPE *)0)->MEMBER)                //用于获取结构体中成员的偏移量
#define BF_DIM(array_) 					(sizeof(array_) / sizeof((array_)[0]))		//计算数组中的元素个数
#define NUM_ROUND(a,b)					(((a)/(b))+ ((0==((a)%(b)))?(0):(1)))		//计算数据包的个数

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef signed int    s32;
typedef signed short   s16;
typedef signed char    s8;

typedef volatile unsigned int   vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char  vu8;

typedef volatile unsigned int  const vuc32;  /* Read Only */
typedef volatile unsigned short const vuc16;  /* Read Only */
typedef volatile unsigned char  const vuc8;   /* Read Only */

#endif
/**************************Copyright BestFu 2014-05-14*************************/
