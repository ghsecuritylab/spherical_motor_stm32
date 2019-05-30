#ifndef __CONFIGURATION
#define __CONFIGURATION
#include "stm32f4xx.h"
#include "arm_math.h"



//����ģʽ���أ��뱣�ֿ���
#define __CFG_DEBUG_MODE     0

//�Ƿ�Ϊ���Թ���ģʽ�����Թ���ʱΪ1������PIDʱΪ0��
#define __CFG_TEST_AMPLIFIER 0

//�Ƿ�Ϊ���Խ������ģʽ�������Խ�����㲻��ʵ�������Ϊ1��������Ϊ0��
#define __CFG_TEST_DECOUPLE  0

//�Ƿ�Ϊ������̬����ģʽ��������Ϊ1��
#define __CFG_TEST_ORIENTATION   0




//�ṹ����
typedef enum{
    __CTL_STATUS_RUNNING = 0,
    __CTL_STATUS_STOP = 1
} SYSTEM_STATUS;
typedef enum{
    __CTL_MODE_ANGULAR_VELOCITY = 0,
    __CTL_MODE_ANGULAR_POSITION = 1,
    __CTL_MODE_TORQUE_STATOR_OUTPUT = 2,
    __CTL_MODE_TORQUE_ROTOR_OUTPUT = 3
} SYSTEM_MODE;
typedef struct{
    SYSTEM_STATUS STATUS;
    SYSTEM_MODE MODE;
} __CTL_SYSTEM_CONTROL;
//*******************************************************************************//
//**************************** �²㽨������(�����) **************************************//
//*******************************************************************************//

//**************************** ADC ���� **************************************//
//����3��ADC�еĿ������
#define __CFG_USE_ADC_1 1
#define __CFG_USE_ADC_2 1
#define __CFG_USE_ADC_3 1
//���ò����ʣ�Ҳ����ϵͳ��ʱ��
#define __CST_SAMPLE_RATE_100KHZ     100000
#define __CST_SAMPLE_RATE_50khz      50000
#define __CST_SAMPLE_RATE_40KHZ      40000
#define __CST_SAMPLE_RATE_20KHZ      20000
#define __CST_SAMPLE_RATE_10KHZ      10000
#define __CST_SAMPLE_RATE_5KHZ       5000
// AD7606*8(*3) �����ʱ��4+2+2+2 = 10us    DAC8563*2*4д����ʱ 20us 


//!!! VENTUSFF: ע�⣺��ʱ������Ŀǰ���÷�ʽ:
//  ����ʱ�ӿ�ʼ��ĵ�2�����ڿ�ʼ��⣬��⵽AD7606_2(/3)��BUSY���ŴӸߵ�������ʱ���� FLAG
//  ���Ҫ��֤�ڵ�2�������ܶ����ߵ�ƽ��Ҳ����Ҫ��֤AD7606_2(/3)�Ĳ���ʱ�䳬����ʱ�� T�����ԶԹ���������Ҫ��
//  ��������£���T = 50us�����ܲ��õĹ�������Ϊ 100(16x) = 74us ֮��
//  ���Ըı�����������ڵ�һ�����ھ��������ߵ�ƽ

/**************************************  ʱ�򲿷�  ******************************************
 * ��ʱ�ӣ�10khz������AD7606_1��DAC8563���μ�����ϵͳ
 * ��ʱ�ӣ�500hz������������ϵͳ������ˢ�µ�
 *      ��SecondaryTask��Delay �� ���� MainTask�� 
 *          һ��ADNS9800��CS �͵�ƽ���� 500~600us����Ԥ��һ��Secondary Task���� 1.2ms
 * 
 * */
#define __CFG_MAIN_CLOCK            __CST_SAMPLE_RATE_10KHZ
#define __CFG_SECONDARY_CLOCK			 200
#define __CFG_PUT_MAIN_IN_SECONDARY	1


//����ʱ�ӵ���ʱ�ӵķ�Ƶϵ��.
#define __CST_MAIN_DIVIDE_TO_SECONDARY       (__CFG_MAIN_CLOCK/__CFG_SECONDARY_CLOCK)
//����ʱ����Ҫ��ȡ��AD7606_2,AD7606_3��������Ҫ��������(��n������ʱ���Զ�ȡ������)
#define __CST_MINIMUM_CLOCK_CYCLE_2_3        (__CFG_MAIN_CLOCK/__CFG_SECONDARY_CLOCK/2)


//***************************** DAC ���� ********************************************//
//******************* ����Ȧ�� ��Ӧ��DACģ�� �˿ڼ�CS������ ***************************//
//�����������ߵ���ʽ��ֻ��Ƭѡ�˲�ͬ������������
#define __CFG_DAC_CHANNEL_NUM    8

//******************* Grouping Method ��Ȧ���鷽ʽ ***************************//
//ֻ���Ѿ�ʵ��8·DAC����Ŀǰ��Ҫֻʵ��3���ɶ���תʱ����Ҫ���ǽ�8·���Ϊ4·
//����16����Ȧ������Ϊ8·��ֻ��Ӳ���в�������->����Ӧ������Ƶĵ�·�����ֳ���

#define __CFG_COIL_NUM   16
#define __CFG_DAC_GROUP_NUM	4
extern const uint8_t _DAC_GROUP_X_NUM[__CFG_DAC_GROUP_NUM];
extern const uint8_t __CFG_Group_of_DAC[__CFG_DAC_GROUP_NUM][2];
extern const uint8_t __CFG_DAC_number_To_coil_number[__CFG_DAC_CHANNEL_NUM][2];
extern const int8_t __CFG_Directions[__CFG_DAC_CHANNEL_NUM][2];
extern const uint8_t __CFG_ENABLE_DAC[__CFG_DAC_CHANNEL_NUM];

//**************************** ADNS9800 ���� **************************************//
//CPI
#define __CFG_ADNS9800_CPI	6350
//Lift Threshold
#define __CFG_ADNS9800_LIFT	16

//*******************************************************************************//
//**************************** ����ϵͳ���� **************************************//
//*******************************************************************************//

//**************************** ������ϵͳ **************************************//
/**
 * �����ѹ��ȷ�ȣ�(+-10V)/16bit*2�� = 0.0006, ���������վ��Ȳ�����0.001
 * ���������ȷ�ȣ�(+-5V)/16bit = 0.00015�����������վ��Ȳ�����0.001
 * �������Ͽ��ǣ���float�����㹻. �����������Ҳ������16bit���洢�ģ���float 4�ֽڣ�double 8�ֽڣ�����float�㹻
 * �ʲ��� float32_t
 * */

typedef float32_t __UNIT_MAIN_PID_DATA;
typedef __UNIT_MAIN_PID_DATA __UNIT_CURRENT_FLOAT_DATA;   //�������ʵ������������MAIN PIDһ�µ����㾫��
#define __CFG_LEVITATION_ON   0       //�������ƿ��ء�ʵ���Ͼ��� SISO PID���� = �����ά�� = ����ʵ����������ά��
#if __CFG_LEVITATION_ON
    #define __CFG_MAIN_PID_CHANNEL_NUM   6
#else
    #define __CFG_MAIN_PID_CHANNEL_NUM   3       //ע��Ҫ��__CFG_COIL_GROUP_NUM �������
#endif

//������������������
#define __CFG_DECOUPLE_EQUATION_ROWS __CFG_MAIN_PID_CHANNEL_NUM
#define __CFG_DECOUPLE_EQUATION_COLUMNS (__CFG_DAC_GROUP_NUM + 1)

//��ת���ƣ�
#define __CFG_THRESHOLD     0.2f
#define __CFG_2_THRESHOLD     0.4f
#define __CFG_REACH_THRESHOLD    0.04f

//*************** ���㲿�֣�����ת�ǵĽ��� ****************//
/**�����Լ
 * 1.������������λmm
 * 2.�Ƕ���������λrad
 * 3.����ʱ����ƣ���<math.h>���㷨��sin(3.141592)����60΢�룬sin(3.141592/2)����40΢�롣
 */
typedef int16_t __UNIT_MouseSensorINTData;
typedef float32_t __UNIT_MouseSensorFLOATData;
typedef float32_t __UNIT_RotateAngleData;
#define __CST_PI             3.141593f
#define __CST_2PI             6.283185f
#define __CST_ROTOR_RADIUS           30.0f
#define __CST_ROTOR_RADIUS_RECIPROCAL    0.03333333f
#define __CST_MOUSE_SENSOR_THETA     0.9424778f   //54degree*pi/180
#define __CST_ROTOR_RADIUS_X_MOUSE_SIN_THETA     24.27051f    //30.0*sin(54*pi/180)
#define __CST_ROTOR_RADIUS_X_MOUSE_SIN_THETA_RECIPROCAL  0.04120227f
#define __CST_COS_MOUSE_SENSOR_THETA             0.5877853f   //cos(54*pi/180)
#define __CST_MOUSE_SENSOR_SCALE_FACTOR_Y        1.030614f   //1/cos((50-(90-54))*pi/180)
#define __CST_RAD_TO_ANGLE           57.29578f                //180/pi
#define _mth_RAD_TO_ANGLE(x)        (x*__CST_RAD_TO_ANGLE)
#define _mth_abs(x)                 (x>0?x:0.0f - x)
#define _mth_sign(x)                (x>0?1:-1)
//*************** ���㲿�֣���Ȧ�ڶ�������ϵ��λ�õļ��� ****************//
#define __CST_DELTA_ANGLE_EPSILONE_DEGREE    80              //500Hz�£��ٶ�һ����������˶�����  
#define __CST_COIL_COOR_RADIUS               0.037f           //�������£�37mm
extern const __UNIT_MAIN_PID_DATA __CST_COIL_POSITION[3*__CFG_COIL_NUM];
#define __CFG_DEBOUPLE_USE_FORMULA        0   //�ڽ���4Ԫһ�η�����ʱ���Ƿ����ù�ʽ����. (�����ü��㷽��)

//**************************** �μ�����ϵͳ **************************************//
typedef __UNIT_MAIN_PID_DATA __UNIT_SUB_PID_DATA;     //�μ�����ϵͳ�����ϵͳ����һ�µ����㾫��


#define __CFG_USE_SUB_PID        1   //��ʶ�Ƿ��ӿ���ϵͳ�Ƿ�ʹ��PID������/PID�ջ����أ�
#define __CFG_USE_PRESET_PID     1   //�Ƿ�ʹ��Ԥ���PID����������ͨ���ֶ���������PID������

#define __CFG_USE_PRESET_PID_MAIN  0

//****** ���� ******//
#define __CST_COIL_R         6.0f     //�����£���Ȧ�������
#define __CST_CURRENT_MULT   2.0f     //�����£��Ŵ�������

//****** �ջ� ******//
//PID���ڼ�¼
/**
 * Kp = 2.0, Ki = 0.04      �ж���𵴣��޷��ﵽ��ֵ
 * Kp = 2.0, Ki = 0.02      ������𵴣��޷��ﵽ��ֵ
 * Kp = 2.0, Ki = 0.01      �𵴼Ӿ磬�޷��ﵽ��ֵ
 * Kp = 3.0, Ki = 0.04      �޷��ﵽ��ֵ�𵴷ǳ����ң�
 * Kp = 2.0, Ki = 0.08      ���ڴﵽ��ֵ���ж���𵴡����ⷢ�ֵ���ƺ�ȷʵ�������д�
 * Kp = 2.0, Ki = 0.1       ����10%
 * Kp = 2.5, Ki = 0.12      �����ﵽ20%���𵴽ϴ�
 * Kp = 1.8, Ki = 0.1       ����8%
 * Kp = 2.5, Ki = 0.08      ����30%
 * Kp = 1.5, Ki = 0.07      �ﲻ����ֵ�����ӽ�
 * Kp = 1.5, Ki = 0.1       ����8%���ң������Լ��ᣬֻ��2��
 * Kp = 1.2, Ki = 0.1       ����5%����ֻ��1��
 * Kp = 1.1 -> 0.9 -> 0.6, Ki = 0.1    �����𽥱��
 *^Kp = 1.2, Ki = 0.09      �޳��������𵴣���Ӧʱ��200us
 * Kp = 1.2, Ki = 0.1 -> 0.09 -> 0.085 �����𽥼�С
 * Kp = 1.2, Ki = 0.085 -> 0.082 -> 0.08      ���𵴣����տ�ʼ�ﲻ����ֵ�пӣ�Խ��Խ��
 * Kp = 1.5, Ki = 0.08      �տ�ʼ�ﲻ����ֵ���𵴻��Ӿ���
 * 
 * ���ϣ�Kp = 1.2ʱ�ܴﵽ��ֵ�һ���û����; Kp ������С���������
 * */

//Ki = 0.2ʱ�ж����
#define __PAR_SUB_PID_KP     1.3f
#define __PAR_SUB_PID_KI     0.8f    //Ki = xe5 (��ΪTs = 5e-5,�ʵ���)
#define __PAR_SUB_PID_KD     0.4f

//#define __PAR_SUB_PID_KI     0.06    //Ki = xe5 (��ΪTs = 5e-5,�ʵ���)
//#define __PAR_SUB_PID_KP     10

#define __CFG_SUB_PID_CHANNEL_NUM __CFG_DAC_CHANNEL_NUM
#define __CFG_SUB_PID_COIL_CURRENT_MAGNITUDE     1.5f         //��Ȧ�������ȡ�
#define __CFG_SUB_PID_OUTPUT_VOLTATE_MAGNITUDE   7.5f         //�Ŵ�ǰ�����ѹ�ĸ�ֵ

#define __CST_SUB_PID_TS     5       //Ts = 5e-5
#define __CST_SUB_PID_OUTPUT_MAX     10.0f

//**************************** ʱ���鲿�� **************************************//
#define __TIME_MAIN_CHECK_COUNT       __CFG_MAIN_CLOCK
#define __TIME_SECONDARY_CHECK_COUNT  __CFG_SECONDARY_CLOCK
#define __TIME_SECONDARY_TO_DEBUG     __CFG_SECONDARY_CLOCK/2

//**************************** ����ͨ�Ų��� **************************************//
#define __CFG_USART_BUFFER_MAX  1024
#define __CFG_USART_RECEIVE_END_FLAG '\r'
#define __CFG_USART_RECEIVE_END_FLAG_2 '\n'

//**************************** ����װ�ò��� **************************************//
#define __CFG_USE_ZERO_DEVICE  1
#endif

/**** Copyright (C)2018 Ventus Guo. All Rights Reserved **** END OF FILE ****/
