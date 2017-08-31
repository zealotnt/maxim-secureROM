/*
 * main.c --
 *
 * ----------------------------------------------------------------------------
 * Copyright (c) 2014, Maxim Integrated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Maxim Integrated nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* [INTERNAL] ------------------------------------------------------------------
 * Created on: Feb 11, 2014
 * Author: Yann G. (yann.gaude@maximintegrated.com)
 *
 * ---- Subversion keywords (need to set the keyword property)
 * $Revision:: $: Revision of last commit
 * $Author:: $: Author of last commit
 * $Date:: $: Date of last commit
 * [/INTERNAL] -------------------------------------------------------------- */

/** Global includes */
#include <config.h>
#include <errors.h>
/** Other includes */
#include <cobra_defines.h>
#include <cobra_macros.h>
#include <cobra_functions.h>
#include <mml_gcr_regs.h>
#include <mml_gcr.h>
#include <mml_sflc.h>
#include <mml_uart.h>
/** Local includes */
#include <uart_config.h>
#include <printf_lite.h>
#include <private.h>


/******************************************************************************/
int main(void)
{
    int											result = COMMON_ERR_UNKNOWN;

    /** Set the system frequency */
    result = mml_set_system_divider(MML_GCR_DIV_1);
    if ( result )
    {
		/** Oops, I did it again ... */
		goto main_out;
    }
    /** Reset interfaces */
    result = iflash_dump_init();
    if ( result )
    {
    	/** Oops, I did it again ... */
    	goto main_out;
    }
    /** Welcome message */
    lite_printf("Welcome to OTP Dump Application !\n");
    lite_printf("\nProject version: ");
    lite_printf(VERSION);
    lite_printf(" uses COBRA SDK modules versions :\n");
    lite_printf("\n\tBUILD_");
    lite_printf(TOOLS_VERSION);
    lite_printf("\n\tINIT_");
    lite_printf(INIT_VERSION);
    lite_printf("\n\tLIB_");
    lite_printf(LIB_VERSION);
    lite_printf("\n\tLINK_");
    lite_printf(LINK_VERSION);
    lite_printf("\n\tINCLUDE_");
    lite_printf(INC_VERSION);
    lite_printf("\n");
		print_USN();
    /** Now display programmed pattern */
    result = iflash_dump_display(K_OTP_DUMP_BASE_ADDRESS, K_OTP_DUMP_MAX_SIZE_BYTE);
    if ( result )
    {
    	/** Error while dumping internal flash */
    	lite_printf("\n\t[OTP Dump] Dumping OTP failed - 0x%08x\n", result);
    }
    else
    {
    	lite_printf("\n\t[OTP Dump] Dumping done.\n");
    }
    /** We're done */
main_out:
    return result;
}

/******************************************************************************/
/* EOF */
