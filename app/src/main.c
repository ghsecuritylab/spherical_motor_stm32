/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 * 2011-06-05     Bernard      modify for STM32F107 version
 */

#include <rthw.h>
#include <rtthread.h>
#include <board.h>
/**
 * @addtogroup STM32
 */

/*@{*/

void show_sysclock_config(void)
{
	RCC_ClocksTypeDef  rcc_clocks;
	RCC_GetClocksFreq(&rcc_clocks);
	rt_kprintf("\n[Sysclock Config]:\n");
	rt_kprintf("fHCLK = %d, fMAIN = %d, fPCLK1 = %d, fPCLK2 = %d. \n",rcc_clocks.HCLK_Frequency, rcc_clocks.SYSCLK_Frequency, rcc_clocks.PCLK1_Frequency, rcc_clocks.PCLK2_Frequency);
}

/* log trace test */
#include <log_trace.h>
#include <string.h>
const static struct log_trace_session _main_session = 
{
	.id = {.name = "main"},
	.lvl = LOG_TRACE_LEVEL_DEBUG,
};

int main(void)
{
	//wait system init.
	rt_thread_delay(DELAY_10MS(10));
	show_sysclock_config();
    /* user app entry */

	/* log trace test */
	log_trace_set_device("uart2");
	log_trace_register_session(&_main_session);
	while(1){
		log_session(&_main_session, "testing...\n",LOG_TRACE_LEVEL_INFO);
		rt_thread_delay(DELAY_S(5));
	}
	
    return 0;
}

/*@}*/
