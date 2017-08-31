/*
 * private.h --
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
 *     * Neither the name of the <organization> nor the
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

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

/** Global includes */
#include <config.h>
/** Other includes */
#include <mml.h>
#include <cobra_defines.h>
#include <cobra_functions.h>
/** Local includes */

/** Maxim system frequency for platform */
#define	K_OTP_DUMP_FREQ_SYSTEM_MAX				MML_SYS_FREQ

#define	K_MML_IFLASH_PATTERN_SIZE_BYTE			K_COBRA_ONE_KB
#define	K_MML_IFLASH_PATTERN_SIZE_INT			( K_MML_IFLASH_PATTERN_SIZE_BYTE / sizeof(unsigned int) )

#define	K_OTP_DUMP_BASE_OFFSET					0x0
#define	K_OTP_DUMP_WORK_SIZE_BYTE				( 1 * K_COBRA_ONE_KB )
#define	K_OTP_DUMP_WORK_SIZE_INT				( K_OTP_DUMP_WORK_SIZE_BYTE / sizeof(unsigned int) )

#define	K_OTP_DUMP_WORKBUFFER_SIZE_BYTE			( 1 * K_COBRA_ONE_KB )
#define	K_OTP_DUMP_WORKBUFFER_SIZE_INT			( K_OTP_DUMP_WORKBUFFER_SIZE_BYTE / sizeof(unsigned int) )

#ifdef _OTP_DUMP_MAXIM_
#define	K_OTP_DUMP_BASE_ADDRESS					MML_MEM_OTP_MXIM_BASE
#define	K_OTP_DUMP_MAX_SIZE_BYTE				MML_MEM_OTP_MXIM_SIZE
#else
#define	K_OTP_DUMP_BASE_ADDRESS					MML_MEM_OTP_USER_BASE
#define	K_OTP_DUMP_MAX_SIZE_BYTE				MML_MEM_OTP_USER_SIZE
#endif /* _OTP_DUMP_MAXIM_ */
#define	K_OTP_DUMP_MAX_SIZE_INT					( K_OTP_DUMP_MAX_SIZE_BYTE / sizeof(unsigned int) )

/** Size of OTP line in bytes */
#define	K_OTP_DUMP_OTP_LINE_SIZE				0x08

/** Size of truncated CV into OTP line rounded to upper bytes value */
#define	K_OTP_DUMP_OTP_CV_LINE_SIZE				0x02

/******************************************************************************/
int uart_write_char(const char c);
void print_USN(void);
void memcpy_int(unsigned int *p_dst,
				unsigned int *p_src,
				unsigned int size);
int iflash_dump_init(void);
int config_uart(void);
 #ifdef _WITH_REALIGNMENT_
void otp_dump_realign_data(unsigned char *p_src,
							unsigned length);
 #endif /* _WITH_REALIGNMENT_ */
int iflash_dump_display(unsigned int address,
						unsigned int length);
#endif /* _PRIVATE_H_ */
/******************************************************************************/
/* EOF */
