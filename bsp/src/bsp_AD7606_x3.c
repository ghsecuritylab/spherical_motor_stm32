#include "bsp_AD7606_x3.h"
#include <stm32f4xx.h>

//todo �ж��е�����

static void AD7606_CtrlGPIOConfig(void);
static void AD7606_FSMCConfig(void);

//3��ADC��A,B��ͨ����ǿ�Ʋ���
//�˴�����3��ADC:1,2,3�Ŀ�����������ŵ�ƽ�仯����
#define AD7606_CONV_1_L() GPIO_ResetBits(GPIOG, GPIO_Pin_6)
#define AD7606_CONV_1_H() GPIO_SetBits(GPIOG, GPIO_Pin_6)

#define AD7606_CONV_2_L() GPIO_ResetBits(GPIOG, GPIO_Pin_7)
#define AD7606_CONV_2_H() GPIO_SetBits(GPIOG, GPIO_Pin_7)

#define AD7606_CONV_3_L() GPIO_ResetBits(GPIOG, GPIO_Pin_8)
#define AD7606_CONV_3_H() GPIO_SetBits(GPIOG, GPIO_Pin_8)

#define AD7606_RESET_L() GPIO_ResetBits(GPIOG, GPIO_Pin_11)
#define AD7606_RESET_H() GPIO_SetBits(GPIOG, GPIO_Pin_11)

#define AD7606_1_READY() (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_RESET)
#define AD7606_2_READY() (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == Bit_RESET)
#define AD7606_3_READY() (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == Bit_RESET)

/* AD7606 FSMC���ߵ�ַ��ֻ�ܶ�������д */
	//6			C			4			0			0			0			0			0
	//0110 	1100 	0100 	0000 	0000 	0000 	0000 	0000
	//31:28	27:24	23:20	19:16	15:12	11:8	7:4		3:0
	//A27,A26��FSMC_NE1,FSMC_NE2,FSMC_NE3,FSMC_NE4������ѡ4��bank
	//A25:0���Լ����ĵ�ַ

//���ε����ʹ��138��������3��AD7606����Ƭѡ
//���ӹ�ϵ��
	//G1���ߣ�G2A,G2B������FSMC_NE4(PG12)
	//A:A23,		B:A24,		C:A25;
	//Y0:AD7606_CS1,		Y1:AD7606_CS2,		Y2:AD7606_CS3
//ʹ�ܱ�
	//000:AD7606_1
	//001:AD7606_2
	//010:AD7606_3

//AD7606_1:   00 0000 x -> 00 0000 x
	//6			C			0			0			0			0			0			0
	//0110 	1100 	0000 	0000 	0000 	0000 	0000 	0000
	//31:28	27:24	23:20	19:16	15:12	11:8	7:4		3:0
#define AD7606_RESULT_1() *(__IO uint16_t *)0x6C000000

//AD7606_2:   01 0000 x -> 00 1000 x
	//6			D			0			0			0			0			0			0
	//0110 	1101 	0000 	0000 	0000 	0000 	0000 	0000
	//31:28	27:24	23:20	19:16	15:12	11:8	7:4		3:0
#define AD7606_RESULT_2() *(__IO uint16_t *)0x6D000000

//AD7606_3:   10 0000 x -> 01 0000 x
	//6			E			0			0			0			0			0			0
	//0110 	1110 	0000 	0000 	0000 	0000 	0000 	0000
	//31:28	27:24	23:20	19:16	15:12	11:8	7:4		3:0
#define AD7606_RESULT_3() *(__IO uint16_t *)0x6E000000

/* if use soft oversample */
#ifdef AD7606_USE_BOARD_OVERSAMPLE
/* ���ù�������GPIO: PH9 PH10 PH11 */
#define OS0_1() GPIOH->BSRRL = GPIO_Pin_9
#define OS0_0() GPIOH->BSRRH = GPIO_Pin_9
#define OS1_1() GPIOH->BSRRL = GPIO_Pin_10
#define OS1_0() GPIOH->BSRRH = GPIO_Pin_10
#define OS2_1() GPIOH->BSRRL = GPIO_Pin_11
#define OS2_0() GPIOH->BSRRH = GPIO_Pin_11

#endif

#ifdef AD7606_USE_BOARD_RANGE_SEL
#define AD7606_RANGE_H() GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define AD7606_RANGE_L() GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#endif
/*
	STM32��������߷�����

	PD0/FSMC_D2
	PD1/FSMC_D3
	//PD4/FSMC_NOE		--- �������źţ�OE = Output Enable �� N ��ʾ����Ч
	    //��Ϊ�����ֲ��� CS �� RD ֱ�������ķ�ʽ����˴˴�Ҳ����ҪRD�ź���
	//PD5/FSMC_NWE		--- д�����źţ�AD7606 ֻ�ж�����д�ź�
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15

	PD14/FSMC_D0
	PD15/FSMC_D1

	PE4/FSMC_A20		
	PE5/FSMC_A21		
	
	PE2/FSMC_A23		--- ����Ƭѡһ������
	PG13/FSMC_A24		--- ����Ƭѡһ������
	PG14/FMSC_A25		--- ����Ƭѡһ������
	
	PE7/FSMC_D4
	PE8/FSMC_D5
	PE9/FSMC_D6
	PE10/FSMC_D7
	PE11/FSMC_D8
	PE12/FSMC_D9
	PE13/FSMC_D10
	PE14/FSMC_D11
	PE15/FSMC_D12

	PG12/FSMC_NE4		--- ��Ƭѡ��TFT, OLED �� AD7606��

	�����Ŀ���IO:

	PH9/DCMI_D0/AD7606_OS0			---> AD7606_OS0		OS2:OS0 ѡ�������˲�����
	PH10/DCMI_D1/AD7606_OS1         ---> AD7606_OS1
	PH11/DCMI_D2/AD7606_OS2         ---> AD7606_OS2
	PH12/DCMI_D3/AD7606_CONVST      ---> AD7606_CONVST	����ADCת�� (CONVSTA �� CONVSTB �Ѿ�����)
	PH14/DCMI_D4/AD7606_RAGE        ---> AD7606_RAGE	����ģ���ѹ���̣�����5V������10V
	PI4/DCMI_D5/AD7606_RESET        ---> AD7606_RESET	��λ
	PI6/DCMI_D6/AD7606_BUSY         ---> AD7606_BUSY	æ�ź�	

*/

/**
 * @brief Configure GPIO settings of FSMC Bus.
 * 
 */
static void AD7606_CtrlGPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��FSMCʱ�� */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

	/* ʹ�� GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);

	/* ���� PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
	 PD.10(D15), PD.14(D0), PD.15(D1) Ϊ����������� */

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
								  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
								  GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/*
		PE4/FSMC_A20		--- ����Ƭѡһ������
		PE5/FSMC_A21		--- ����Ƭѡһ������
		PE6/FSMC_A22
		PE2/FSMC_A23		--- ����Ƭѡһ������

		PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 	PE.14(D11), PE.15(D12)
	*/
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource2, GPIO_AF_FSMC);

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |
								  GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
								  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
								  GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/*
		PG13/FSMC_A24		--- ����Ƭѡһ������
		PG14/FMSC_A25		--- ����Ƭѡһ������
	*/
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* ���� PG12  Ϊ����������� */ //FSMC_NE4����Ƭѡ
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/*	���ü��������õ�GPIO
		PG6/AD7606_CONVST1      ---> AD7606_CONVST1	����ADCת��1
		PG7/AD7606_CONVST2      ---> AD7606_CONVST2	����ADCת��2
		PG8/AD7606_CONVST3      ---> AD7606_CONVST3	����ADCת��2
		PG11/AD7606_RESET        ---> AD7606_RESET	��λ
		
        ����3�����жϴ�����
		PA8/AD7606_BUSY1			---> AD7606_BUSY1    ת���������ź�
		PB8/AD7606_BUSY2			---> AD7606_BUSY2    ת���������ź�
		PB9/AD7606_BUSY3			---> AD7606_BUSY3    ת���������ź�
	*/
	{
		/* ʹ�� GPIOGʱ�� */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_11;
		GPIO_Init(GPIOG, &GPIO_InitStructure);
	}
}

/**
 * @brief Configure FSMC parallel port
 * 
 */
static void AD7606_FSMCConfig(void)
{
	FSMC_NORSRAMInitTypeDef init;
	FSMC_NORSRAMTimingInitTypeDef timing;

	/*
		AD7606�����Ҫ��(3.3Vʱ)��RD���źŵ͵�ƽ���������21ns���ߵ�ƽ������̿��15ns��

		������������ ������������Ϊ�˺�ͬBANK��LCD������ͬ��ѡ��3-0-6-1-0-0
		3-0-5-1-0-0  : RD�߳���75ns�� �͵�ƽ����50ns.  1us���ڿɶ�ȡ8·�������ݵ��ڴ档
		1-0-1-1-0-0  : RD��75ns���͵�ƽִ��12ns���ң��½��ز��Ҳ12ns.  ���ݶ�ȡ��ȷ��
	*/
	/* FSMC_Bank1_NORSRAM4 configuration */

	//ʵ��1-0-1-1-0-0������channel����
	timing.FSMC_AddressSetupTime = 3;
	timing.FSMC_AddressHoldTime = 0;
	timing.FSMC_DataSetupTime = 5;
	timing.FSMC_BusTurnAroundDuration = 1;
	timing.FSMC_CLKDivision = 0;
	timing.FSMC_DataLatency = 0;
	timing.FSMC_AccessMode = FSMC_AccessMode_A;

	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	init.FSMC_WrapMode = FSMC_WrapMode_Disable;
	init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

	init.FSMC_ReadWriteTimingStruct = &timing;
	init.FSMC_WriteTimingStruct = &timing;

	FSMC_NORSRAMInit(&init);

	/* - BANK 1 (of NOR/SRAM Bank 1~4) is enabled */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

//��ADC����ֵתΪ������ֵ
static float _mth_ADC_UINT_To_FLOAT(int16_t x)
{
	#ifndef AD7606_USE_BOARD_RANGE_SEL
    return x * _cst_ADC_1_BIT_TO_V;
	#else
	#error "todo"
	#endif
}

/**
 * @brief Hardware reset AD7606. After reseting, AD7606 will restore normal work state.
 * 
 */
void AD7606_x3_Reset(void)
{
	AD7606_RESET_L(); /* �˳���λ״̬ */

	AD7606_RESET_H(); /* ���븴λ״̬ */
	AD7606_RESET_H(); /* only for delay an interval. RESET high level requires a minimum period of 50ns�� */
	AD7606_RESET_H();
	AD7606_RESET_H();

	AD7606_RESET_L(); /* �˳���λ״̬ */
}

/**
 * @brief start a ADC convert once.
 * 
 * @param ADC_port 
 */
void AD7606x_StartConvst(uint8_t ADC_port)
{
	/* page 7��  CONVST �ߵ�ƽ�����Ⱥ͵͵�ƽ��������� 25ns */
	/* CONVSTƽʱΪ�� */
	switch (ADC_port)
	{
	case AD7606_PORT1:
	{
		//ʵ��200ns
		AD7606_CONV_1_L();
		AD7606_CONV_1_L();
		AD7606_CONV_1_L();
		AD7606_CONV_1_L();

		AD7606_CONV_1_H();
		break;
	}
	case AD7606_PORT2:
	{
		AD7606_CONV_2_L();
		AD7606_CONV_2_L();
		AD7606_CONV_2_L();
		AD7606_CONV_2_L();

		AD7606_CONV_2_H();
		break;
	}
	case AD7606_PORT3:
	{
		AD7606_CONV_3_L();
		AD7606_CONV_3_L();
		AD7606_CONV_3_L();
		AD7606_CONV_3_L();

		AD7606_CONV_3_H();
		break;
	}
	default:
	{
		break;
	}
	}
}

void AD7606x_ReadNowAdc_RawArray(uint8_t ADC_port, int16_t *rcvBuf, uint8_t num)
{
	//todo check the validity of num
	uint8_t idx = 0;
	switch (ADC_port)
	{
	case AD7606_PORT1:
	{
		for (idx = 0; idx < num; idx++)
		{
			rcvBuf[idx] = AD7606_RESULT_1();
			break;
		}
	}
	case AD7606_PORT2:
	{
		for (idx = 0; idx < num; idx++)
		{
			rcvBuf[idx] = AD7606_RESULT_2();
			break;
		}
	}
	case AD7606_PORT3:
	{
		for (idx = 0; idx < num; idx++)
		{
			rcvBuf[idx] = AD7606_RESULT_3();
			break;
		}
	}
	}
}

void AD7606x_ReadNowAdc_VoltageArray(uint8_t ADC_port, float *rcvBuf, uint8_t num)
{
	//todo check the validity of num
	int16_t data[AD7606_CHANNEL_MAX];
	AD7606x_ReadNowAdc_RawArray(ADC_port, data, num);
	for(uint8_t i = 0; i < num; i++)
	{
		rcvBuf[i] = _mth_ADC_UINT_To_FLOAT(data[i]);
	}
}

uint8_t AD7606x_IsReady(uint8_t ADC_port)
{
	switch (ADC_port)
	{
	case AD7606_PORT1:
	{
		return AD7606_1_READY();
	}
	case AD7606_PORT2:
	{
		return AD7606_2_READY();
	}
	case AD7606_PORT3:
	{
		return AD7606_3_READY();
	}
	default:
	{
		return 0;
		break;
	}
	}
}

void AD7606_x3_hw_init(void)
{
	AD7606_CtrlGPIOConfig();
	AD7606_FSMCConfig();

#ifdef AD7606_USE_BOARD_OVERSAMPLE
	AD7606_SetOS(AD_OS_NO); //no oversampling
#endif

#ifdef AD7606_USE_BOARD_RANGE_SEL
	AD7606_SetInputRange(0); //0 for +-5V, 1 for +-10V
#endif

	AD7606_x3_Reset();

	AD7606_CONV_1_H(); /* ����ת����GPIOƽʱ����Ϊ�� */
	AD7606_CONV_2_H();
	AD7606_CONV_3_H();
}