/**
 * @file bsp_DAC8563_x4.c
 * @author Jianfei (Ventus) Guo (ffventus@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-05-30
 * 
 * @copyright Copyright (c) 2019
 * 
 */

/*
*********************************************************************************************************
*
*	ģ������ : DAC8562/8563 ����ģ��(��ͨ����16λDAC)
*	�ļ����� : bsp_dac8562.c
*	��    �� : V1.0
*	˵    �� : DAC8562/8563ģ���CPU֮�����SPI�ӿڡ�����������֧��Ӳ��SPI�ӿں����SPI�ӿڡ�
*			  ͨ�����л���
*
*	�޸ļ�¼ :
*		�汾��  ����         ����     ˵��
*		V1.0    2014-01-17  armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp_DAC8563_x4.h"

#include <stm32f4xx.h>

static uint16_t _mth_DAC_FLOAT_To_UINT(float x);

#define SOFT_SPI		/* ������б�ʾʹ��GPIOģ��SPI�ӿ� */
//#define HARD_SPI		/* ������б�ʾʹ��CPU��Ӳ��SPI�ӿ� */

static void DAC8563_x4_WriteCmd_Parallel(uint32_t _cmd1, uint32_t _cmd2, uint32_t _cmd3, uint32_t _cmd4);
static void DAC8562_x4_WriteCmd_Parallel_Same(uint32_t _cmd);


/*
	DAC8501ģ�����ֱ�Ӳ嵽STM32-V5������CN19��ĸ(2*4P 2.54mm)�ӿ���

    DAC8562/8563ģ��    STM32F407������
	  GND   ------  GND    
	  VCC   ------  3.3V
	  
	  LDAC  ------  PA4/NRF905_TX_EN/NRF24L01_CE/DAC1_OUT
      SYNC  ------  PF7/NRF24L01_CSN
      	  
      SCLK  ------  PB3/SPI3_SCK
      DIN   ------  PB5/SPI3_MOSI

			------  PB4/SPI3_MISO
	  CLR   ------  PH7/NRF24L01_IRQ
*/

/*
	DAC8562��������:
	1������2.7 - 5V;  ������ʹ��3.3V��
	4���ο���ѹ2.5V��ʹ���ڲ��ο�

	��SPI��ʱ���ٶ�Ҫ��: �ߴ�50MHz�� �ٶȺܿ�.
	SCLK�½��ض�ȡ����, ÿ�δ���24bit���ݣ� ��λ�ȴ�
*/

#if !defined(SOFT_SPI) && !defined(HARD_SPI)
 	#error "Please define SPI Interface mode : SOFT_SPI or HARD_SPI"
#endif

#ifdef SOFT_SPI		/* ���SPI */
	/* ����GPIO�˿� */
	#define RCC_SCLK 	RCC_AHB1Periph_GPIOF
	#define PORT_SCLK	GPIOF
	#define PIN_SCLK	GPIO_Pin_8
	
	#define RCC_DIN 	RCC_AHB1Periph_GPIOC
	#define PORT_DIN 	GPIOC

	#define PIN_DIN1 	GPIO_Pin_4
	#define PIN_DIN2 	GPIO_Pin_5
	#define PIN_DIN3 	GPIO_Pin_8
	#define PIN_DIN4 	GPIO_Pin_9
	
	/* Ƭѡ */
	#define RCC_SYNC 	RCC_AHB1Periph_GPIOF
	#define PORT_SYNC	GPIOF
	#define PIN_SYNC	GPIO_Pin_7

	/* LDAC, ���Բ��� */
	#define RCC_LDAC	RCC_AHB1Periph_GPIOF
	#define PORT_LDAC	GPIOF
	#define PIN_LDAC	GPIO_Pin_6
	
	/* CLR, ���Բ��� */
	#define RCC_CLR 	RCC_AHB1Periph_GPIOD
	#define PORT_CLR	GPIOD
	#define PIN_CLR 	GPIO_Pin_6

	/* ���������0����1�ĺ� */
	#define SYNC_0()	PORT_SYNC->BSRRH = PIN_SYNC
	#define SYNC_1()	PORT_SYNC->BSRRL = PIN_SYNC

	#define SCLK_0()	PORT_SCLK->BSRRH = PIN_SCLK
	#define SCLK_1()	PORT_SCLK->BSRRL = PIN_SCLK

	#define DIN_0()		PORT_DIN->BSRRH = PIN_DIN1
	#define DIN_1()		PORT_DIN->BSRRL = PIN_DIN1
	

	#define LDAC_0()	PORT_LDAC->BSRRH = PIN_LDAC
	#define LDAC_1()	PORT_LDAC->BSRRL = PIN_LDAC

	#define CLR_0()		PORT_CLR->BSRRH = PIN_CLR
	#define CLR_1()		PORT_CLR->BSRRL = PIN_CLR	
#endif

#ifdef HARD_SPI		/* Ӳ��SPI (δ��) */
	;
#endif

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitDAC8562
*	����˵��: ����STM32��GPIO��SPI�ӿڣ��������� DAC8562
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8563_x4_hw_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef SOFT_SPI
	SYNC_1();	/* SYNC = 1 */

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_SCLK | RCC_DIN | RCC_CLR, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_SCLK;
	GPIO_Init(PORT_SCLK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_DIN1 | PIN_DIN2 | PIN_DIN3 | PIN_DIN4;
	GPIO_Init(PORT_DIN, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_SYNC;
	GPIO_Init(PORT_SYNC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_CLR;
	GPIO_Init(PORT_CLR, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = PIN_LDAC;
	GPIO_Init(PORT_LDAC, &GPIO_InitStructure);
#endif

	/* Power up DAC-A and DAC-B */
	//DAC8562_WriteCmd((4 << 19) | (0 << 16) | (3 << 0));
	DAC8562_x4_WriteCmd_Parallel_Same((4 << 19) | (0 << 16) | (3 << 0));
	
	/* LDAC pin inactive for DAC-B and DAC-A  ��ʹ��LDAC���Ÿ������� */
	//DAC8562_WriteCmd((6 << 19) | (0 << 16) | (3 << 0));
	//DAC8562_x4_WriteCmd_Parallel_Same((6 << 19) | (3 << 16) | (3 << 0));
	
	/* LDAC pin active for DAC-B and DAC-A  ʹ��LDAC���Ÿ������� */
	DAC8562_x4_WriteCmd_Parallel_Same((6 << 19) | (0 << 16) | (0 << 0));
	LDAC_1();	//Ӳ��LDACģʽ�£���ҪLDAC��ʼ����
	

	/* ��λ2��DAC���м�ֵ, ���2.5V */
	
	// DAC8563x_SetData(0, 32767);
	// DAC8563x_SetData(1, 32767);
	// DAC8563_x4_SetData_Parallel_Raw(32767,32767,32767,32767,32767,32767,32767,32767);
    DAC8563_x4_ZeroData_Parallel();
	//__NOP();//������������LDAC����֮�䣬��Ҫ5ns��
    //let's pretend these code below is an NOP :P
    volatile int dummy = 0;
    dummy++;
	LDAC_0();	//Ӳ��LDACģʽ�£�д��������ҪLDAC������ͬ������

	/* ѡ���ڲ��ο�����λ2��DAC������=2 ����λʱ���ڲ��ο��ǽ�ֹ��) */
	DAC8562_x4_WriteCmd_Parallel_Same((7 << 19) | (0 << 16) | (1 << 0));
}

/* -----------------------  code example of single DAC8563  ----------------------- */
/*
*********************************************************************************************************
*	�� �� ��: DAC8562_WriteCmd
*	����˵��: ��SPI���߷���24��bit���ݡ�
*	��    ��: _cmd : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
// void DAC8562_WriteCmd(uint32_t _cmd)
// {
// 	uint8_t i;
// 	SYNC_0();
	
// 	/*��DAC8562 SCLKʱ�Ӹߴ�50M����˿��Բ��ӳ� */
// 	for(i = 0; i < 24; i++)
// 	{
// 		if (_cmd & 0x800000)
// 		{
// 			DIN_1();
// 		}
// 		else
// 		{
// 			DIN_0();
// 		}
// 		//ʵ�ʼ����Է��֣�SCKL�������֮����³�����24ns
// 		SCLK_1();
// 		_cmd <<= 1;
// 		SCLK_0();
// 	}
	
// 	SYNC_1();
// }

/*
*********************************************************************************************************
*	�� �� ��: DAC8563_x4_WriteCmd_Parallel
*	����˵��: ��4·DAC8563��SPI����ͬʱ����4·24��bit��ͬ�����ݡ�
*	��    ��: _cmd : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DAC8563_x4_WriteCmd_Parallel(uint32_t _cmd1, uint32_t _cmd2, uint32_t _cmd3, uint32_t _cmd4)
{
    uint16_t zeroPins = 0x0;
    uint16_t onePins = 0x0;
	SYNC_0();
	
	/*��DAC8562 SCLKʱ�Ӹߴ�50M����˿��Բ��ӳ� */
	uint32_t idx = 0x800000;
	for(uint8_t i = 0; i < 24; i++)
	{
		zeroPins = 0x0;
		onePins = 0x0;
		
		if(_cmd1 & idx)
			onePins |= PIN_DIN1;
		else
			zeroPins |= PIN_DIN1;
		
		if(_cmd2 & idx)
			onePins |= PIN_DIN2;
		else
			zeroPins |= PIN_DIN2;
		
		if(_cmd3 & idx)
			onePins |= PIN_DIN3;
		else
			zeroPins |= PIN_DIN3;
		
		if(_cmd4 & idx)
			onePins |= PIN_DIN4;
		else
			zeroPins |= PIN_DIN4;
		
		if(onePins)
			GPIO_SetBits(PORT_DIN,onePins);
		if(zeroPins)
			GPIO_ResetBits(PORT_DIN,zeroPins);
		//ʵ�ʼ����Է��֣�SCKL�������֮����³�����24ns
		SCLK_1();
		idx >>= 1;
		SCLK_0();
	}
	
	SYNC_1();
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8562_x4_WriteCmd_Parallel_Same
*	����˵��: ��4·DAC8563��SPI����ͬʱ����24bit��ͬ�����ݡ�
*	��    ��: _cmd : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DAC8562_x4_WriteCmd_Parallel_Same(uint32_t _cmd)
{
	uint16_t common = PIN_DIN1 | PIN_DIN2 | PIN_DIN3 | PIN_DIN4;
	SYNC_0();
	
	/*��DAC8562 SCLKʱ�Ӹߴ�50M����˿��Բ��ӳ� */
	for(uint8_t i = 0; i < 24; i++)
	{
		if (_cmd & 0x800000)
		{
			GPIO_SetBits(PORT_DIN,common);
		}
		else
		{
			GPIO_ResetBits(PORT_DIN,common);
		}
		//ʵ�ʼ����Է��֣�SCKL�������֮����³�����24ns
		SCLK_1();
		_cmd <<= 1;
		SCLK_0();
	}
	
	SYNC_1();
}

/* -----------------------  code example of single DAC8563  ----------------------- */
/*
*********************************************************************************************************
*	�� �� ��: DAC8563x_SetData
*	����˵��: ����DAC��������������¡�
*	��    ��: _ch, ͨ��, 0 , 1
*		     _data : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
// void DAC8563x_SetData(uint8_t _ch, uint16_t _dac)
// {
// 	if (_ch == 0)
// 	{
// 		/* Write to DAC-A input register and update DAC-A; */
// 		DAC8562_WriteCmd((3 << 19) | (0 << 16) | (_dac << 0));
// 	}
// 	else if (_ch == 1)
// 	{
// 		/* Write to DAC-B input register and update DAC-A; */
// 		DAC8562_WriteCmd((3 << 19) | (1 << 16) | (_dac << 0));		
// 	}
// }

void DAC8563_x4_ZeroData_Parallel(void)
{
	DAC8563_x4_SetData_Parallel_Raw(__CST_DAC8563_OUTPUT_ZERO,__CST_DAC8563_OUTPUT_ZERO,__CST_DAC8563_OUTPUT_ZERO,__CST_DAC8563_OUTPUT_ZERO,__CST_DAC8563_OUTPUT_ZERO,__CST_DAC8563_OUTPUT_ZERO,__CST_DAC8563_OUTPUT_ZERO,__CST_DAC8563_OUTPUT_ZERO);
}

void DAC8563_x4_SetData_Parallel_Raw(uint16_t _dac11, uint16_t _dac12, uint16_t _dac21, uint16_t _dac22, uint16_t _dac31, uint16_t _dac32, uint16_t _dac41, uint16_t _dac42)
{
	LDAC_1();
	
	DAC8563_x4_WriteCmd_Parallel((3 << 19) | (0 << 16) | (_dac11 << 0), 
				(3 << 19) | (0 << 16) | (_dac21 << 0),
				(3 << 19) | (0 << 16) | (_dac31 << 0),
				(3 << 19) | (0 << 16) | (_dac41 << 0));
	
	DAC8563_x4_WriteCmd_Parallel((3 << 19) | (1 << 16) | (_dac12 << 0), 
				(3 << 19) | (1 << 16) | (_dac22 << 0),
				(3 << 19) | (1 << 16) | (_dac32 << 0),
				(3 << 19) | (1 << 16) | (_dac42 << 0));
	
	LDAC_0();
}

/**
 * @brief 
 * 
 * @param data_x8 
 */
void DAC8563_x4_SetData_Parallel_RawArray(uint16_t * data_x8)
{
	LDAC_1();
	
	DAC8563_x4_WriteCmd_Parallel((3 << 19) | (0 << 16) | (data_x8[0] << 0), 
				(3 << 19) | (0 << 16) | (data_x8[2] << 0),
				(3 << 19) | (0 << 16) | (data_x8[4] << 0),
				(3 << 19) | (0 << 16) | (data_x8[6] << 0));
	
	DAC8563_x4_WriteCmd_Parallel((3 << 19) | (1 << 16) | (data_x8[1] << 0), 
				(3 << 19) | (1 << 16) | (data_x8[3] << 0),
				(3 << 19) | (1 << 16) | (data_x8[5] << 0),
				(3 << 19) | (1 << 16) | (data_x8[7] << 0));
	
	LDAC_0();
}

//��DAC�����͵�ѹֵתΪ����ֵ
static uint16_t _mth_DAC_FLOAT_To_UINT(float x)
{
    // not needed. because there is no physical meaning of this operaion
    // && uint16_t already ensure the limitation of 0~65535
    // return (uint16_t)(((int16_t)(x * __CST_DAC_1_V_TO_BIT) + __CST_DAC8563_OUTPUT_ZERO) % __CST_DAC8563_OUTPUT_MAX);
    return (uint16_t)(((int16_t)(x * __CST_DAC_1_V_TO_BIT) + __CST_DAC8563_OUTPUT_ZERO));
}

void DAC8563_x4_SetData_Parallel_VoltageArray(float * data_x8)
{
    uint16_t i_data[8];
    for(uint8_t i = 0; i < sizeof(i_data); i++)
    {
        i_data[i] = _mth_DAC_FLOAT_To_UINT(data_x8[i]);
    }
    DAC8563_x4_SetData_Parallel_RawArray(i_data);
}

void DAC8563_x4_SetData_Parallel_Voltage(float _dac11, float _dac12, float _dac21, float _dac22, float _dac31, float _dac32, float _dac41, float _dac42)
{
    float data[8];
    data[0] = _dac11;
    data[1] = _dac12;
    data[2] = _dac21;
    data[3] = _dac22;
    data[4] = _dac31;
    data[5] = _dac32;
    data[6] = _dac41;
    data[7] = _dac42;
    DAC8563_x4_SetData_Parallel_VoltageArray(data);
}



/**** Copyright (C)2018 Ventus Guo. All Rights Reserved **** END OF FILE ****/
