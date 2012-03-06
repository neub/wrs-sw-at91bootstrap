/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 * ----------------------------------------------------------------------------
 * Copyright (c) 2006, Atmel Corporation

 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 * File Name           : main.c
 * Object              :
 * Creation            : ODi Apr 19th 2006
 *-----------------------------------------------------------------------------
 */
#include <pp_printf.h>
#include "part.h"
#include "main.h"
#include "dbgu.h"
#include "debug.h"
#include "dataflash.h"
#include "nandflash.h"
#include "flash.h"
#ifdef CONFIG_USER_HW_INIT
void user_hw_init(void);
#endif

extern void Jump(unsigned int addr);    // Function import from startup.s file

extern unsigned int load_SDCard();

void sclk_enable(void);

void LoadLinux();

void LoadWince();

/*------------------------------------------------------------------------------*/
/* Function Name       : Wait							*/
/* Object              : software loop waiting function				*/
/*------------------------------------------------------------------------------*/
#ifdef WINCE
void Wait(unsigned int count)
{
    volatile unsigned int i;
    volatile unsigned int j = 0;

    for (i = 0; i < count; i++)
        j++;
}
#else
void Wait(unsigned int count)
{
    unsigned int i;

    for (i = 0; i < count; i++)
        asm volatile ("    nop");
}
#endif

/*------------------------------------------------------------------------------*/
/* Function Name       : main							*/
/* Object              : Main function						*/
/* Input Parameters    : none							*/
/* Output Parameters   : True							*/
/*------------------------------------------------------------------------------*/
int main(void)
{
    extern const char build_time[];
    extern const char git_user[];
    extern const char git_revision[];
   
	extern void mem_test(unsigned long ini, unsigned long end);
  
    /*
     * ================== 1st step: Hardware Initialization ================= 
     *
     * Performs the hardware initialization 
     */
#ifdef CONFIG_HW_INIT
    hw_init();
#endif

    pp_printf("Compiled by %s (%s)\r\ngit rev:%s\r\n\r\n",git_user,build_time,git_revision);
    
#ifdef CONFIG_USER_HW_INIT
    user_hw_init();
#endif

#if defined(CONFIG_AT91SAM9X5EK)
    extern void load_1wire_info();
    load_1wire_info();
#endif

    pp_printf("Running application at %p\n", mem_test);
    mem_test(0x70000000,0x74000000);
    return 0; /* not reached */
}
