// ----------------------------------------------------------------------------
// Copyright (c) 2009-2011, Maxim Integrated Products
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED PRODUCTS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED PRODUCTS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------
//
// Created on: Jan,2011
// Author:
//
// ---- Subversion keywords (need to set the keyword property)
// $Rev::               $:  Revision of last commit
// $Author::            $:  Author of last commit
// $Date::              $:  Date of last commit
//
// CRK public key signature

// this tool creates signed applications from applications,
//using the FLORA secure encapsulation format defined in SDS22S01

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <defs.h>
#include <string.h>


#ifdef WIN32
#define __API__
#endif

#ifdef __cplusplus
extern "C"
{
#endif


#include <ucl/ucl_data_conv.h>
#include <ucl/ucl_stack.h>

#include <ucl/ucl_pkcs1_mgf1_sha1.h>
#include <ucl/ucl_sha1.h>

#include <ucl/ucl_rng.h>
#include <ucl/ucl_pkcs1_mgf1_sha1.h>
#include <ucl/ucl_config.h>
#include <ucl/ucl_defs.h>
#include <ucl/ucl_retdefs.h>
#include <ucl/ucl_types.h>

#include <ucl/ucl_sys.h>
#include <ucl/ucl_trng.h>
#include <ucl/ucl_des_ecb.h>
#include <ucl/ucl_3des_ecb.h>
#include <ucl/ucl_des_cbc.h>
#include <ucl/ucl_des_cbc_mac.h>
#include <ucl/ucl_tdes_cbc_mac.h>
#include <ucl/ucl_3des_cbc.h>
#include <ucl/ucl_des_ofb.h>
#include <ucl/ucl_3des_ofb.h>
#include <ucl/ucl_des_cfb.h>
#include <ucl/ucl_3des_cfb.h>
#include <ucl/ucl_aes.h>
#include <ucl/ucl_uaes.h>
#include <ucl/ucl_uaes_ecb.h>
#include <ucl/ucl_uaes_cbc.h>
#include <ucl/ucl_rsa.h>
#include <ucl/ucl_sha1.h>
#include <ucl/ucl_sha256.h>
#include <ucl/ucl_pkcs1_ssa_pss_sha256.h>
#ifndef _MXIM_HSM
#include <ucl/ecdsa_generic_api.h>
#endif

#ifdef __cplusplus
}
#endif

//#define MAJV 1
//#define MINV 5
//#define ZVER 5
//#define BUILD 1
//1001 initial version, from crk_sign
//1101 FA header new options added
//1201 FA header UCI options finalized
//1211 SR_PRFSH reduced to 2 bytes (16bits) as in the spec
//1221 allocation size error for payload (forgotten strlen(arguments)), which led to output wrong bytes (dhb report)
//1231 sdr_* parameters renamed into sdr_*
//1241 warnings removed; file dump under verbose=true only
//1251 version (4bytes) and SR_PEXT (1byte) fields added #2262
//1261 dynamic memory slot parameters names and lengths changed to match with flora wiki binary header format
//1271 uci encryption and integrity support, including source information (#2282 and flora wiki binary header format)
//1281 ky1r removed
//1.3.0 build 4 display RSA key name in case of ncipher session; correct a bug in displaying RSA key name in case of ncipher session
//1.4.0 correct the way to read binary file and get its size (fscanf_s shall be avoided use fread is better). now ca_sign_build.exe (with hsm) and ncipher_ca_sign_build.exe (without hsm) give the same result.
/*Adding code to manage both rsa key file format:
-1- modulus
-2- public exponent
-3- private exponent
or
-1- modulus
-2- private exponent
-3- public exponent
correct the way to read binary file and get its size (fscanf_s shall be avoided use fread is better).
now ca_sign_build.exe (with hsm) and ncipher_ca_sign_build.exe (without hsm) give the same result.
resource file: version string
add a cast for c++ build
*/
//1.5.0: support of angela/ecdsa
//1.5.1: minor changes
//1.5.2: 64-bit
//1.5.3.0: HSM modification call to UCL lib
//1.5.4: #4539 .sig generation (new parameter added in the .ini: signonly=yes|no
//1.5.5: secure header generation becomes an option managed by the header parameter
#include "ca_sign_build.h"

//  DR - 10/04/2014 : REMOVE ALL DEFINITIONS IN HEADER FILES
#ifndef _MXIM_HSM
#define INIFILE "ca_sign_build.ini"

#define ECDSA_MODULUS_LEN SECP256R1_BYTESIZE
#define RSA_BYTE_LEN 256
#define AES_BYTE_LEN 16
#define EXP_BYTE_LEN 4
#define CV_BYTE_LEN 4
#define HEADER_SYNC_LEN 8
//1.2.5
#define HEADER_VERSION_LEN 4
//for angela
#define HEADER_APPLICATION_VERSION_LEN 4
#define HEADER_LOAD_ADDRESS_LEN 4
#define HEADER_JUMP_ADDRESS_LEN 4
#define HEADER_BINARY_LEN 4
#define HEADER_ARGV_LEN 4
//#define HEADER_GENERAL_INFO_LEN 1
//1.2.6
//SDR renamed in SR
//lengths changed to latest flora wiki binary header format
#define HEADER_SR_PAPD_LEN 1
//1.2.6
#define HEADER_SR_PRFSH_LEN 4
#define HEADER_SR_PCFG_LEN 4
//1.2.5
#define HEADER_SR_PEXT_LEN 1
//1.2.6
#define HEADER_DMC_GCFG_LEN 4
//1.2.6
#define HEADER_DMC_CLK_LEN 1
//1.2.7
#define HEADER_UCI_KSRC_CONFIGENCINT_LEN 1
#define HEADER_UCI0_AC1R_START_OFFSET_LEN 4
#define HEADER_UCI0_AC1R_END_OFFSET_LEN 4
#define HEADER_UCI0_DDR_R0_LEN 4
//#define HEADER_CTRL_REG_LEN 4
//1.2.8
//#define HEADER_KY1R_LEN 16
//1.2.5: sr_pext and version added
//1.2.7: ksrc renamed
//1.2.8: ky1r removed
#define FLORA_HEADER_LEN HEADER_SYNC_LEN+HEADER_VERSION_LEN+HEADER_LOAD_ADDRESS_LEN+HEADER_JUMP_ADDRESS_LEN+HEADER_BINARY_LEN+HEADER_ARGV_LEN+/*HEADER_GENERAL_INFO_LEN*/+HEADER_SR_PAPD_LEN+HEADER_SR_PRFSH_LEN+HEADER_SR_PCFG_LEN+HEADER_SR_PEXT_LEN+HEADER_DMC_GCFG_LEN+HEADER_DMC_CLK_LEN+HEADER_UCI_KSRC_CONFIGENCINT_LEN+HEADER_UCI0_AC1R_START_OFFSET_LEN+HEADER_UCI0_AC1R_END_OFFSET_LEN+HEADER_UCI0_DDR_R0_LEN
#define ANGELA_HEADER_LEN HEADER_SYNC_LEN+HEADER_VERSION_LEN+HEADER_LOAD_ADDRESS_LEN+HEADER_JUMP_ADDRESS_LEN+HEADER_BINARY_LEN+HEADER_ARGV_LEN+HEADER_APPLICATION_VERSION_LEN
#define MAX_HEADER_LEN FLORA_HEADER_LEN

#define BL_MAGIC_NUMBER 0xCAFEFADE
#define BS_MAGIC_NUMBER 0xCAFEFADE

#define MAJOR_VERSION_IDENTIFIER_SIZE 4
#define ANNEX_IDENTIFIER_SIZE 260
#define DATA_SIZE 4
#define FLORA_SIGNATURE_LEN RSA_BYTE_LEN
#define ANGELA_SIGNATURE_LEN ECDSA_MODULUS_LEN*2

#define BL_MAGIC_NUMBER_ADDRESS  0xa1010000
#define BL_RSA_SIGNATURE_ADDRESS            0xa1010004
#define BL_MAJOR_VERSION_IDENTIFIER_ADDRESS 0xa1010104
#define BL_ANNEX_IDENTIFIER_ADDRESS 0xa1010108
#define BL_DATA_SIZE_ADDRESS     0xa101020c
#define BL_DATA_STARTING_ADDRESS 0xa1010210

#define BS_MAGIC_NUMBER_ADDRESS  0xa100fff0
#define BS_RSA_SIGNATURE_ADDRESS 0xa100fef0
#define BS_MAJOR_VERSION_IDENTIFIER_ADDRESS 0xa100fde0
#define BS_ANNEX_IDENTIFIER_ADDRESS 0xa100fde4
#define BS_DATA_SIZE_ADDRESS 0xa100020c
#define BS_DATA_STARTING_ADDRESS 0xa1000000

#define INTERNAL_FLASH 0
#define EXTERNAL_FLASH_SPANSION_S29 1
#define EXTERNAL_FLASH_STRATA_P30 2

#define INTERNAL_FLASH_START_ADDRESS 0xA1000000
#define INTERNAL_FLASH_SIZE 256*1024
#define INTERNAL_FLASH_SECTOR0_SIZE 32*1024
#define INTERNAL_FLASH_SECTOR1_SIZE 32*1024
#define EXTERNAL_FLASH_START_ADDRESS 0x00000000
#define MAXFLASH 3
#define MAXSECTORS 500

#define TRUE 1
#define FALSE 0
#define SBL 0
#define SCP_ON_AES 1
#define SCP_OFF_AES 2
#define SCP_RSA 3
#define HOST 0
#define USIP 1
#define MAX_FRAME 10000
#define MAX_IDF 400
#define MAX_STRING 10240
#define MAXLINE 2000
#define MAX_TAB 100
#define MAX_PACKET 10240
#define MAX_FLASH 32*1024*1024
#define CHUNK_SIZE 1024
#define _MXIM_ERROR 1
#define OK 0
#define YES 1
#define NOT 0

#define SYNCH1          0xBE
#define SYNCH2          0xEF
#define SYNCH3          0xED

#define CON_REQ   0x01  //Connection request
#define CON_REP         0x02  //Connection reply
#define CON_REF   0x09  //Connection refused
#define DISC_REQ  0x03  //Disconnection request
#define DISC_REP  0x04  //Disconnection reply
#define DATA_TRANSFER 0x05  //Data Exchange
#define ACK         0x06  //Acknowledge
#define ECHO_REQ  0x0B  //Echo request
#define ECHO_REP  0x0C  //Echo reply
#define CHG_SP_0  0x07  //Change speed to 57600 b/s
#define CHG_SP_1  0x17  //Change speed to 115200 b/s
#define CHG_SP_2  0x27  //Change speed to 230400 b/s
#define CHG_SP_3  0x37  //Change speed to 345600 b/s
#define CHG_SP_4  0x47  //Change speed to 460800 b/s
#define CHG_SP_5  0x57  //Change speed to 576000 b/s
#define CHG_SP_6  0x67  //Change speed to 691200 b/s
#define CHG_SP_7  0x77  //Change speed to 806400 b/s
#define CHG_SP_8  0x87  //Change speed to 921600 b/s
#define CHG_SP_REP  0x08  //Change speed reply

#define WRITE_KEY 0xB2
#define WRITE_BPK       0xB3
#define WRITE_TIMEOUT 0x3A
#define UPDATE_LIFE_CYCLE   0xDA
#define UPDATE_6A   0xDB
#define WRITE_FLASH 0xC3
#define ERASE_FLASH 0xD4
#define BLANK_CHECK_FLASH   0xE5
#define VERIFY_FLASH  0x08
#define SIGNATURE_CHECK 0xF7
#define SIGN_WRITE_FLASH    0xF9
#define LOCK_FLASH    0xD6
#define WRITE_PROCEDURE 0x73
#define REGISTER_PROCEDURE  0x75
#define CALL_PROCEDURE  0x74
#define ERROR_NO  0x00
#define ERROR_INVAL 0xEA
#define ERROR_ALREADY 0x137

#define BL_MAGIC_NUMBER_ADDRESS  0xa1010000
#define BL_RSA_SIGNATURE_ADDRESS            0xa1010004
#define BL_MAJOR_VERSION_IDENTIFIER_ADDRESS 0xa1010104
#define BL_ANNEX_IDENTIFIER_ADDRESS 0xa1010108
#define BL_DATA_SIZE_ADDRESS     0xa101020c
#define BL_DATA_STARTING_ADDRESS 0xa1010210

#define BS_MAGIC_NUMBER_ADDRESS  0xa100fff0
#define BS_RSA_SIGNATURE_ADDRESS 0xa100fef0
#define BS_MAJOR_VERSION_IDENTIFIER_ADDRESS 0xa100fde0
#define BS_ANNEX_IDENTIFIER_ADDRESS 0xa100fde4
#define BS_DATA_SIZE_ADDRESS 0xa100020c
#define BS_DATA_STARTING_ADDRESS 0xa1000000

#define READ_CONF 0x4203
#define ERASE_MEM 0x4401
#define WRITE_MEM 0x2402
#define VERIFY_MEM 0x2403
#define WRITE_BLPK 0x4702
#define WRITE_FAK 0x2703
#define AES_COMP 0x2701
#define MEM_MAPPING 0x2404
#define WRITE_CONF 0x4202

#define USER_FLASH_CHECK 0x02
#define RESERVED_SECTOR_CHECK 0x03

#define HELLO_REQ       0x1
#define HELLO_REP 0x2
#define HELLO_OFF_REQ   0x8
#define HELLO_OFF_REP   0x9
#define CHALLENGE 0x7
#define SUCCESS         0x3
#define FAILURE         0x4
#define DATA          0x5

#define DATA_CHECKSUM_LEN 4

#define RANDOM_LEN 16
#define SUCCESS_LEN 0
#define FAILURE_LEN 0
#define SECTOR_MAX 35
#define SECTOR_SIZE 4096
#define PP_CLEAR 0
#define PP_RMAC 1
#define PP_E_RMAC 2
#define PP_CMAC 5
#define PP_E_CMAC 6
#define PP_RSA 9

#define S19_ADDRESS_LEN 4
#define S19_ADDRESS_START 4
#define S19_DATA_START S19_ADDRESS_START+S19_ADDRESS_LEN*2
#define S19_CRC_LEN 1
#define S19_WDATA_CHAR1 'S'
#define S19_WDATA_CHAR2 '3'
#define S19_LINE_LEN_POS1 2
#define S19_LINE_LEN_POS2 3

#endif //--  #ifndef _MXIM_HSM

typedef struct _type_config
{
    u8 rsa[MAX_STRING];
    u8 rsa_privexp[MAX_STRING];
    u8 rsa_pubexp[MAX_STRING];
    int rsa_len;
    int rsa_explen;
    int ecdsa_len;
    u8 ecdsa_privkey[ECDSA_MODULUS_LEN];
    u8 ecdsa_pubkey_x[ECDSA_MODULUS_LEN];
    u8 ecdsa_pubkey_y[ECDSA_MODULUS_LEN];
    u8 already_diversified;
    u8 pp;
} type_config_struct;

type_config_struct config_struct;



u8 signature[UCL_RSA_KEY_MAXSIZE];
char load_address_string[MAX_STRING];
char jump_address_string[MAX_STRING];
char arguments[MAX_STRING];
//u8 general_info;
u8 sr_papd;
u32 sr_prfsh;
u32 sr_pcfg;
u32 dmc_gcfg;
//1.2.6
u8 dmc_clk;
u32 uci2_ctrl_reg;
//1.2.7: ksrc renamed
u8 uci0_ksrc_configencint;
u32 uci0_ac1r_so;
u32 uci0_ac1r_eo;
u32 uci0_ddr_r0;

//1.2.8: ky1r removed
//u8  ky1r[16];
//1.2.5
u8 sr_pext;
u32 version;
//angela parameter
u32 application_version;
int header_len;

char algo[MAX_STRING];
u32 init_buffer[2048];
char *source[2] = {"host", "bl"};





char idf_ctl[MAX_IDF][MAX_STRING];
int list_ctl[MAX_IDF];
char idf_keyuse[MAX_IDF][MAX_STRING];
int list_keyuse[MAX_IDF];
char idf_cmd[MAX_IDF][MAX_STRING];
int list_cmd[MAX_IDF];
char idf_app_cmd[MAX_IDF][MAX_STRING];
int list_app_cmd[MAX_IDF];
char idf_pp[MAX_IDF][MAX_STRING];
int list_pp[MAX_IDF];
int list_pp_ic400d[MAX_IDF];
char idf_fai[MAX_IDF][MAX_STRING];
int list_fai[MAX_IDF];

u8 blsignpuk[RSA_BYTE_LEN];
u8 blsignprivexp[RSA_BYTE_LEN];
u8 blsignpukexp[EXP_BYTE_LEN];
int seq, ch_id, tr_id;
u8 frame[MAX_FRAME];
u8 random_number[UCL_AES_BLOCKSIZE];
u8 response[UCL_AES_BLOCKSIZE];
int iframe;
u8 who;
int start_addr;
int end_addr;
u8 data[MAX_FLASH];
u8 *payload;
u32 payload_len = 0;
int data_len;
int addr[MAX_FLASH];
int found;
char message[MAX_STRING];
char cafile[MAX_STRING];
char scafile[MAX_STRING];
//1.5.4 #4539
char sigfile[MAX_STRING];
char rsafile[MAX_STRING];
char ecdsafile[MAX_STRING];
int compteur;
u8 verbose;
//1.5.4 #4539
u8 signonly;
u8 headergenerated;
u8 bootloader;
int offset_output;
FILE *fp;

int test_hex(char c1, char c2)
{
    int value;
    int ok1, ok2;
    value = 0;
    ok1 = ok2 = 0;
    if (c1 >= 'A' && c1 <= 'F')
    {
        value = (c1 - 'A' + 10);
        ok1 = 1;
    }
    if (c1 >= 'a' && c1 <= 'f')
    {
        value = (c1 - 'a' + 10);
        ok1 = 1;
    }
    if (c1 >= '0' && c1 <= '9')
    {
        value = (c1 - '0');
        ok1 = 1;
    }
    value *= 16;
    if (c2 >= 'A' && c2 <= 'F')
    {
        value += (c2 - 'A' + 10);
        ok2 = 1;
    }
    if (c2 >= 'a' && c2 <= 'f')
    {
        value += (c2 - 'a' + 10);
        ok2 = 1;
    }
    if (c2 >= '0' && c2 <= '9')
    {
        value += (c2 - '0');
        ok2 = 1;
    }
    if (ok1 == 0 || ok2 == 0)
        return (EXIT_FAILURE);
    return (EXIT_SUCCESS);
}

int hex(char c1, char c2)
{
    int value;
    value = 0;
    if (c1 >= 'A' && c1 <= 'F')
        value = (c1 - 'A' + 10);
    if (c1 >= 'a' && c1 <= 'f')
        value = (c1 - 'a' + 10);
    if (c1 >= '0' && c1 <= '9')
        value = (c1 - '0');
    value *= 16;
    if (c2 >= 'A' && c2 <= 'F')
        value += (c2 - 'A' + 10);
    if (c2 >= 'a' && c2 <= 'f')
        value += (c2 - 'a' + 10);
    if (c2 >= '0' && c2 <= '9')
        value += (c2 - '0');
    return (value);
}


int ecdsa_sign_payload(u8 *signature, u8 *payload, u32 payload_len)
{
    u8 a[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
    //  u8 b[]={0x5A,0xC6,0x35,0xD8,0xAA,0x3A,0x93,0xE7,0xB3,0xEB,0xBD,0x55,0x76,0x98,0x86,0xBC,0x65,0x1D,0x06,0xB0,0xCC,0x53,0xB0,0xF6,0x3B,0xCE,0x3C,0x3E,0x27,0xD2,0x60,0x4B};
    u8 xg[] = {0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96};
    u8 yg[] = {0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5};
    u8 n[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84, 0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51};

    u8 p[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    //RFC4754 test vector
    //secret key
    u8 d3[] = {0xDC, 0x51, 0xD3, 0x86, 0x6A, 0x15, 0xBA, 0xCD, 0xE3, 0x3D, 0x96, 0xF9, 0x92, 0xFC, 0xA9, 0x9D, 0xA7, 0xE6, 0xEF, 0x09, 0x34, 0xE7, 0x09, 0x75, 0x59, 0xC2, 0x7F, 0x16, 0x14, 0xC8, 0x8A, 0x7F};
    //message
    u8 msg3[] = {'a', 'b', 'c'};
    //public key
    u8 xq3[] = {0x24, 0x42, 0xA5, 0xCC, 0x0E, 0xCD, 0x01, 0x5F, 0xA3, 0xCA, 0x31, 0xDC, 0x8E, 0x2B, 0xBC, 0x70, 0xBF, 0x42, 0xD6, 0x0C, 0xBC, 0xA2, 0x00, 0x85, 0xE0, 0x82, 0x2C, 0xB0, 0x42, 0x35, 0xE9, 0x70};
    u8 yq3[] = {0x6F, 0xC9, 0x8B, 0xD7, 0xE5, 0x02, 0x11, 0xA4, 0xA2, 0x71, 0x02, 0xFA, 0x35, 0x49, 0xDF, 0x79, 0xEB, 0xCB, 0x4B, 0xF2, 0x46, 0xB8, 0x09, 0x45, 0xCD, 0xDF, 0xE7, 0xD5, 0x09, 0xBB, 0xFD, 0x7D};
    //signature for the message above
    u8 r3[] = {0xCB, 0x28, 0xE0, 0x99, 0x9B, 0x9C, 0x77, 0x15, 0xFD, 0x0A, 0x80, 0xD8, 0xE4, 0x7A, 0x77, 0x07, 0x97, 0x16, 0xCB, 0xBF, 0x91, 0x7D, 0xD7, 0x2E, 0x97, 0x56, 0x6E, 0xA1, 0xC0, 0x66, 0x95, 0x7C};
    u8 s3[] = {0x86, 0xFA, 0x3B, 0xB4, 0xE2, 0x6C, 0xAD, 0x5B, 0xF9, 0x0B, 0x7F, 0x81, 0x89, 0x92, 0x56, 0xCE, 0x75, 0x94, 0xBB, 0x1E, 0xA0, 0xC8, 0x92, 0x12, 0x74, 0x8B, 0xFF, 0x3B, 0x3D, 0x5B, 0x03, 0x15};

    int i = 0;
    int resu = 0;
    //verify the KAT
#ifdef _MXIM_HSM

    u8        l_tucSignature[_MXIM_ECDSA_SIGNATURE_LEN];
    int       l_iSignatureLength = _MXIM_ECDSA_SIGNATURE_LEN;
    unsigned long l_ulAttributeKeyType = CKA_LABEL;
    unsigned long l_ulHSMLabelKeyLength = strlen(g_tcHSMECDSALabelKey);

    int       l_iMsg3Length = sizeof(msg3);

    resu = g_objMXIMUCLLibrary.ECDSAVerifyP256r1Sha256(32, xg, yg, xq3, yq3, r3, s3, a, n, p, msg3, sizeof(msg3));
#else
    resu = ucl_ecdsa_verify_p256r1_sha256(32, xg, yg, xq3, yq3, r3, s3, a, n, p, msg3, sizeof(msg3));
#endif


    if (TRUE == verbose)
    {
        if (resu == UCL_OK)
        {
            printf("KAT ECDSA-P256r1-SHA256 SIGNATURE VERIFICATION TEST-1 OK\n");
        }
        else
        {
            printf("KAT ECDSA-P256r1-SHA256 SIGNATURE VERIFICATION TEST-1 NOK %d \n", resu);
        }
    }

#ifdef _MXIM_HSM

    memset(l_tucSignature, 0, _MXIM_ECDSA_SIGNATURE_LEN);

    resu = MXIMHSMSHA256ECDSASign(&g_objMXHSMCLI,
                                  &g_objMXIMUCLLibrary,
                                  (PUCHAR)payload,
                                  (PULONG)&payload_len,
                                  &l_ulAttributeKeyType,
                                  (PUCHAR)g_tcHSMECDSALabelKey,
                                  &l_ulHSMLabelKeyLength,
                                  (PUCHAR)l_tucSignature,
                                  (PULONG)&l_iSignatureLength);

    if (resu != UCL_OK)
    {
        printf("ERROR on ECDSA sha256 sign (%d)\n", resu);
        return (EXIT_FAILURE);
    }

    for (i = 0; i < _MXIM_ECDSA_SIGNATURE_LEN; i++)
    {
        if (i < 32)
        {
            r3[i] = l_tucSignature[i];
        }
        else
        {
            s3[i - 32] = l_tucSignature[i];
        }
    }

#else

    for (i = 0; i < ECDSA_MODULUS_LEN; i++)
        d3[i] = config_struct.ecdsa_privkey[i];

    for (i = 0; i < ECDSA_MODULUS_LEN; i++)
        xq3[i] = config_struct.ecdsa_pubkey_x[i];

    for (i = 0; i < ECDSA_MODULUS_LEN; i++)
        yq3[i] = config_struct.ecdsa_pubkey_y[i];

    resu = ucl_ecdsa_sign_p256r1_sha256(32, xg, yg, xq3, yq3, r3, s3, a, n, p, d3, payload, payload_len);

#endif

    if (resu != UCL_OK)
    {
        printf("ECDSA-P256r1-SHA256 SIGNATURE COMPUTATION TEST-1 NOK %d \n", resu);
        exit(0);
    }

#ifdef _MXIM_HSM

    resu = MXIMHSMSHA256ECDSAVerify(&g_objMXHSMCLI,
                                    &g_objMXIMUCLLibrary,
                                    (PUCHAR)payload,
                                    (PULONG)&payload_len,
                                    &l_ulAttributeKeyType,
                                    (PUCHAR)g_tcHSMECDSALabelKey,
                                    &l_ulHSMLabelKeyLength,
                                    (PUCHAR)l_tucSignature,
                                    (ULONG)l_iSignatureLength);
    if (resu != UCL_OK)
    {
        printf("ERROR on ECDSA sha256 verify (%d)\n", resu);
        return (EXIT_FAILURE);
    }

#else //=> #ifndef _MXIM_HSM

    resu = ucl_ecdsa_verify_p256r1_sha256(32, xg, yg, xq3, yq3, r3, s3, a, n, p, payload, payload_len);

#endif //=> _MXIM_HSM

    if (resu != UCL_OK)
    {
        printf("ECDSA-P256r1-SHA256 SIGNATURE VERIFICATION TEST-1 NOK %d \n", resu);
        return (EXIT_FAILURE);
    }
    else
    {
        if (TRUE == verbose)
        {
            printf("ECDSA-P256r1-SHA256 SIGNATURE VERIFICATION TEST-1 OK %d \n", resu);
        }
    }

#ifndef _MXIM_HSM

    if (TRUE == verbose)
    {
        printf("payload(%d):", payload_len);

        for (i = 0; i < (int)payload_len; i++)
            printf("%02x", payload[i]);

        printf("\n");
        printf("r3:");

        for (i = 0; i < 32; i++)
            printf("%02x", r3[i]);

        printf("\n");
        printf("s3:");

        for (i = 0; i < 32; i++)
            printf("%02x", s3[i]);

        printf("\n");
    }

    for (i = 0; i < ECDSA_MODULUS_LEN; i++)
        signature[i] = r3[i];

    for (i = 0; i < ECDSA_MODULUS_LEN; i++)
        signature[ECDSA_MODULUS_LEN + i] = s3[i];

#else //=> #ifdef _MXIM_HSM

    if (TRUE == verbose)
    {
        printf("payload(%d):", payload_len);

        for (i = 0; i < (int)payload_len; i++)
        {
            printf("%02x", payload[i]);
        }

        printf("\n");
        printf("r3:");

        for (i = 0; i < (l_iSignatureLength / 2); i++)
        {
            printf("%02x", l_tucSignature[i]);
        }

        printf("\n");
        printf("s3:");

        for (i = (l_iSignatureLength / 2); i < l_iSignatureLength; i++)
        {
            printf("%02x", l_tucSignature[i]);
        }

        printf("\n");
    }

    memcpy(signature, l_tucSignature, l_iSignatureLength);

#endif




    if (TRUE == verbose)
    {
        printf("payload+signature:");

        for (i = 0; i < (int)payload_len; i++)
            printf("%02x", payload[i]);

        printf("\n");

        for (i = 0; i < (int)ECDSA_MODULUS_LEN * 2; i++)
            printf("%02x", signature[i]);

        printf("\n");
    }

    return (EXIT_SUCCESS);
}

int rsa_sign_payload(u8 *signature, u8 *payload, u32 payload_len)
{

    int j, i;
    int err;

#ifndef _MXIM_HSM
    ucl_rsa_public_key_t keyPu;
    ucl_rsa_private_key_t keyPr;

    int sl;


    u8 hash[UCL_SHA256_HASHSIZE];



    //  printf("rsa len=%d bytes\n",config_struct.rsa_len);
    keyPr.modulus_length = config_struct.rsa_len;
    for (j = 0; j < config_struct.rsa_len; j++)
        keyPr.modulus[j] = config_struct.rsa[j];
    for (j = 0; j < (int)keyPr.modulus_length; j++)
        keyPr.private_exponent[j] = config_struct.rsa_privexp[j];
    keyPu.public_exponent_length = config_struct.rsa_explen;
    for (j = 0; j < (int)keyPu.public_exponent_length; j++)
        keyPu.public_exponent[j] = config_struct.rsa_pubexp[j];
    keyPu.modulus_length = config_struct.rsa_len;
    for (j = 0; j < (int)keyPu.modulus_length; j++)
        keyPu.modulus[j] = config_struct.rsa[j];

    //salt length is 0
    sl = 0;

    //set up data to be signed
    err = ucl_sha256(hash, payload, payload_len);
    if (UCL_OK != err)
    {
        printf("ERROR on sha1\n");
        return (EXIT_FAILURE);
    }
    if (TRUE == verbose)
    {
        printf("hash:");
        for (i = 0; i < UCL_SHA256_HASHSIZE; i++)
            printf("%02x", hash[i]);
        printf("\n");
    }
    err = ucl_pkcs1_ssa_pss_sha256_sign(signature, payload, payload_len, &keyPr, sl);
    if (err != UCL_OK)
    {
        printf("ERROR on rsa pkcs1 sha256 sign (%d)\n", err);
        return (EXIT_FAILURE);
    }
    err = ucl_pkcs1_ssa_pss_sha256_verify(signature, payload, payload_len, &keyPu, sl);
    if (err != UCL_OK)
    {
        printf("ERROR in verify signature\n");
        return (EXIT_FAILURE);
    }
    else
    {
        if (TRUE == verbose)
        printf("pkcs1 sha256 ok\n");
    }

#else
    int       l_iSignatureLength    = UCL_RSA_KEY_MAXSIZE;
    unsigned long l_ulAttributeKeyType  = CKA_LABEL;
    unsigned long l_ulHSMLabelKeyLength = strlen(g_tcHSMRSALabelKey);

    //memset(input,0,inputsize);
    memset(signature, 0, l_iSignatureLength);

    err = MXIMHSMSHA256Sign(  &g_objMXHSMCLI,
                              &g_objMXIMUCLLibrary,
                              (PUCHAR)payload,
                              (PULONG)&payload_len,
                              &l_ulAttributeKeyType,
                              (PUCHAR)g_tcHSMRSALabelKey,
                              &l_ulHSMLabelKeyLength,
                              CKM_RSA_X_509,
                              (PUCHAR)signature,
                              (PULONG)&l_iSignatureLength);


    if (err != UCL_OK)
    {
        printf("ERROR on rsa pkcs1 sha256 sign (%d)\n", err);
        return (EXIT_FAILURE);
    }

#endif


    if (TRUE == verbose)
    {
        printf("signature:");
        for (i = 0; i < FLORA_SIGNATURE_LEN; i++)
            printf("%02x", signature[i]);
        printf("\n");
    }
    return (EXIT_SUCCESS);
}

int write_file(char *filename, u8 *payload, u32 payload_len, u8 *signature, int signature_len)
{
    FILE *fp;
    u32 i;
    int resu;
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        printf("ERROR on opening <%s>\n", filename);
        return (EXIT_FAILURE);
    }
    for (i = 0; i < payload_len; i++)
    {
        resu = fwrite(&(payload[i]), sizeof(u8), 1, fp);
        if (1 != resu)
        {
            printf("ERROR on writing payload byte #%d\n", i);
            return (EXIT_FAILURE);
        }
    }
    for (i = 0; i < (u32)signature_len; i++)
    {
        resu = fwrite(&(signature[i]), sizeof(u8), 1, fp);
        if (1 != resu)
        {
            printf("ERROR on writing signature byte #%d\n", i);
            return (EXIT_FAILURE);
        }
    }
    if (TRUE == verbose)
    {
        printf("signature:");
        for (i = 0; i < (u32)signature_len; i++)
            printf("%02x", signature[i]);
        printf("\n");
    }
    (void)fclose(fp);
    return (EXIT_SUCCESS);
}

//1.5.4: #4539
int write_signature(char *filename, u8 *signature, int signature_len)
{
    FILE *fp;
    u32 i;
    int resu;
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        printf("ERROR on opening <%s>\n", filename);
        return (EXIT_FAILURE);
    }
    for (i = 0; i < (u32)signature_len; i++)
    {
        resu = fwrite(&(signature[i]), sizeof(u8), 1, fp);
        if (1 != resu)
        {
            printf("ERROR on writing signature byte #%d\n", i);
            return (EXIT_FAILURE);
        }
    }
    if (TRUE == verbose)
    {
        printf("signature:");
        for (i = 0; i < (u32)signature_len; i++)
            printf("%02x", signature[i]);
        printf("\n");
    }
    (void)fclose(fp);
    return (EXIT_SUCCESS);
}

int add_header_to_payload(u8 *payload, u32 *payload_len)
{
    int i;
    /* offset represents the current index when filling the *header */
    int offset;
    int arguments_len;
    u8 header[MAX_HEADER_LEN];
    //synchro pattern (64 bits)
    if (strstr(algo, "rsa") != NULL)
    {
        header_len = FLORA_HEADER_LEN;
        header[0] = 0x59;
        header[1] = 0x45;
        header[2] = 0x53;
        header[3] = 0x57;
        header[4] = 0x45;
        header[5] = 0x43;
        header[6] = 0x41;
        header[7] = 0x4E;
    }
    if (strstr(algo, "ecdsa") != NULL)
    {
        header_len = ANGELA_HEADER_LEN;
        header[0] = 0x48;
        header[1] = 0x49;
        header[2] = 0x53;
        header[3] = 0x57;
        header[4] = 0x45;
        header[5] = 0x44;
        header[6] = 0x47;
        header[7] = 0x44;
    }

    offset = 8;
    //1.2.5
    //copy version (32bits)
    //copy byte per byte version to header, starting from msB to lsB
    for (i = 0; i < HEADER_VERSION_LEN; i++, offset++)
        header[offset] = version >> ((HEADER_VERSION_LEN - i - 1) * 8) & 0xff;
    //binary load address (32 bits)
    //check load_address_string is ok, i.e. 8 chars <=> 4 hex bytes <=> 32bits
    if (strlen(load_address_string) != HEADER_LOAD_ADDRESS_LEN * 2)
    {
        printf("ERROR on load_address len <%s>: is %d and shall be %d\n", load_address_string, (int)strlen(load_address_string), HEADER_SYNC_LEN);
        return (EXIT_FAILURE);
    }
    for (i = 0; i < (int)strlen(load_address_string); i += 2, offset++)
        if (EXIT_SUCCESS == test_hex(load_address_string[i], load_address_string[i + 1]))
            header[offset] = hex(load_address_string[i], load_address_string[i + 1]);
        else
        {
            printf("ERROR: non hexa char detected in string <%s> <%c%c>\n", load_address_string, load_address_string[i], load_address_string[i + 1]);
            return (EXIT_FAILURE);
        }
    if (sizeof(*payload_len) != HEADER_BINARY_LEN)
    {
        printf("ERROR: payload len size (%dbytes) is not %d bytes\n", (int)sizeof(*payload_len), HEADER_BINARY_LEN);
        return (EXIT_FAILURE);
    }
    //copy byte per byte payload_len to header, starting from msB to lsB
    for (i = 0; i < HEADER_BINARY_LEN; i++, offset++)
        header[offset] = (*payload_len >> ((HEADER_BINARY_LEN - i - 1) * 8)) & 0xff;

    //binary jump address (32 bits)
    //check jump_address_string is ok, i.e. 8 chars <=> 4 hex bytes <=> 32bits
    if (strlen(jump_address_string) != HEADER_JUMP_ADDRESS_LEN * 2)
    {
        printf("ERROR on jump_address len <%s>: is %d and shall be %d\n", jump_address_string, (int)strlen(jump_address_string), HEADER_SYNC_LEN);
        return (EXIT_FAILURE);
    }
    for (i = 0; i < (int)strlen(jump_address_string); i += 2, offset++)
        if (EXIT_SUCCESS == test_hex(jump_address_string[i], jump_address_string[i + 1]))
            header[offset] = hex(jump_address_string[i], jump_address_string[i + 1]);
        else
        {
            printf("ERROR: non hexa char detected in string <%s> <%c%c>\n", jump_address_string, jump_address_string[i], jump_address_string[i + 1]);
            return (EXIT_FAILURE);
        }
    arguments_len = (u32)strlen(arguments);
    //copy byte per byte arguments_len to header, starting from msB to lsB
    for (i = 0; i < HEADER_ARGV_LEN; i++, offset++)
        header[offset] = arguments_len >> ((HEADER_ARGV_LEN - i - 1) * 8) & 0xff;
    if (strstr(algo, "rsa") != NULL)
    {
        //copy general_information byte -removed-
        //  header[offset]=general_info;
        //offset++;
        //copy sr_papd (1byte)
        header[offset] = sr_papd;
        offset++;
        //copy sr_prfsh (4bytes)
        //copy byte per byte sr_prfsh to header, starting from msB to lsB
        for (i = 0; i < HEADER_SR_PRFSH_LEN; i++, offset++)
            header[offset] = sr_prfsh >> ((HEADER_SR_PRFSH_LEN - i - 1) * 8) & 0xff;
        //copy sr_pcfg (4bytes)
        //copy byte per byte sr_pcfg to header, starting from msB to lsB
        for (i = 0; i < HEADER_SR_PCFG_LEN; i++, offset++)
            header[offset] = sr_pcfg >> ((HEADER_SR_PCFG_LEN - i - 1) * 8) & 0xff;
        //1.2.5
        //copy sr_pext (1byte)
        header[offset] = sr_pext;
        offset++;
        //copy dmc_gcfg(4bytes)
        //copy byte per byte dmc_pcfg to header, starting from msB to lsB
        for (i = 0; i < HEADER_DMC_GCFG_LEN; i++, offset++)
            header[offset] = dmc_gcfg >> ((HEADER_DMC_GCFG_LEN - i - 1) * 8) & 0xff;
        //copy dmc_clk(1byte)
        //1.2.6: 1byte and not 2
        //copy byte per byte dmc_clk to header, starting from msB to lsB
        //  for(i=0;i<HEADER_DMC_CLK_LEN;i++,offset++)
        // header[offset]=dmc_clk>>((HEADER_DMC_CLK_LEN-i-1)*8)&0xff;
        header[offset] = dmc_clk;
        offset++;
        //1.2.7: ksrc renamed
        //copy uci0_ksrc_configencint
        header[offset] = uci0_ksrc_configencint;
        offset++;
        //copy uci0_ac1r_start_offset
        //copy byte per byte uci0_ac1r_so to header, starting from msB to lsB
        for (i = 0; i < HEADER_UCI0_AC1R_START_OFFSET_LEN; i++, offset++)
            header[offset] = uci0_ac1r_so >> ((HEADER_UCI0_AC1R_START_OFFSET_LEN - i - 1) * 8) & 0xff;
        //copy uci0_ac1r_end_offset
        //copy byte per byte uci0_ac1r_eo to header, starting from msB to lsB
        for (i = 0; i < HEADER_UCI0_AC1R_END_OFFSET_LEN; i++, offset++)
            header[offset] = uci0_ac1r_eo >> ((HEADER_UCI0_AC1R_END_OFFSET_LEN - i - 1) * 8) & 0xff;
        //copy uci0_ddr_r0
        //copy byte per byte uci0_ddr_r0 to header, starting from msB to lsB
        for (i = 0; i < HEADER_UCI0_DDR_R0_LEN; i++, offset++)
            header[offset] = uci0_ddr_r0 >> ((HEADER_UCI0_DDR_R0_LEN - i - 1) * 8) & 0xff;
        //1.2.8: ky1r removed
        //copy ky1r
        //copy byte per byte ky1r to header,
        //  for(i=0;i<HEADER_KY1R_LEN;i++,offset++)
        //header[offset]=ky1r[i];
    }
    if (strstr(algo, "ecdsa") != NULL)
    {
        for (i = 0; i < HEADER_APPLICATION_VERSION_LEN; i++, offset++)
            header[offset] = application_version >> ((HEADER_APPLICATION_VERSION_LEN - i - 1) * 8) & 0xff;
    }
    if (TRUE == verbose)
    {
        printf("header:");
        for (i = 0; i < header_len; i++)
            printf("%02x", header[i]);
        printf("\n");
        printf("header len: %d\n", header_len);
    }

    //shift the payload to accept the arguments
    for (i = *payload_len - 1; i >= 0; i--)
        payload[i + arguments_len] = payload[i];
    //put the arguments in the empty space
    for (i = 0; i < arguments_len; i++)
        payload[i] = arguments[i];
    if (TRUE == verbose)
    {
        printf("binary after arguments\n");
        for (i = 0; i < (int)(*payload_len + arguments_len); i++)
            printf("%02x", payload[i]);
        printf("\n");
    }
    if (TRUE == verbose)
    {
        printf("payload len was %dbytes\n", *payload_len);
    }
    *payload_len = *payload_len + arguments_len;
    if (TRUE == verbose)
    {
        printf("payload len is now %dbytes\n", *payload_len);
    }

    //shift the payload to accept the header
    for (i = *payload_len - 1; i >= 0; i--)
        payload[i + header_len] = payload[i];
    //put the header in the empty space
    for (i = 0; i < header_len; i++)
        payload[i] = header[i];
    if (TRUE == verbose)
    {
        printf("payload len was %dbytes\n", *payload_len);
    }
    *payload_len = *payload_len + header_len;
    if (TRUE == verbose)
    {
        printf("binary after HEADER\n");
        for (i = 0; i < (int)(*payload_len); i++)
            printf("%02x", payload[i]);
        printf("\n");
    }
    if (TRUE == verbose)
    {
        printf("payload len is now %dbytes\n", *payload_len);
    }

    return (EXIT_SUCCESS);
}

int read_binary_file( u8*   p_pucData, int* size, char* filename)
{
    int   i   = 0;
    int   resu  = EXIT_SUCCESS;
    FILE* pFile = NULL;


    if (TRUE == verbose)
    {
        printf("<read_binary_file <%s>>\n", filename);
    }

    pFile = fopen(filename, "rb");
    if (pFile == NULL)
    {
        printf("ERROR on opening <%s>\n", filename);

        return EXIT_FAILURE;
    }

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    *size = ftell (pFile);
    rewind (pFile);

    // copy the file into the buffer:
    resu = fread(p_pucData, 1, *size, pFile);
    if (resu != *size)
    {
        printf ("Reading error <%s>\n", filename);
        return EXIT_FAILURE;
    }

    if (TRUE == verbose)
    {
        printf("%d bytes read\n", *size);

        for (i = 0; i < (int)(*size); i++)
            printf("%02x", p_pucData[i]);

        printf("\n");
    }

    fclose(pFile);



    return EXIT_SUCCESS;
}


int read_file_ascii_data(FILE*  p_pFile, const int p_iHexDataLength, u8* p_pucHexDataBuf, int*  p_piHexDataBufLen)
{
    int   l_iErr   = EXIT_SUCCESS;
    int   l_iIndex = 0;
    int   l_iData  = 0;
    if (p_pFile == NULL)
    {
        return EXIT_FAILURE;
    }
    if (p_pucHexDataBuf == NULL)
    {
        return EXIT_FAILURE;
    }
    if (p_piHexDataBufLen == NULL)
    {
        return EXIT_FAILURE;
    }
    if (*p_piHexDataBufLen == 0)
    {
        return EXIT_FAILURE;
    }
    if (*p_piHexDataBufLen < p_iHexDataLength)
    {
        return EXIT_FAILURE;
    }
    for (l_iIndex = 0; l_iIndex < p_iHexDataLength; l_iIndex++)
    {
#ifndef _MXIM_HSM
        l_iErr = fscanf(p_pFile, "%02x", &l_iData);
#else
        l_iErr = fscanf_s(p_pFile, "%02x", &l_iData);
#endif//MXIM_HSM
        if (l_iErr != 1)
        {
            printf("ERROR: read text file error\n");
            return (EXIT_FAILURE);
        }
        p_pucHexDataBuf[l_iIndex] = l_iData;
    }
    return EXIT_SUCCESS;
}

int read_file_ecdsa(u8* puk_x, u8* puk_y, u8* privk, int size, char* filename)
{
    FILE* l_pFile = NULL;
    int   l_iErr = 0;
    int   l_iSize = 0;
    int   l_iIndex = 0;
    if (filename == NULL)
    {
        printf("ERROR read_file_ecdsa - invalid file name. \n");
        return EXIT_FAILURE;
    }
    if (TRUE == verbose)
        printf("<read_file_ecdsa <%s>>\n", filename);
    l_pFile = fopen(filename, "r");
    if (l_pFile == NULL)
    {
        printf("ERROR on opening <%s>\n", filename);
        return EXIT_FAILURE;
    }
    //-----------------//
    // ECDSA - PRIVK --//
    //-----------------------------------------------------------------
    memset(privk, 0, size);
    l_iSize = size;
    l_iErr = read_file_ascii_data(l_pFile, size, privk, &l_iSize);
    if (l_iErr != EXIT_SUCCESS)
    {
        printf("ERROR: read privk in ecdsa key file error\n");
        return (EXIT_FAILURE);
    }
    //-----------------//
    // ECDSA - PUK_X --//
    //-----------------------------------------------------------------
    memset(puk_x, 0, size);
    l_iSize = size;
    l_iErr = read_file_ascii_data(l_pFile, size, puk_x, &l_iSize);
    if (l_iErr != EXIT_SUCCESS)
    {
        printf("ERROR: read puk_x in ecdsa key file error\n");
        return (EXIT_FAILURE);
    }
    //-----------------//
    // ECDSA - PUK_Y --//
    //-----------------------------------------------------------------
    memset(puk_y, 0, size);
    l_iSize = size;
    l_iErr = read_file_ascii_data( l_pFile, size, puk_y, &l_iSize);
    if (l_iErr != EXIT_SUCCESS)
    {
        printf("ERROR: read puk_y in ecdsa key file error\n");
        return (EXIT_FAILURE);
    }
    if (TRUE == verbose)
    {
        for (l_iIndex = 0; l_iIndex < size; l_iIndex++)
            printf("%02x", puk_x[l_iIndex]);
        printf("\n");
        for (l_iIndex = 0; l_iIndex < size; l_iIndex++)
            printf("%02x", puk_y[l_iIndex]);
        printf("\n");
        for (l_iIndex = 0; l_iIndex < size; l_iIndex++)
            printf("%02x", privk[l_iIndex]);
        printf("\n");
    }
    fclose(l_pFile);
    return EXIT_SUCCESS;
}

int read_file_rsa(u8 *puk, int size, u8 *pukexp, u8 *privexp, int expsize, char *filename)
{
    FILE* l_pFile = NULL;

    int   l_iErr = 0;

    char  l_tcLine[MAXLINE];
    fpos_t  l_llCurrentPos;
    int   l_iSize = 0;
    int   l_iIndex = 0;

    if (TRUE == verbose)
    {
        printf("<read_file_rsa <%s>>\n", filename);
    }

    l_pFile = fopen(filename, "r");
    if (l_pFile == NULL)
    {
        printf("ERROR on opening <%s>\n", filename);
        return (EXIT_FAILURE);
    }

    memset(puk, 0, size);
    l_iSize = size;
    l_iErr = read_file_ascii_data(  l_pFile,
                                    size,
                                    puk,
                                    &l_iSize);
    if (l_iErr != EXIT_SUCCESS)
    {
        printf("ERROR: read puk in rsa file error\n");
        return (EXIT_FAILURE);
    }

    //-- save current file position
    fgetpos(l_pFile, &l_llCurrentPos);
    memset(l_tcLine, 0, MAXLINE);
    fgets(l_tcLine, MAXLINE, l_pFile);
    fsetpos(l_pFile, &l_llCurrentPos);

    if (((int)strlen(l_tcLine) / 2) == expsize)
    {
        //-- Read first public exponent and then private exponent
        memset(pukexp, 0, expsize);
        l_iSize = expsize;
        l_iErr = read_file_ascii_data(  l_pFile,
                                        expsize,
                                        pukexp,
                                        &l_iSize);
        if (l_iErr != EXIT_SUCCESS)
        {
            printf("ERROR: read pukexp in rsa file error\n");
            return (EXIT_FAILURE);
        }



        memset(privexp, 0, size);
        l_iSize = size;
        l_iErr = read_file_ascii_data(  l_pFile,
                                        size,
                                        privexp,
                                        &l_iSize);
        if (l_iErr != EXIT_SUCCESS)
        {
            printf("ERROR: read privexp in rsa file error\n");
            return (EXIT_FAILURE);
        }

    }
    else
    {
        //-- Read first private exponent and then public exponent
        memset(privexp, 0, size);
        l_iSize = size;
        l_iErr = read_file_ascii_data(  l_pFile,
                                        size,
                                        privexp,
                                        &l_iSize);
        if (l_iErr != EXIT_SUCCESS)
        {
            printf("ERROR: read privexp in rsa file error\n");
            return (EXIT_FAILURE);
        }


        memset(pukexp, 0, expsize);
        l_iSize = expsize;
        l_iErr = read_file_ascii_data(  l_pFile,
                                        expsize,
                                        pukexp,
                                        &l_iSize);
        if (l_iErr != EXIT_SUCCESS)
        {
            printf("ERROR: read pukexp in rsa file error\n");
            return (EXIT_FAILURE);
        }


    }

    if (TRUE == verbose)
    {
        for (l_iIndex = 0; l_iIndex < size; l_iIndex++)
            printf("%02x", puk[l_iIndex]);

        printf("\n");

        for (l_iIndex = 0; l_iIndex < expsize; l_iIndex++)
            printf("%02x", pukexp[l_iIndex]);

        printf("\n");

        for (l_iIndex = 0; l_iIndex < size; l_iIndex++)
            printf("%02x", privexp[l_iIndex]);

        printf("\n");
    }

    fclose(l_pFile);

    return EXIT_SUCCESS;
}


int read_file_size(u32 *size, char *filename)
{
    //  int   resu = EXIT_SUCCESS;
    FILE* pFile  = NULL;


    //char* buffer=NULL;

    if (TRUE == verbose)
        printf("<read_file_size <%s>>\n", filename);

    pFile = fopen ( filename , "rb" );
    if (pFile == NULL)
    {
        printf ("File error\n");
        return EXIT_FAILURE;
    }

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    *size = ftell (pFile);
    rewind (pFile);


    // allocate memory to contain the whole file:
    //buffer = (char*) malloc (sizeof(char)*(*size));
    //if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    //resu = fread (buffer,1,*size,pFile);
    //if (resu != *size) {fputs ("Reading error",stderr); exit (3);}



    fclose(pFile);
    //free(buffer);

    return EXIT_SUCCESS;
}


int process_string(char *output, char *reference, char *line, int fgets_correction)
{
    int i, j;
    char dupline[MAXLINE];
    char dupreference[MAXLINE];
    for (i = 0; i < (int)strlen(line); i++)
        dupline[i] = (char)toupper((int)line[i]);
    dupline[strlen(line)] = '\0';
    for (i = 0; i < (int)strlen(reference); i++)
        dupreference[i] = (char)toupper((int)reference[i]);
    dupreference[strlen(reference)] = '\0';
    if (strstr(dupline, dupreference) == dupline)
    {
        found = 1;
        for (j = -1, i = 0; i < (int)strlen(line) - fgets_correction; i++)
            if (line[i] == '=')
                j = i;
        if (strlen(line) > MAXLINE)
            return (EXIT_FAILURE);
        if (j != -1)
            for (i = j + 1; i < (int)strlen(line) - fgets_correction; i++)
                output[i - j - 1] = line[i];
        else
            return (EXIT_FAILURE);
        output[i - j - 1] = '\0';
    }
    return (EXIT_SUCCESS);
}

int process_hextab(u8 *value, char *reference, char *line, int fgets_correction)
{
    int i, j, k;
    char dupline[MAXLINE];
    char dupreference[MAXLINE];
    for (i = 0; i < (int)strlen(line); i++)
        dupline[i] = (char)toupper((int)line[i]);
    dupline[strlen(line)] = '\0';
    for (i = 0; i < (int)strlen(reference); i++)
        dupreference[i] = (char)toupper((int)reference[i]);
    dupreference[strlen(reference)] = '\0';
    k = -1;
    if (strstr(dupline, dupreference) == dupline)
    {
        found = 1;
        for (j = -1, i = 0; i < (int)strlen(line) - 1; i++)
            if (line[i] == '=')
                j = i;
        if (j != -1)
            for (k = 0, i = j + 1; i < (int)strlen(line) - fgets_correction; i += 2, k++)
            {
                if (EXIT_SUCCESS == test_hex(line[i], line[i + 1]))
                    value[k] = hex(line[i], line[i + 1]);
                else
                {
                    printf("ERROR: non hexa char detected in string <%s> <%c%c>\n", line, line[i], line[i + 1]);
                    return (EXIT_FAILURE);
                }
            }
    }
    return (EXIT_SUCCESS);
}

int process_value(u8 *value, char *reference, char *line, int limit, int fgets_correction)
{
    int i, j;
    char dupline[MAXLINE];
    char dupreference[MAXLINE];
    for (i = 0; i < (int)strlen(line); i++)
        dupline[i] = (char)toupper((int)line[i]);
    dupline[strlen(line)] = '\0';
    for (i = 0; i < (int)strlen(reference); i++)
        dupreference[i] = (char)toupper((int)reference[i]);
    dupreference[strlen(reference)] = '\0';
    if (strstr(dupline, dupreference) == dupline)
    {
        found = 1;
        for (j = -1, i = 0; i < (int)strlen(line) - 1; i++)
            if (line[i] == '=')
                j = i;
        if (j != -1)
            for ((*value) = 0, i = j + 1; i < (int)strlen(line) - fgets_correction; i++)
                (*value) = ((*value) * 10) + (int)line[i] - (int)'0';
        if ((limit != -1) && (*value) >= limit)
        {
            printf("ERROR: %s shall be less than %d\n", reference, limit);
            return (EXIT_FAILURE);
        }
    }
    return (EXIT_SUCCESS);
}

int process_longvalue(int *value, char *reference, char *line, int limit, int fgets_correction)
{
    int i, j;
    char dupline[MAXLINE];
    char dupreference[MAXLINE];
    for (i = 0; i < (int)strlen(line); i++)
        dupline[i] = (char)toupper((int)line[i]);
    dupline[strlen(line)] = '\0';
    for (i = 0; i < (int)strlen(reference); i++)
        dupreference[i] = (char)toupper((int)reference[i]);
    dupreference[strlen(reference)] = '\0';
    if (strstr(dupline, dupreference) == dupline)
    {
        found = 1;
        for (j = -1, i = 0; i < (int)strlen(line) - 1; i++)
            if (line[i] == '=')
                j = i;
        if (j != -1)
            for ((*value) = 0, i = j + 1; i < (int)strlen(line) - fgets_correction; i++)
                (*value) = ((*value) * 10) + (int)line[i] - (int)'0';
        if ((limit != -1) && (*value) >= limit)
        {
            printf("ERROR: %s shall be less than %d\n", reference, limit);
            return (EXIT_FAILURE);
        }
    }
    return (EXIT_SUCCESS);
}

int process_longhexvalue(u32 *value, char *reference, char *line, int limit, int fgets_correction)
{
    int i, j;
    char dupline[MAXLINE];
    char dupreference[MAXLINE];
    for (i = 0; i < (int)strlen(line); i++)
        dupline[i] = (char)toupper((int)line[i]);
    dupline[strlen(line)] = '\0';
    for (i = 0; i < (int)strlen(reference); i++)
        dupreference[i] = (char)toupper((int)reference[i]);
    dupreference[strlen(reference)] = '\0';
    if (strstr(dupline, dupreference) == dupline)
    {
        found = 1;
        for (j = -1, i = 0; i < (int)strlen(line) - 1; i++)
            if (line[i] == '=')
                j = i;
        if (j != -1)
            for ((*value) = 0, i = j + 1; i < (int)(strlen(line) - fgets_correction); i += 2)
                (*value) = ((*value) * 256) + hex(line[i], line[i + 1]);
        if ((limit != -1) && (int)(*value) >= limit)
        {
            printf("ERROR: %s shall be less than %d\n", reference, limit);
            return (EXIT_FAILURE);
        }
    }
    return (EXIT_SUCCESS);
}

int process_hexvalue(u8 *value, char *reference, char *line, int limit, int fgets_correction)
{
    int i, j;
    char dupline[MAXLINE];
    char dupreference[MAXLINE];
    for (i = 0; i < (int)strlen(line); i++)
        dupline[i] = (char)toupper((int)line[i]);
    dupline[strlen(line)] = '\0';
    for (i = 0; i < (int)strlen(reference); i++)
        dupreference[i] = (char)toupper((int)reference[i]);
    dupreference[strlen(reference)] = '\0';
    if (strstr(dupline, dupreference) == dupline)
    {
        found = 1;
        for (j = -1, i = 0; i < (int)strlen(line) - 1; i++)
            if (line[i] == '=')
                j = i;
        if (j != -1)
            for ((*value) = 0, i = j + 1; i < (int)strlen(line) - fgets_correction; i += 2)
                (*value) = ((*value) * 256) + hex(line[i], line[i + 1]);
        if ((limit != -1) && (*value) >= limit)
        {
            printf("ERROR: %s shall be less than %d\n", reference, limit);
            return (EXIT_FAILURE);
        }
    }
    return (EXIT_SUCCESS);
}

/* this function retrieves (and checks if possible) parameters provided in the ini file */
int process_arg(char *line, int fgets_correction)
{
    int resu;
    found = 0;

    /*  resu=process_hexvalue(&general_info,"general_info",line,255,fgets_correction);
    if(EXIT_SUCCESS!=resu)
        {
            printf("ERROR while extracting <general_info> field\n");
            return(EXIT_FAILURE);
            }*/
    resu = process_longhexvalue(&application_version, "application_version", line, 0xffffffff, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <version> field\n");
        return (EXIT_FAILURE);
    }
    resu = process_hexvalue(&sr_papd, "sr_papd", line, 255, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <sr_papd> field\n");
        return (EXIT_FAILURE);
    }
    //1.2.5
    resu = process_hexvalue(&sr_pext, "sr_pext", line, 255, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <sr_pext> field\n");
        return (EXIT_FAILURE);
    }
    //1.2.6: 4bytes
    resu = process_longhexvalue(&sr_prfsh, "sr_prfsh", line, 0xFFFFFFFF, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <sr_prfsh> field\n");
        return (EXIT_FAILURE);
    }
    //1.2.5
    resu = process_longhexvalue(&version, "version", line, 0xFFFFFFFF, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <version> field\n");
        return (EXIT_FAILURE);
    }
    resu = process_longhexvalue(&sr_pcfg, "sr_pcfg", line, 0xFFFFFFFF, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <sr_pcfg> field\n");
        return (EXIT_FAILURE);
    }
    //1.2.6: 4bytes
    resu = process_longhexvalue(&dmc_gcfg, "dmc_gcfg", line, 0xFFFFFFFF, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <dmc_gcfg> field\n");
        return (EXIT_FAILURE);
    }
    //1.2.6: 1byte
    //  resu=process_longhexvalue(&dmc_clk,"dmc_clk",line,255,fgets_correction);
    resu = process_hexvalue(&dmc_clk, "dmc_clk", line, 255, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <dmc_clk> field\n");
        return (EXIT_FAILURE);
    }

//1.2.7: ksrc renamed
    resu = process_hexvalue(&uci0_ksrc_configencint, "uci0_ksrc_configencint", line, 255, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
//1.2.7: ksrc renamed
        printf("ERROR while extracting <uci0_ksrc_configencint> field\n");
        return (EXIT_FAILURE);
    }

    resu = process_longhexvalue(&uci0_ac1r_so, "uci0_ac1r_so", line, 0xFFFFFFFF, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <uci0_ac1r_so> field\n");
        return (EXIT_FAILURE);
    }

    resu = process_longhexvalue(&uci0_ac1r_eo, "uci0_ac1r_eo", line, 0xFFFFFFFF, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <uci0_ac1r_eo> field\n");
        return (EXIT_FAILURE);
    }

    resu = process_longhexvalue(&uci0_ddr_r0, "uci0_ddr_r0", line, 0xFFFFFFFF, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <uci0_ddr_r0> field\n");
        return (EXIT_FAILURE);
    }

//1.2.8: ky1r removed
//  resu=process_hextab(ky1r,"ky1r",line,fgets_correction);
    //if(EXIT_SUCCESS!=resu)
    //{
    //  printf("ERROR while extracting <ky1r> field\n");
    //  return(EXIT_FAILURE);
    //}
    resu = process_string(rsafile, "rsa_file", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <rsa_file> field\n");
        return (EXIT_FAILURE);
    }

    resu = process_string(ecdsafile, "ecdsa_file", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <ecdsa_file> field\n");
        return (EXIT_FAILURE);
    }

    resu = process_string(algo, "algo", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <algo> field\n");
        return (EXIT_FAILURE);
    }

    if (strstr(line, "verbose") != NULL)
    {
        verbose = (strstr(line, "yes") != NULL) ? TRUE : FALSE;
        found = 1;
    }
    if (strstr(line, "signonly") != NULL)
    {
        signonly = (strstr(line, "yes") != NULL) ? TRUE : FALSE;
        found = 1;
    }
    if (strstr(line, "header") != NULL)
    {
        headergenerated = (strstr(line, "yes") != NULL) ? TRUE : FALSE;
        found = 1;
    }
    resu = process_string(cafile, "ca", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <ca> field\n");
        return (EXIT_FAILURE);
    }
    resu = process_string(load_address_string, "load_address", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <load_address> field\n");
        return (EXIT_FAILURE);
    }
    resu = process_string(jump_address_string, "jump_address", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <jump_address> field\n");
        return (EXIT_FAILURE);
    }
    resu = process_string(arguments, "arguments", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <arguments> field\n");
        return (EXIT_FAILURE);
    }
    resu = process_string(scafile, "sca", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <sca> field\n");
        return (EXIT_FAILURE);
    }


//#ifdef _MXIM_HSM

    resu = process_string(g_tcHSMRSALabelKey, "name_of_rsa_key", line, fgets_correction);

    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <name_of_rsa_key> field\n");
        return (EXIT_FAILURE);
    }

    resu = process_string(g_tcHSMECDSALabelKey, "name_of_ecdsa_key", line, fgets_correction);

    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <name_of_ecdsa_key> field\n");
        return (EXIT_FAILURE);
    }

    resu = process_string(g_tcQuorum_K, "quorum_k", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <quorum_k> field\n");
        return (EXIT_FAILURE);
    }


    resu = process_string(g_tcQuorum_N, "quorum_n", line, fgets_correction);
    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR while extracting <quorum_n> field\n");
        return (EXIT_FAILURE);
    }


//#endif

    if (!found)
    {
        printf("ERROR: line with unknown field: <%s>\n", line);
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

static int load_args(int argc, char **argv)
{
    int k;
    int resu;
    for (k = 1; k < argc; k++)
    {
        resu = process_arg(argv[k], 0);
        if (EXIT_SUCCESS != resu)
            return (EXIT_FAILURE);
    }
    for (k = 0; k < (int)strlen(algo); k++)
        algo[k] = (char)tolower((int)algo[k]);
    return (EXIT_SUCCESS);
}

int process(void)
{
    int   resu = 0;
    int   err = 0;
    int signature_len;
    config_struct.rsa_explen = EXP_BYTE_LEN;
    config_struct.rsa_len = FLORA_SIGNATURE_LEN;
    config_struct.ecdsa_len = ECDSA_MODULUS_LEN;

#ifndef _MXIM_HSM
    if (strstr(algo, "rsa") != NULL)
    {
        //reading the signing key
        resu = read_file_rsa( config_struct.rsa, config_struct.rsa_len, config_struct.rsa_pubexp, config_struct.rsa_privexp, config_struct.rsa_explen, rsafile);

        if (EXIT_SUCCESS != resu)
        {
            printf("ERROR in read_file_rsa\n");
            return (EXIT_FAILURE);
        }
    }
    if (strstr(algo, "ecdsa") != NULL)
    {
        //reading the signing key
        resu = read_file_ecdsa(config_struct.ecdsa_pubkey_x, config_struct.ecdsa_pubkey_y, config_struct.ecdsa_privkey, config_struct.ecdsa_len, ecdsafile);

        if (EXIT_SUCCESS != resu)
        {
            printf("ERROR in read_file_ecdsa\n");
            return (EXIT_FAILURE);
        }
    }

#endif

    //reading the binary file to be signed
    //file is read and data are put in payload, payload_len
    err = read_file_size(&payload_len, cafile);

    if (EXIT_SUCCESS != err)
    {
        return (EXIT_FAILURE);
    }

    payload = (u8*)malloc((MAX_HEADER_LEN + payload_len + strlen(arguments)) * sizeof(u8));

    if (NULL == payload)
    {
        printf("ERROR: unable to allocate memory for payload (%d bytes required)\n", payload_len);
        return (EXIT_FAILURE);
    }

    err = read_binary_file(payload, (int*)&payload_len, cafile);
    if (EXIT_SUCCESS != err)
    {
        printf("ERROR read_binary_file(%s)=%d\n", cafile, err);
        return (EXIT_FAILURE);
    }

    if (TRUE == verbose)
    {
        if (strstr(algo, "rsa") != NULL)
            printf("rsa_sign\n");
        if (strstr(algo, "ecdsa") != NULL)
            printf("ecdsa_sign\n");
    }

    if (TRUE == headergenerated)
    {
        err = add_header_to_payload(payload, &payload_len);
        if (EXIT_SUCCESS != err)
        {
            printf("ERROR add_header(%d)\n", err);
            return (EXIT_FAILURE);
        }
    }
    else
        printf("header not generated as instructed\n");

    if (strstr(algo, "rsa") != NULL)
    {
        err = rsa_sign_payload(signature, payload, payload_len);
        if (EXIT_SUCCESS != err)
        {
            printf("ERROR rsa_sign(%d)\n", err);
            return (EXIT_FAILURE);
        }
        signature_len = FLORA_SIGNATURE_LEN;
    }
    if (strstr(algo, "ecdsa") != NULL)
    {
        err = ecdsa_sign_payload(signature, payload, payload_len);
        if (EXIT_SUCCESS != err)
        {
            printf("ERROR ecdsa_sign(%d)\n", err);
            return (EXIT_FAILURE);
        }
        signature_len = ANGELA_SIGNATURE_LEN;
    }


    //1.5.4: #4539: if signature only, no signed file creation
    if (TRUE == verbose)
        printf("write_signature\n");
    sprintf(sigfile, "%s.sig", cafile);
    err = write_signature(sigfile, signature, signature_len);
    if (TRUE != signonly)
    {
        if (TRUE == verbose)
            printf("write_file\n");
        err = write_file(scafile, payload, payload_len, signature, signature_len);
    }
    free(payload);
    if (EXIT_SUCCESS != err)
    {
        printf("ERROR write(%d)\n", err);
        return (EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

int load_default_config()
{
    verbose = TRUE;
    sprintf(cafile, "file.bin");
    sprintf(scafile, "file.sbin");
    sprintf(rsafile, "crk_rsa_2048bits_test1.key");
    sprintf(load_address_string, "01020304");
    sprintf(jump_address_string, "02030405");
    sprintf(arguments, "now, this is time for all");
    sprintf(algo, "rsa");
    //  general_info=0xab;
    //1.2.5
    version = 0x010203ff;
    sr_papd = 0xac;
    //1.2.6
    sr_prfsh = 0xabcdef01;
    sr_pcfg = 0x01efcdab;
    //1.2.5
    sr_pext = 0xa3;
    //1.2.6
    dmc_gcfg = 0x02030405;
    //1.2.6
    dmc_clk = 0x04;
    uci2_ctrl_reg = 0x040506cd;
//1.2.8: ky1r removed
    return (EXIT_SUCCESS);
}

int load_ini_config(FILE *fp)
{
    char line[MAXLINE];
    int resu;
    //    printf("<load .ini config>\n");
    while (fgets(line, MAXLINE, fp) != NULL)
    {
        if (line[strlen(line)] != '\0')
        {
            printf("ERROR: overflow on line <%s>\n", line);
            return (EXIT_FAILURE);
        }
        if ('#' == line[0])
            continue;
        resu = process_arg(line, 1);
        if (resu != EXIT_SUCCESS)
            return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

//this function reads the .ini and configures the parameters
static int load_config(void)
{
    FILE *fp;
    int resu;
    load_default_config();
    //read the configuration file
    fp = fopen(INIFILE, "r");
    //if file not present
    if (fp == NULL)
    {
        //setup with the default configuration
        printf("WARNING: <%s> not found\n", INIFILE);
    }
    else
    {
        resu = load_ini_config(fp);
        if (resu != EXIT_SUCCESS)
            return (EXIT_FAILURE);
        (void)fclose(fp);
    }
    return (EXIT_SUCCESS);
}

int display_config(void)
{
//1.2.8: ky1r removed
//  int i;
    printf("<display config>\n");
    if (TRUE == verbose)
        printf("verbose\n");
    else
        printf("mute\n");
//1.5.4: #4539 signature only
    if (TRUE == signonly)
        printf("signature file generation only\n");
    else
        printf("signed application file generation\n");

    //1.5.5
    if (TRUE == headergenerated)
        printf("the secure header is generated\n");
    else
        printf("the secure header is not generated\n");

    //1.2.5
    printf("version: %08x\n", version);
#ifndef _MXIM_HSM
    if (strstr(algo, "rsa") != NULL)
        printf("rsa file: %s\n", rsafile);
    if (strstr(algo, "ecdsa") != NULL)
        printf("ecdsa file: %s\n", ecdsafile);
#endif
    printf("customer application (input) file: %s\n", cafile);
    printf("signed customer application (output) file: %s\n", scafile);
    printf("binary load address: %s\n", load_address_string);
    printf("binary jump address: %s\n", jump_address_string);
    printf("application arguments: <%s>\n", arguments);
    if (strstr(algo, "rsa") != NULL)
        printf("RSA\n");
    if (strstr(algo, "ecdsa") != NULL)
        printf("ECDSA\n");
    if (strstr(algo, "ecdsa") != NULL)
    {
        printf("application version: %08x\n", application_version);
    }
    if (strstr(algo, "rsa") != NULL)
    {
        printf("Dynamic Memory Slot:");
        //  printf("\tgeneral information: %02x\n",general_info);
        printf("\tSR_PAPD: %02x\n", sr_papd);
        //1.2.6
        printf("\tSR_PRFSH: %08x\n", sr_prfsh);
        printf("\tSR_PCFG: %08x\n", sr_pcfg);
        printf("\tSR_PEXT: %02x\n", sr_pext);
        //1.2.6
        printf("\tMEM_GCFG: %08x\n", dmc_gcfg);
        //1.2.6
        printf("\tDMC_CLK: %02x\n", dmc_clk);
        printf("UCI2 parameters\n");
        //1.2.7: ksrc renamed
        printf("\tKSRC-Config ENC-INT: %02x\n", uci0_ksrc_configencint);
        printf("\tuci0-AC1R-start-offset: %08x\n", uci0_ac1r_so);
        printf("\tuci0-AC1R-end-offset: %08x\n", uci0_ac1r_eo);
        printf("\tuci0-DDR-r0: %08x\n", uci0_ddr_r0);
        //1.2.8: ky1r removed
#ifdef _MXIM_HSM
        if (strlen(g_tcHSMRSALabelKey) != 0)
        {
            printf("rsa key: %s\n", g_tcHSMRSALabelKey);
        }

        if (strlen(g_tcHSMECDSALabelKey) != 0)
        {
            printf("ecdsa key: %s\n", g_tcHSMECDSALabelKey);
        }
#endif
    }
    return (EXIT_SUCCESS);
}

int init()
{
    int err = EXIT_SUCCESS;

#ifdef _MXIM_HSM
    err = g_objMXIMUCLLibrary.Init(init_buffer, 2048);

    compteur = 0;
    if (err != UCL_OK)
    {
        printf("ERROR for ucl_init %d\n", err);
    }
    //if(TRUE==verbose)
    //  printf("UCL Version: %s (%s)\n", (char *)g_objMXIMUCLLibrary.GetVersion(),(char *)g_objMXIMUCLLibrary.GetBuildDate());

#else
    err = ucl_init(init_buffer, 2048);
    if (err != UCL_OK)
    {
        printf("ERROR for ucl_init %d\n", err);
        return (EXIT_FAILURE);
    }
#endif

    return err;
}

int main(int argc, char **argv)
{
    int resu;

#ifdef _MXIM_HSM

    ULONG l_ulQuorumKValue = 0; /*= atol(g_tcQuorum_K);*/
    ULONG l_ulQuorumNValue = 0; /*= atol(g_tcQuorum_N);*/
    bool l_bPKCS11Login = false;
    unsigned long l_ulAttributeKeyType = CKA_LABEL;
    unsigned long l_ulHSMLabelKeyLength = 0;/*strlen(g_tcHSMLabelKey);*/

    memset(g_tcHSMRSALabelKey, 0, _MXIM_MAX_STRING);
    memset(g_tcHSMECDSALabelKey, 0, _MXIM_MAX_STRING);
    memset(g_tcQuorum_K, 0, _MXIM_MAX_STRING);
    memset(g_tcQuorum_N, 0, _MXIM_MAX_STRING);

    printf( "CA signature build v%d.%d.%d (build %d) (c)Maxim Integrated 2006-2016\n",
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_MAJ,
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_MIN,
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_Z,
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_BUILD);
    printf("\n--warning: this tool does handle keys with Thales nCipher Edge HSM --\n");

#else
    printf( "CA signature build v%d.%d.%d (build %d) (c)Maxim Integrated 2006-2016\n",
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_MAJ,
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_MIN,
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_Z,
            _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_BUILD);
    printf("\n--warning: this tool does not handle keys in a PCI-PTS compliant way, only for test --\n");

#endif

    resu = load_config();
    if (resu != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    resu = load_args(argc, argv);
    if (resu != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if (TRUE == verbose)
        resu = display_config();

    if (resu != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if (EXIT_SUCCESS != init())
        return (EXIT_FAILURE);

#ifdef _MXIM_HSM

    printf("\nHSM try open connection\n");

    l_ulQuorumKValue = atol(g_tcQuorum_K);
    l_ulQuorumNValue = atol(g_tcQuorum_N);


    if ( (l_ulQuorumKValue != 0) && (l_ulQuorumNValue != 0))
    {
        l_bPKCS11Login = true;
    }

    resu = MXIMHSMOpenConnection(&g_objMXHSMCLI,
                                 verbose,
                                 0x04,
                                 &l_ulQuorumKValue,
                                 &l_ulQuorumNValue);

    if (EXIT_SUCCESS != resu)
    {
        printf("ERROR: unable to open connection with HSM.\n");

        return (EXIT_FAILURE);
    }

#endif


    resu = process();

    if (EXIT_SUCCESS != resu)
    {
#ifdef _MXIM_HSM

        MXIMHSMCloseConnection(&g_objMXHSMCLI);

        if (EXIT_SUCCESS != resu)
        {
            printf("Warning: fails to close HSM connection\n");
        }
#endif

        return (EXIT_FAILURE);
    }



#ifdef _MXIM_HSM

    MXIMHSMCloseConnection(&g_objMXHSMCLI);

    if (EXIT_SUCCESS != resu)
    {
        printf("Warning: fails to close HSM connection\n");

    }
#endif


    return (EXIT_SUCCESS);

}
