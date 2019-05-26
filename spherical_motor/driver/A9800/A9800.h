/**
 * @file A9800.h
 * @author Jianfei (Ventus) Guo (ffventus@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-05-26
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef __A9800__H__INCLUDED
#define __A9800__H__INCLUDED

#include <stm32f4xx.h>
#include "A9800Chip.h"
#include "A9800Srom.h"

/***************************************************************************************************
定义:   系统配置.
***************************************************************************************************/

typedef enum    //操作类.
{
    OP_FAIL,
    OP_SUCCESS = !OP_FAIL
}OP;

typedef enum
{
	ADNS9800_CHIP1 = 0,
	ADNS9800_CHIP2 = 1
}ADNS9800_CHIP;

typedef struct{
    uint8_t   g_uA9800Cpi;     //CPI值.
    uint8_t   g_uA9800Lift;                  //抬起高度门限.

    int16_t MotionX_1;
    int16_t MotionY_1;
    int16_t MotionX_2;
    int16_t MotionY_2;

    int32_t  PosX_1;                //X轴位移.
    int32_t  PosY_1;                //Y轴位移.

    int32_t  PosX_2;              //X轴位移.第2块鼠标芯片
    int32_t  PosY_2;              //Y轴位移.第2块鼠标芯片

    uint32_t  TotalMotionX;						//x轴绝对值位移，鼠标芯片-1
    uint32_t  TotalMotionY;						//y轴绝对值位移，鼠标芯片-1

    uint32_t  TotalMotionX_2;					//x轴绝对值位移，鼠标芯片-2
    uint32_t  TotalMotionY_2;					//y轴绝对值位移，鼠标芯片-2
} ADNS9800_VAR_T;

/***************************************************************************************************
定义:   公有方法.
***************************************************************************************************/
//功能.
extern void A9800SetCpi(uint8_t uCpi);
extern void A9800SetLift(uint8_t uLift);
extern uint8_t    SpiXRead(uint8_t chip);                 //读.读之前必须保证片选有效.
extern void     SpiXWrite(uint8_t uWriteData, uint8_t chip);    //写.写之前必须保证片选有效.

//调度.
extern void A9800OnTimerTask(void);

//状态.
extern void A9800XSInit(uint8_t bIap);
extern void SpiXSInit(void);                 //初始化.
void bsp_ADNS9800_Initializes(void);
/**************************************************************************************************/


#endif