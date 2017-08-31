/*
 * private.c --
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
 * Created on: Jul 07, 2014
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
#include <mml.h>
#include <mml_uart.h>
#include <mml_intc.h>
#include <mml_intc_regs.h>
#include <mml_sflc.h>
#include <mml_sflc_regs.h>
/** Local includes */
#include <printf_lite.h>
#include <private.h>
#include <uart_config.h>


unsigned char work_buffer[K_OTP_DUMP_WORKBUFFER_SIZE_BYTE];
/******************************************************************************/
int uart_write_char(const char c)
{
	return mml_uart_write_char(MML_UART_DEV0, c);
}

/******************************************************************************/
void memcpy_int(unsigned int *p_dst, unsigned int *p_src, unsigned int size)
{
	register unsigned int						i;

	for( i = 0;i < size;i++ )
	{
		p_dst[i] = p_src[i];
	}
	/** We're done */
	return;
}

/******************************************************************************/
int iflash_dump_init(void)
{
	int											result = COMMON_ERR_UNKNOWN;

	/** Call SFLC initialization*/
	mml_sflc_init();
	/** Initialize UART0 port with default configurations */
	result = config_uart();
	/** We're done */
	return result;
}

/******************************************************************************/
int config_uart(void)
{
	mml_uart_config_t							uart_conf;

	uart_conf.baudrate = K_LITE_UART0_DEFAULT_BAUDRATE;
	uart_conf.data_bits = MML_UART_DATA_TRANSFER_SIZE_8_BITS;
	uart_conf.flwctrl = MML_UART_HW_FLOW_CTL_DISABLE;
	uart_conf.parity = MML_UART_PARITY_NONE;
	uart_conf.parity_mode = MML_UART_PARITY_MODE_ONES;
	uart_conf.rts_ctl = MML_UART_RTS_IO_LEVEL_LOW;
	uart_conf.stop_bits = MML_UART_STOPBITS_ONE;
	uart_conf.handler = (mml_uart_handler_t)config_uart;
	/** We're done */
	return  mml_uart_init(MML_UART_DEV0, uart_conf);
}

/******************************************************************************/
/** This function re-aligns data in work buffer by removing CV and lock bit */
void otp_dump_realign_data(unsigned char *p_src, unsigned int length)
{
	register unsigned int						j;
	unsigned int								i;
	unsigned char								*p_recover = p_src;
	unsigned char								*p_tmp = p_src;

	/** Loop on full length OTP line number */
	for( i = 0;i < ( length / K_OTP_DUMP_OTP_LINE_SIZE );i++ )
	{
		/** Shift data - CVs removed, bit lock removed */
		for( j = 0;j < ( K_OTP_DUMP_OTP_LINE_SIZE - K_OTP_DUMP_OTP_CV_LINE_SIZE );j++ )
		{
			p_recover[j] = ( ( p_tmp[j + K_OTP_DUMP_OTP_CV_LINE_SIZE] << 1 ) & 0xfe ) | ( ( p_tmp[j + ( K_OTP_DUMP_OTP_CV_LINE_SIZE - 1 )] >> 7 ) & 0x01 );
		}
		/** Increment pointer */
		p_tmp += K_OTP_DUMP_OTP_LINE_SIZE;
		p_recover += ( K_OTP_DUMP_OTP_LINE_SIZE - K_OTP_DUMP_OTP_CV_LINE_SIZE );
	}
	/** We're done */
	return;
}

/******************************************************************************/
int iflash_dump_display(unsigned int address, unsigned int length)
{
	int											result = COMMON_ERR_UNKNOWN;
	unsigned int								last_size = length;
	unsigned int								i;


	/** Check input parameters */
	if ( !address )
	{
		/** Null pointer */
		result = COMMON_ERR_NULL_PTR;
	}
	else
	{
		volatile unsigned int					addr_src = (unsigned int)address;
		volatile unsigned char					*p_dst = (unsigned char*)work_buffer;

		/** Looping :) */
		while( last_size )
		{
			unsigned int						size = M_COBRA_MIN(last_size,sizeof(work_buffer));

			/** Initialize work buffer */
			memset(work_buffer, 0x00, sizeof(work_buffer));
			/** Copy buffer to display into work buffer */
#ifdef _OTP_DUMP_MAXIM_
			result = mml_sflc_otp_read(N_MML_SFLC_OTP_ID_MAXIM, addr_src, (unsigned char*)p_dst, size);
#else
			result = mml_sflc_otp_read(N_MML_SFLC_OTP_ID_USER, addr_src, (unsigned char*)p_dst, size);
#endif /* _OTP_DUMP_MAXIM_ */
#ifdef _WITH_REALIGNMENT_
			/** Realign data */
			otp_dump_realign_data((unsigned char*)p_dst, size);
#endif /* _WITH_REALIGNMENT_ */
			/** Display work buffer */
			for( i = 0;i < size;i++ )
			{
				/**  */
				if ( !( i % 8 ) )
				{
					/**  */
					lite_printf("\n0x%08x:", ( addr_src + i ));
				}
				/**  */
				lite_printf(" %02x", p_dst[i]);
			}
			/** Update lasting size */
			last_size -= size;
			/** Update work pointer */
			addr_src += size;
		}
		/**  */
		lite_printf("\n");
		/** No error */
		result = NO_ERROR;
	}
	/** We're done */
	return result;
}

void compute_USN64bit(unsigned int USNR1,unsigned int USNR2){

					unsigned int USN1 = ( USNR1 >> 15 );
					unsigned int USN2 = ( USNR2 & 0x7FFFFFFF );


					unsigned char US1 = USN1 & 0xFF ;
					unsigned char US2 = ((USN1 >> 8 ) & 0xFF );
					unsigned char US3 = ((USN2 << 0x1 ) & 0xFE ) + (( USN1 >> 16 ) & 0x1 );
					unsigned char US4 = ((USN2 >> 0x7 ) & 0xFF );
					unsigned char US5 = ((USN2 >> 15 ) & 0xFF );
					unsigned char US6 = ((USN2 >> 23 ) & 0xFF );

					lite_printf("%02x ", US1);
					lite_printf("%02x ", US2);
					lite_printf("%02x ", US3);
					lite_printf("%02x ", US4);
					lite_printf("%02x ", US5);
					lite_printf("%02x ", US6);

}


void print_USN(void){

	unsigned int otpdata[8];
	mml_sflc_otp_read(N_MML_SFLC_OTP_ID_MAXIM, MML_MEM_OTP_MXIM_BASE, (unsigned char*)otpdata, 32);
	lite_printf("\nUSN : ");
	compute_USN64bit(otpdata[0],otpdata[1]);
	compute_USN64bit(otpdata[2],otpdata[3]);
	unsigned char US13 = ( otpdata[4] >> 15 ) & 0xFF ;
	lite_printf("%02x\n", US13);

}

/******************************************************************************/
/* EOF */
