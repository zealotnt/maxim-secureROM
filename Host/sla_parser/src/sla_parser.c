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
// Created on: Sep,2013
// Author:
//
// ---- Subversion keywords (need to set the keyword property)
// $Rev::               $:  Revision of last commit
// $Author::            $:  Author of last commit
// $Date::              $:  Date of last commit
//
// CRK public key signature

// this tool parses signed applications from applications,
//using the FLORA secure encapsulation format defined in SDS22S01

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <defs.h>
#include <string.h>

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
#include <ucl/ecdsa_generic_api.h>

#define INIFILE "sla_parser.ini"
#define RSA_BYTE_LEN 256
#define AES_BYTE_LEN 16
#define EXP_BYTE_LEN 4
#define CV_BYTE_LEN 4
#define HEADER_SYNC_LEN 8
#define HEADER_VERSION_LEN 4
#define HEADER_LOAD_ADDRESS_LEN 4
#define HEADER_JUMP_ADDRESS_LEN 4
#define HEADER_BINARY_LEN 4
#define HEADER_ARGV_LEN 4
//#define HEADER_GENERAL_INFO_LEN 1
//SDR renamed in SR
//lengths changed to latest flora wiki binary header format
#define HEADER_SR_PAPD_LEN 1
#define HEADER_SR_PRFSH_LEN 4
#define HEADER_SR_PCFG_LEN 4
#define HEADER_SR_PEXT_LEN 1
#define HEADER_DMC_GCFG_LEN 4
#define HEADER_DMC_CLK_LEN 1
#define HEADER_UCI_KSRC_CONFIGENCINT_LEN 1
#define HEADER_UCI0_AC1R_START_OFFSET_LEN 4
#define HEADER_UCI0_AC1R_END_OFFSET_LEN 4
#define HEADER_UCI0_DDR_R0_LEN 4
//#define HEADER_CTRL_REG_LEN 4
//#define HEADER_KY1R_LEN 16
//for angela

#define HEADER_APPLICATION_VERSION_LEN 4
#define HEADER_LOAD_ADDRESS_LEN 4
#define HEADER_JUMP_ADDRESS_LEN 4
#define HEADER_BINARY_LEN 4
#define HEADER_ARGV_LEN 4

#define FLORA_HEADER_LEN HEADER_SYNC_LEN+HEADER_VERSION_LEN+HEADER_LOAD_ADDRESS_LEN+HEADER_JUMP_ADDRESS_LEN+HEADER_BINARY_LEN+HEADER_ARGV_LEN+/*HEADER_GENERAL_INFO_LEN*/+HEADER_SR_PAPD_LEN+HEADER_SR_PRFSH_LEN+HEADER_SR_PCFG_LEN+HEADER_SR_PEXT_LEN+HEADER_DMC_GCFG_LEN+HEADER_DMC_CLK_LEN+HEADER_UCI_KSRC_CONFIGENCINT_LEN+HEADER_UCI0_AC1R_START_OFFSET_LEN+HEADER_UCI0_AC1R_END_OFFSET_LEN+HEADER_UCI0_DDR_R0_LEN
#define ANGELA_HEADER_LEN HEADER_SYNC_LEN+HEADER_VERSION_LEN+HEADER_LOAD_ADDRESS_LEN+HEADER_JUMP_ADDRESS_LEN+HEADER_BINARY_LEN+HEADER_ARGV_LEN+HEADER_APPLICATION_VERSION_LEN
#define MAX_HEADER_LEN FLORA_HEADER_LEN

#define MAJV 1
#define MINV 1
#define ZVER 1
#define BUILD 1
//1001 initial version, from ca_sign_build 1.2.9
//1011 signature start position corrected
//1021 bin file fopen moved from r to rb
//1101 ecdsa support (yumen, lighthouse)
//1111 algo check in display_config

#define ECDSA_MODULUS_LEN SECP256R1_BYTESIZE

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
#define ERROR 1
#define OK 0
#define YES 1
#define NOT 0

#define SYNCH1          0xBE
#define SYNCH2          0xEF
#define SYNCH3          0xED

#define CON_REQ 	0x01	//Connection request
#define CON_REP	        0x02	//Connection reply
#define CON_REF 	0x09	//Connection refused
#define DISC_REQ 	0x03	//Disconnection request
#define DISC_REP	0x04	//Disconnection reply
#define DATA_TRANSFER	0x05	//Data Exchange
#define ACK	        0x06	//Acknowledge
#define ECHO_REQ	0x0B	//Echo request
#define ECHO_REP	0x0C	//Echo reply
#define CHG_SP_0	0x07	//Change speed to 57600 b/s
#define CHG_SP_1	0x17	//Change speed to 115200 b/s
#define CHG_SP_2	0x27	//Change speed to 230400 b/s
#define CHG_SP_3	0x37	//Change speed to 345600 b/s
#define CHG_SP_4	0x47	//Change speed to 460800 b/s
#define CHG_SP_5	0x57	//Change speed to 576000 b/s
#define CHG_SP_6	0x67	//Change speed to 691200 b/s
#define CHG_SP_7	0x77	//Change speed to 806400 b/s
#define CHG_SP_8	0x87	//Change speed to 921600 b/s
#define CHG_SP_REP	0x08	//Change speed reply

#define WRITE_KEY	0xB2
#define WRITE_BPK       0xB3
#define WRITE_TIMEOUT	0x3A
#define UPDATE_LIFE_CYCLE 	0xDA
#define UPDATE_6A 	0xDB
#define WRITE_FLASH	0xC3
#define ERASE_FLASH	0xD4
#define BLANK_CHECK_FLASH 	0xE5
#define VERIFY_FLASH	0x08
#define SIGNATURE_CHECK	0xF7
#define SIGN_WRITE_FLASH    0xF9
#define LOCK_FLASH		0xD6
#define WRITE_PROCEDURE	0x73
#define REGISTER_PROCEDURE	0x75
#define CALL_PROCEDURE	0x74
#define ERROR_NO	0x00
#define ERROR_INVAL	0xEA
#define ERROR_ALREADY	0x137

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
#define HELLO_REP	0x2
#define HELLO_OFF_REQ   0x8
#define HELLO_OFF_REP   0x9
#define CHALLENGE	0x7
#define SUCCESS	        0x3
#define FAILURE	        0x4
#define DATA	        0x5

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

typedef struct _type_config
{
  u8 rsa[MAX_STRING];
  u8 rsa_pubexp[MAX_STRING];
  int rsa_len;
  int rsa_explen;
  u8 already_diversified;
  int ecdsa_len;
  u8 ecdsa_privkey[ECDSA_MODULUS_LEN];
  u8 ecdsa_pubkey_x[ECDSA_MODULUS_LEN];
  u8 ecdsa_pubkey_y[ECDSA_MODULUS_LEN];
  u8 pp;
} type_config_struct;

u8 signature[UCL_RSA_KEY_MAXSIZE];
char load_address_string[MAX_STRING];
char jump_address_string[MAX_STRING];
char arguments[MAX_STRING];
//u8 general_info;
u8 sr_papd;
u32 sr_prfsh;
u32 sr_pcfg;
u32 dmc_gcfg;
u8 dmc_clk;
u32 uci2_ctrl_reg;
u8 uci0_ksrc_configencint;
u32 uci0_ac1r_so;
u32 uci0_ac1r_eo;
u32 uci0_ddr_r0;

//u8  ky1r[16];
u8 sr_pext;
u32 version;

u32 init_buffer[2048];
char *source[2]={"host","bl"};
type_config_struct config_struct;
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
int seq,ch_id,tr_id;
u8 frame[MAX_FRAME];
u8 random_number[UCL_AES_BLOCKSIZE];
u8 response[UCL_AES_BLOCKSIZE];
int iframe;
u8 who;
int start_addr;
int end_addr;
u8 data[MAX_FLASH];
u8 *payload;
u32 payload_len;
int data_len;
int addr[MAX_FLASH];
int found;
char message[MAX_STRING];
char slafile[MAX_STRING];
char rsafile[MAX_STRING];
char ecdsafile[MAX_STRING];
char algo[MAX_STRING];
int compteur;
u8 verbose;
u8 bootloader;
int offset_output;
FILE *fp;

int test_hex(char c1,char c2)
{
  int value;
  int ok1,ok2;
  value=0;
  ok1=ok2=0;
  if(c1>='A' && c1<='F')
    {
      value=(c1-'A'+10);
      ok1=1;
    }
  if(c1>='a' && c1<='f')
    {
    value=(c1-'a'+10);
      ok1=1;
    }
  if(c1>='0' && c1<='9')
    {
    value=(c1-'0');
      ok1=1;
    }
  value*=16;
  if(c2>='A' && c2<='F')
    {
    value+=(c2-'A'+10);
      ok2=1;
    }
  if(c2>='a' && c2<='f')
    {
    value+=(c2-'a'+10);
      ok2=1;
    }
  if(c2>='0' && c2<='9')
    {
    value+=(c2-'0');
      ok2=1;
    }
  if(ok1==0 || ok2==0)
    return(EXIT_FAILURE);
  return(EXIT_SUCCESS);
}

int hex(char c1,char c2)
{
  int value;
  value=0;
  if(c1>='A' && c1<='F')
    value=(c1-'A'+10);
  if(c1>='a' && c1<='f')
    value=(c1-'a'+10);
  if(c1>='0' && c1<='9')
    value=(c1-'0');
  value*=16;
  if(c2>='A' && c2<='F')
    value+=(c2-'A'+10);
  if(c2>='a' && c2<='f')
    value+=(c2-'a'+10);
  if(c2>='0' && c2<='9')
    value+=(c2-'0');
  return(value);
}

int rsa_check_payload(u8 *signature,u8 *payload, u32 payload_len)
{
  ucl_rsa_public_key_t keyPu;
  int sl;
  int j,i;
  int err;
  u8 hash[UCL_SHA256_HASHSIZE];

  keyPu.public_exponent_length=config_struct.rsa_explen;
  for(j=0;j<(int)keyPu.public_exponent_length;j++)
    keyPu.public_exponent[j]=config_struct.rsa_pubexp[j];
  keyPu.modulus_length=config_struct.rsa_len;
  for(j=0;j<(int)keyPu.modulus_length;j++)
    keyPu.modulus[j]=config_struct.rsa[j];

  //salt length is 0
  sl=0;

  //set up data to be signed
  err=ucl_sha256(hash,payload,payload_len);
  if(UCL_OK!=err)
    {
      printf("ERROR on sha256\n");
      return(EXIT_FAILURE);
    }
  if(TRUE==verbose)
    {
      printf("hash:");
      for(i=0;i<UCL_SHA256_HASHSIZE;i++)
	printf("%02x",hash[i]);
      printf("\n");
    }
  err=ucl_pkcs1_ssa_pss_sha256_verify(signature,payload,payload_len, &keyPu,sl);
  if(err!=UCL_OK)
    {
      printf("ERROR in verify signature\n");
      return(EXIT_FAILURE);
    }
  else
    if(TRUE==verbose)
      printf("signature pkcs1 sha256 ok\n");
  return(EXIT_SUCCESS);
}

int ecdsa_check_payload(u8 *signature,u8 *payload, u32 payload_len)
{
  int i;
  u8 a[]={0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC};
    //  u8 b[]={0x5A,0xC6,0x35,0xD8,0xAA,0x3A,0x93,0xE7,0xB3,0xEB,0xBD,0x55,0x76,0x98,0x86,0xBC,0x65,0x1D,0x06,0xB0,0xCC,0x53,0xB0,0xF6,0x3B,0xCE,0x3C,0x3E,0x27,0xD2,0x60,0x4B};
  u8 xg[]={0x6B,0x17,0xD1,0xF2,0xE1,0x2C,0x42,0x47,0xF8,0xBC,0xE6,0xE5,0x63,0xA4,0x40,0xF2,0x77,0x03,0x7D,0x81,0x2D,0xEB,0x33,0xA0,0xF4,0xA1,0x39,0x45,0xD8,0x98,0xC2,0x96};
  u8 yg[]={0x4F,0xE3,0x42,0xE2,0xFE,0x1A,0x7F,0x9B,0x8E,0xE7,0xEB,0x4A,0x7C,0x0F,0x9E,0x16,0x2B,0xCE,0x33,0x57,0x6B,0x31,0x5E,0xCE,0xCB,0xB6,0x40,0x68,0x37,0xBF,0x51,0xF5};
  u8 n[]={0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51};

  u8 p[]={0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

   //public key
  u8 xq3[SECP256R1_BYTESIZE];
  u8 yq3[SECP256R1_BYTESIZE];
  u8 r3[SECP256R1_BYTESIZE];
  u8 s3[SECP256R1_BYTESIZE];
  
  int resu;
  for(i=0;i<ECDSA_MODULUS_LEN;i++)
    r3[i]=signature[i];
  for(i=0;i<ECDSA_MODULUS_LEN;i++)
    s3[i]=signature[i+ECDSA_MODULUS_LEN];

  for(i=0;i<ECDSA_MODULUS_LEN;i++)
    xq3[i]=config_struct.ecdsa_pubkey_x[i];
  
  for(i=0;i<ECDSA_MODULUS_LEN;i++)
    yq3[i]=config_struct.ecdsa_pubkey_y[i];
  
  resu=ucl_ecdsa_verify_p256r1_sha256(32,xg,yg,xq3,yq3,r3,s3,a,n,p,payload,payload_len);
  if(UCL_OK!=resu)
    return(EXIT_FAILURE);
  else  
    {
      printf("SIGNATURE OK\n");
      return(EXIT_SUCCESS);
    }
}

int write_file(char *filename,u8 *payload, u32 payload_len)
{
  FILE *fp;
  u32 i;
  int resu;
  fp=fopen(filename,"wb");
  if(fp==NULL)
    {
      printf("ERROR on opening <%s>\n",filename);
      return(EXIT_FAILURE);
    }
  for(i=0;i<payload_len;i++)
    {
      resu=fwrite(&(payload[FLORA_HEADER_LEN+i]),sizeof(u8),1,fp);
      if(1!=resu)
	{
	  printf("ERROR on writing payload byte #%d\n",i);
	  return(EXIT_FAILURE);
	}
    }
  (void)fclose(fp);
  return(EXIT_SUCCESS);
}

int parse_header_payload(u8 *payload,u32 payload_len)
{
  int i;
  /* offset represents the current index when filling the *header */
  int offset;
  int arguments_len;
  int binary_len;
  u32 application_version;
  int header_len;
  int err;
  u8 header[FLORA_HEADER_LEN];
  //synchro pattern (64 bits)
  if(strstr(algo,"rsa")!=NULL)
    {
      header_len=FLORA_HEADER_LEN;
      header[0]=0x59;
      header[1]=0x45;
      header[2]=0x53;
      header[3]=0x57;
      header[4]=0x45;
      header[5]=0x43;
      header[6]=0x41;
      header[7]=0x4E;
    }
  if(strstr(algo,"ecdsa")!=NULL)
    {
      header_len=ANGELA_HEADER_LEN;
      header[0]=0x48;
      header[1]=0x49;
      header[2]=0x53;
      header[3]=0x57;
      header[4]=0x45;
      header[5]=0x44;
      header[6]=0x47;
      header[7]=0x44;
    }

  printf("HEADER\n");
  printf("synchronization pattern: ");
  for(i=0;i<8;i++)
    {
      printf("%02x",payload[i]);
      if(header[i]!=payload[i])
	{
	  printf("ERROR in HEADER: value is %02x instead of %02x\n",payload[i],header[i]);
	  return(EXIT_FAILURE);
	}
    }
  printf("\n");
  offset=8;

  printf("ROM code version: ");
  for(i=0;i<HEADER_VERSION_LEN;i++)
    printf("%02x",payload[offset+i]);
  printf("\n");
  offset+=HEADER_VERSION_LEN;

  printf("binary load address: ");
  for(i=0;i<HEADER_BINARY_LEN;i++)
    printf("%02x",payload[offset+i]);
  printf("\n");
  offset+=HEADER_BINARY_LEN;

  printf("binary length: ");
  for(binary_len=0,i=0;i<HEADER_BINARY_LEN;i++)
    {
      printf("%02x",payload[offset+i]);
      binary_len=(binary_len<<8)^payload[offset+i];
    }
  printf(" -> %d bytes (+%d=%d)\n",binary_len,header_len,payload_len);
  printf("\n");
  offset+=HEADER_BINARY_LEN;

  printf("binary jump address: ");
  for(i=0;i<HEADER_JUMP_ADDRESS_LEN;i++)
    printf("%02x",payload[offset+i]);
  printf("\n");
  offset+=HEADER_JUMP_ADDRESS_LEN;

  printf("SLA arguments length: ");
  for(arguments_len=0,i=0;i<HEADER_ARGV_LEN;i++)
    {
      printf("%02x",payload[offset+i]);
      arguments_len=(arguments_len<<8)^payload[offset+i];
    }
  printf("\n");
  offset+=HEADER_ARGV_LEN;

 if(strstr(algo,"rsa")!=NULL)
    {
      printf("\nDynamic Memory Slot parameters\n");
      printf("SR_PAPD: ");
      printf("%02x",payload[offset]);
      offset++;

      printf("SR_PRFSH: ");
      for(i=0;i<4;i++)
	printf("%02x",payload[offset+i]);
      printf("\n");
      offset+=4;

      printf("SR_PCFG: ");
      for(i=0;i<4;i++)
	printf("%02x",payload[offset+i]);
      printf("\n");
      offset+=4;

      printf("SR_PEXT: ");
      printf("%02x",payload[offset]);
      offset++;

      printf("DMC_GCFG: ");
      for(i=0;i<4;i++)
	printf("%02x",payload[offset+i]);
      printf("\n");
      offset+=4;

      printf("DMC_CLK: ");
      printf("%02x",payload[offset]);
      offset++;
      
      printf("UCI2 parameters\n");

      printf("UCI0 basic configuration: ");
      printf("%02x",payload[offset]);
      offset++;

      printf("UCI0 configuration: ");
      printf("%02x",payload[offset]);
      offset++;

      printf("UCI0AC1R start offset: ");
      for(i=0;i<4;i++)
	printf("%02x",payload[offset+i]);
      printf("\n");
      offset+=4;

      printf("UCI0AC1R end offset: ");
      for(i=0;i<4;i++)
	printf("%02x",payload[offset+i]);
      printf("\n");
      offset+=4;

      printf("UCI0DDRR0: ");
      for(i=0;i<4;i++)
	printf("%02x",payload[offset+i]);
      printf("\n");
      offset+=4;
}
  if(strstr(algo,"ecdsa")!=NULL)
    {
      for(i=0;i<HEADER_APPLICATION_VERSION_LEN;i++,offset++)
	header[offset]=application_version>>((HEADER_APPLICATION_VERSION_LEN-i-1)*8)&0xff;
    }

  printf("\nArguments\n");
  for(i=0;i<arguments_len;i++)
    printf("%c",payload[offset+i]);
  printf("\n");
  offset+=arguments_len;

  printf("\nSIGNATURE\n");
  if(strstr(algo,"rsa")!=NULL)
    {
      for(i=0;i<FLORA_SIGNATURE_LEN;i++)
	{
	  signature[i]=payload[header_len+binary_len+i];
	  printf("%02x",signature[i]);
	}
    }
  if(strstr(algo,"ecdsa")!=NULL)
    {
      for(i=0;i<ANGELA_SIGNATURE_LEN;i++)
	{
	  signature[i]=payload[header_len+binary_len+i];
	  printf("%02x",signature[i]);
	}
    }

  printf("\n");
  write_file("raw.bin",payload,binary_len);
  if(strstr(algo,"rsa")!=NULL)
    {
      err=rsa_check_payload(signature,payload,header_len+binary_len);
      if(EXIT_SUCCESS!=err)
	{
	  printf("ERROR rsa_check(%d)\n",err);
	  return(EXIT_FAILURE);
	}
    }

  if(strstr(algo,"ecdsa")!=NULL)
    {
      err=ecdsa_check_payload(signature,payload,header_len+binary_len);
      if(EXIT_SUCCESS!=err)
	{
	  printf("ERROR ecdsa_check(%d)\n",err);
	  return(EXIT_FAILURE);
	}
    }

  return(EXIT_SUCCESS);
}

int read_file(u8 *data,u32 *size,char *filename)
{
  int i;
  int resu;
  u8 d8;
  FILE *fp;
  if(TRUE==verbose)
    printf("<read_file <%s>>\n",filename);
  fp=fopen(filename,"rb");
  if(fp==NULL)
    {
      printf("ERROR on opening <%s>\n",filename);
      return(EXIT_FAILURE);
    }
  *size=0;
  while((resu=fread(&d8,sizeof(u8),1,fp))==1)
    {
      data[*size]=d8;
      (*size)++;
    }
  if(TRUE==verbose)
    {
      printf("%d bytes read\n",*size);
      for(i=0;i<(int)(*size);i++)
      	printf("%02x",data[i]);
      printf("\n");
      }
  (void)fclose(fp);
  return(EXIT_SUCCESS);
}

int read_file_size(u32 *size,char *filename)
{
  int resu;
  u8 d8;
  FILE *fp;
  if(TRUE==verbose)
    printf("<read_file_size <%s>>\n",filename);
  fp=fopen(filename,"rb");
  if(fp==NULL)
    {
      printf("ERROR on opening <%s>\n",filename);
      return(EXIT_FAILURE);
    }
  *size=0;
  while((resu=fread(&d8,sizeof(u8),1,fp))==1)
    {
      (*size)++;
    }
  if(TRUE==verbose)
    {
      printf("%d bytes read\n",*size);
    }
  (void)fclose(fp);
  return(EXIT_SUCCESS);
}
int read_file_ascii_data(FILE*	p_pFile, const int p_iHexDataLength, u8* p_pucHexDataBuf, int*	p_piHexDataBufLen)
{
  int		l_iErr	 = EXIT_SUCCESS;
  int		l_iIndex = 0;
  int		l_iData  = 0; 
  if(p_pFile == NULL)
    {
      return EXIT_FAILURE;
    }
  if(p_pucHexDataBuf == NULL)
    {
      return EXIT_FAILURE;
    }
  if(p_piHexDataBufLen == NULL)
    {
      return EXIT_FAILURE;
    }
  if(*p_piHexDataBufLen == 0)
    {
      return EXIT_FAILURE;
    }
  if(*p_piHexDataBufLen < p_iHexDataLength)
    {
      return EXIT_FAILURE;
    }
  for(l_iIndex=0;l_iIndex<p_iHexDataLength;l_iIndex++)
    {
#ifndef _MXIM_HSM
      l_iErr=fscanf(p_pFile,"%02x",&l_iData);
#else
      l_iErr=fscanf_s(p_pFile,"%02x",&l_iData);
#endif//MXIM_HSM
      if(l_iErr!=1)
	{
	  printf("ERROR: read text file error\n");
	  return(EXIT_FAILURE);
	} 
      p_pucHexDataBuf[l_iIndex]=l_iData;
    }
  return EXIT_SUCCESS;
}

int read_file_public_ecdsa(u8* puk_x, u8* puk_y, int size, char* filename)
{
  FILE*	l_pFile = NULL;
  int		l_iErr=0;
  int		l_iSize=0;
  int		l_iIndex=0;
  if(filename == NULL)
    {
      printf("ERROR read_file_ecdsa - invalid file name. \n");
      return EXIT_FAILURE;
    }
  if(TRUE==verbose)
    printf("<read_file_ecdsa <%s>>\n",filename);
  l_pFile=fopen(filename,"r");
  if(l_pFile==NULL)
    {
      printf("ERROR on opening <%s>\n",filename);
      return EXIT_FAILURE;
    }
  //-----------------//
  // ECDSA - PUK_X --//
  //-----------------------------------------------------------------
  memset(puk_x,0,size);
  l_iSize=size;
  l_iErr = read_file_ascii_data(l_pFile, size, puk_x, &l_iSize);
  if(l_iErr != EXIT_SUCCESS)
    {
      printf("ERROR: read puk_x in ecdsa key file error\n");
      return(EXIT_FAILURE);
    }
  //-----------------//
  // ECDSA - PUK_Y --//
  //-----------------------------------------------------------------
  memset(puk_y,0,size);
  l_iSize=size;
  l_iErr = read_file_ascii_data( l_pFile, size, puk_y, &l_iSize);
  if(l_iErr != EXIT_SUCCESS)
    {
      printf("ERROR: read puk_y in ecdsa key file error\n");
      return(EXIT_FAILURE);
    }
  if(TRUE==verbose)
    {
      for(l_iIndex=0;l_iIndex<size;l_iIndex++)
	printf("%02x",puk_x[l_iIndex]);
      printf("\n");
      for(l_iIndex=0;l_iIndex<size;l_iIndex++)
	printf("%02x",puk_y[l_iIndex]);
      printf("\n");
    }
  fclose(l_pFile);
  return EXIT_SUCCESS;
}

int read_file_public_rsa(u8 *puk,int size,u8 *pukexp,char *filename)
{
  FILE *fp;
  int i;
  int resu;
  u8 d8;
  char line[MAXLINE];
    if(TRUE==verbose)
      printf("<read_file_public_rsa <%s>>\n",filename);
  fp=fopen(filename,"r");
  if(fp==NULL)
    {
      printf("ERROR on opening <%s>\n",filename);
      return(EXIT_FAILURE);
    }
  for(i=0;i<size;i++)
    puk[i]=0;
  for(i=0;i<size;i++)
    {
      resu=fscanf(fp,"%02x",(unsigned int*)&d8);
      if(resu!=1)
	{
	  printf("ERROR: unexpected size (%d-%d)\n",size, strlen(line)-1);
	  return(EXIT_FAILURE);
	}
      puk[i]=d8;
    }
  for(i=0;i<EXP_BYTE_LEN;i++)
    pukexp[i]=0;
  for(i=0;i<EXP_BYTE_LEN;i++)
    {
      resu=fscanf(fp,"%02x",(unsigned int*)&d8);
      if(resu!=1)
	{
	  printf("ERROR: unexpected size (%d-%d)\n",size, strlen(line)-1);
	  return(EXIT_FAILURE);
	}
      pukexp[i]=d8;
    }
  if(TRUE==verbose)
    {
      for(i=0;i<size;i++)
	printf("%02x",puk[i]);
      printf("\n");
      for(i=0;i<EXP_BYTE_LEN;i++)
	printf("%02x",pukexp[i]);
      printf("\n");
    }
  (void)fclose(fp);
  return(EXIT_SUCCESS);
}

int process_string(char *output,char *reference,char *line,int fgets_correction)
{
  int i,j;
  char dupline[MAXLINE];
  char dupreference[MAXLINE];
  for(i=0;i<(int)strlen(line);i++)
    dupline[i]=(char)toupper((int)line[i]);
  dupline[strlen(line)]='\0';
  for(i=0;i<(int)strlen(reference);i++)
    dupreference[i]=(char)toupper((int)reference[i]);
  dupreference[strlen(reference)]='\0';
  if(strstr(dupline,dupreference)==dupline)
    {
      found=1;
      for(j=-1,i=0;i<(int)strlen(line)-fgets_correction;i++)
	if(line[i]=='=')
	  j=i;
      if(strlen(line)>MAXLINE)
	return(EXIT_FAILURE);
      if(j!=-1)
	for(i=j+1;i<(int)strlen(line)-fgets_correction;i++)
	  output[i-j-1]=line[i];
      else
	return(EXIT_FAILURE);
      output[i-j-1]='\0';
    }
  return(EXIT_SUCCESS);
}

int process_hextab(u8 *value,char *reference,char *line,int fgets_correction)
{
  int i,j,k;
  char dupline[MAXLINE];
  char dupreference[MAXLINE];
  for(i=0;i<(int)strlen(line);i++)
    dupline[i]=(char)toupper((int)line[i]);
  dupline[strlen(line)]='\0';
  for(i=0;i<(int)strlen(reference);i++)
    dupreference[i]=(char)toupper((int)reference[i]);
  dupreference[strlen(reference)]='\0';
  k=-1;
  if(strstr(dupline,dupreference)==dupline)
    {
      found=1;
      for(j=-1,i=0;i<(int)strlen(line)-1;i++)
	if(line[i]=='=')
	  j=i;
      if(j!=-1)
	for(k=0,i=j+1;i<(int)strlen(line)-fgets_correction;i+=2,k++)
	  {
	    if(EXIT_SUCCESS==test_hex(line[i],line[i+1]))
	      value[k]=hex(line[i],line[i+1]);
	    else
	      {
		printf("ERROR: non hexa char detected in string <%s> <%c%c>\n",line,line[i],line[i+1]);
		return(EXIT_FAILURE);
	      }
	  }
    }
  return(EXIT_SUCCESS);
}

int process_value(u8 *value,char *reference,char *line,int limit,int fgets_correction)
{
  int i,j;
  char dupline[MAXLINE];
  char dupreference[MAXLINE];
  for(i=0;i<(int)strlen(line);i++)
    dupline[i]=(char)toupper((int)line[i]);
  dupline[strlen(line)]='\0';
  for(i=0;i<(int)strlen(reference);i++)
    dupreference[i]=(char)toupper((int)reference[i]);
  dupreference[strlen(reference)]='\0';
  if(strstr(dupline,dupreference)==dupline)
    {
      found=1;
      for(j=-1,i=0;i<(int)strlen(line)-1;i++)
	if(line[i]=='=')
	  j=i;
      if(j!=-1)
	for((*value)=0,i=j+1;i<(int)strlen(line)-fgets_correction;i++)
	  (*value)=((*value)*10)+(int)line[i]-(int)'0';
      if((limit!=-1) && (*value)>=limit)
	{
	  printf("ERROR: %s shall be less than %d\n",reference,limit);
	  return(EXIT_FAILURE);
	}
    }
  return(EXIT_SUCCESS);
}


int process_longvalue(int *value,char *reference,char *line,int limit,int fgets_correction)
{
  int i,j;
  char dupline[MAXLINE];
  char dupreference[MAXLINE];
  for(i=0;i<(int)strlen(line);i++)
    dupline[i]=(char)toupper((int)line[i]);
  dupline[strlen(line)]='\0';
  for(i=0;i<(int)strlen(reference);i++)
    dupreference[i]=(char)toupper((int)reference[i]);
  dupreference[strlen(reference)]='\0';
  if(strstr(dupline,dupreference)==dupline)
    {
      found=1;
      for(j=-1,i=0;i<(int)strlen(line)-1;i++)
	if(line[i]=='=')
	  j=i;
      if(j!=-1)
	for((*value)=0,i=j+1;i<(int)strlen(line)-fgets_correction;i++)
	  (*value)=((*value)*10)+(int)line[i]-(int)'0';
      if((limit!=-1)&&(*value)>=limit)
	{
	  printf("ERROR: %s shall be less than %d\n",reference,limit);
	  return(EXIT_FAILURE);
	}
    }
  return(EXIT_SUCCESS);
}

int process_longhexvalue(u32 *value,char *reference,char *line,int limit,int fgets_correction)
{
  int i,j;
  char dupline[MAXLINE];
  char dupreference[MAXLINE];
  for(i=0;i<(int)strlen(line);i++)
    dupline[i]=(char)toupper((int)line[i]);
  dupline[strlen(line)]='\0';
  for(i=0;i<(int)strlen(reference);i++)
    dupreference[i]=(char)toupper((int)reference[i]);
  dupreference[strlen(reference)]='\0';
  if(strstr(dupline,dupreference)==dupline)
    {
      found=1;
      for(j=-1,i=0;i<(int)strlen(line)-1;i++)
	if(line[i]=='=')
	  j=i;
      if(j!=-1)
	for((*value)=0,i=j+1;i<(int)(strlen(line)-fgets_correction);i+=2)
	  (*value)=((*value)*256)+hex(line[i],line[i+1]);
      if((limit!=-1)&&(int)(*value)>=limit)
	{
	  printf("ERROR: %s shall be less than %d\n",reference,limit);
	  return(EXIT_FAILURE);
	}
    }
  return(EXIT_SUCCESS);
}

int process_hexvalue(u8 *value,char *reference,char *line,int limit,int fgets_correction)
{
  int i,j;
  char dupline[MAXLINE];
  char dupreference[MAXLINE];
  for(i=0;i<(int)strlen(line);i++)
    dupline[i]=(char)toupper((int)line[i]);
  dupline[strlen(line)]='\0';
  for(i=0;i<(int)strlen(reference);i++)
    dupreference[i]=(char)toupper((int)reference[i]);
  dupreference[strlen(reference)]='\0';
  if(strstr(dupline,dupreference)==dupline)
    {
      found=1;
      for(j=-1,i=0;i<(int)strlen(line)-1;i++)
	if(line[i]=='=')
	  j=i;
      if(j!=-1)
	for((*value)=0,i=j+1;i<(int)strlen(line)-fgets_correction;i+=2)
	  (*value)=((*value)*256)+hex(line[i],line[i+1]);
      if((limit!=-1)&&(*value)>=limit)
	{
	  printf("ERROR: %s shall be less than %d\n",reference,limit);
	  return(EXIT_FAILURE);
	}
    }
  return(EXIT_SUCCESS);
}

/* this function retrieves (and checks if possible) parameters provided in the ini file */
int process_arg(char *line,int fgets_correction)
{
  int resu;
  found=0;
  resu=process_string(ecdsafile,"ecdsa_file",line,fgets_correction);
  if(EXIT_SUCCESS!=resu)
    {
      printf("ERROR while extracting <ecdsa_file> field\n");
      return(EXIT_FAILURE);
    }

  resu=process_string(algo,"algo",line,fgets_correction);
  if(EXIT_SUCCESS!=resu)
    {
      printf("ERROR while extracting <algo> field\n");
      return(EXIT_FAILURE);
    }

  resu=process_string(rsafile,"rsa_file",line,fgets_correction);
  if(EXIT_SUCCESS!=resu)
    {
      printf("ERROR while extracting <rsa> field\n");
      return(EXIT_FAILURE);
    }

  if(strstr(line,"verbose")!=NULL)
    {
      verbose=(strstr(line,"yes")!=NULL)?TRUE:FALSE;
      found=1;
    }
  resu=process_string(slafile,"sla",line,fgets_correction);
  if(EXIT_SUCCESS!=resu)
    {
      printf("ERROR while extracting <sla> field\n");
      return(EXIT_FAILURE);
    }
  if(!found)
    {
      printf("ERROR: line with unknown field: <%s>\n",line);
      return(EXIT_FAILURE);
    }
  return(EXIT_SUCCESS);
}

int load_args(int argc, char **argv)
{
  int k;
  int resu;
  for(k=1;k<argc;k++)
    {
      resu=process_arg(argv[k],0);
      if(EXIT_SUCCESS!=resu)
	return(EXIT_FAILURE);
    }
  return(EXIT_SUCCESS);
}

int process(void)
{
  int resu;
  int err;
  u32 psize;
  config_struct.rsa_explen=EXP_BYTE_LEN;
  config_struct.rsa_len=FLORA_SIGNATURE_LEN;
  config_struct.ecdsa_len=ECDSA_MODULUS_LEN;
  //reading the signing public key
  if(strstr(algo,"rsa")!=NULL)
    {
      resu=read_file_public_rsa(config_struct.rsa,config_struct.rsa_len,config_struct.rsa_pubexp,rsafile);
      if(EXIT_SUCCESS!=resu)
	{
	  printf("ERROR in read_file_public_rsa\n");
	  return(EXIT_FAILURE);
	}
    }
  if(strstr(algo,"ecdsa")!=NULL)
    {
      //reading the signing key
      resu=read_file_public_ecdsa(config_struct.ecdsa_pubkey_x,config_struct.ecdsa_pubkey_y,config_struct.ecdsa_len,ecdsafile);
      
      if(EXIT_SUCCESS!=resu)
	{
	  printf("ERROR in read_file_ecdsa\n");
	  return(EXIT_FAILURE);
	}
    }
  //file is read and data are put in payload, payload_len
  err=read_file_size(&payload_len,slafile);
  payload=(u8*)malloc(payload_len*sizeof(u8));
  if(NULL==payload)
    {
      printf("ERROR: unable to allocate memory for payload (%d bytes required)\n",payload_len);
      return(EXIT_FAILURE);
    }
  err=read_file(payload,&psize,slafile);
  if(EXIT_SUCCESS!=err)
    {
      printf("ERROR read_file(%s)=%d\n",slafile,err);
      return(EXIT_FAILURE);
    }
  if(psize!=payload_len)
    {
      printf("ERROR: %s size has changed: %d bytes vs %d bytes\n",slafile,psize,payload_len);
      return(EXIT_FAILURE);
    }
  if(TRUE==verbose)
    printf("\n");
  err=parse_header_payload(payload,payload_len);
  if(EXIT_SUCCESS!=err)
    {
      printf("ERROR add_header(%d)\n",err);
      return(EXIT_FAILURE);
    }
  free(payload);
  return(EXIT_SUCCESS);
}

int load_default_config(void)
{
  verbose=TRUE;
  sprintf(slafile,"file.sbin");
  sprintf(rsafile,"rsa.pub");
  return(EXIT_SUCCESS);
}

int load_ini_config(FILE *fp)
{
  char line[MAXLINE];
  int resu;
  //    printf("<load .ini config>\n");
  while(fgets(line,MAXLINE,fp)!=NULL)
    {
      if(line[strlen(line)]!='\0')
	{
	  printf("ERROR: overflow on line <%s>\n",line);
	  return(EXIT_FAILURE);
	}
      if('#'==line[0])
	continue;
      resu=process_arg(line,1);
      if(resu!=EXIT_SUCCESS)
	return(EXIT_FAILURE);
    }
  return(EXIT_SUCCESS);
}

//this function reads the .ini and configures the parameters
int load_config(void)
{
  FILE *fp;
  int resu;
  load_default_config();
  //read the configuration file
  fp=fopen(INIFILE,"r");
  //if file not present
  if(fp==NULL)
    {
      //setup with the default configuration
      printf("WARNING: <%s> not found\n",INIFILE);
    }
  else
    {
      resu=load_ini_config(fp);
      if(resu!=EXIT_SUCCESS)
	return(EXIT_FAILURE);
      (void)fclose(fp);
    }
  return(EXIT_SUCCESS);
}

int display_config(void)
{
  printf("<display config>\n");
  if(TRUE==verbose)
    printf("verbose\n");
  else
    printf("mute\n");
  printf("version: %08x\n",version);
  //1.1.1
  if(strstr(algo,"rsa")!=NULL)
    printf("rsa file: %s\n",rsafile);
  else
    if(strstr(algo,"ecdsa")!=NULL)
      printf("ecdsa file: %s\n",ecdsafile);
    else
      {
	printf("ERROR: algo is not specified\n");
	return(EXIT_FAILURE);
      }
  printf("customer application (input) file: %s\n",slafile);
  if(strstr(algo,"rsa")!=NULL)
    printf("RSA\n");
  if(strstr(algo,"ecdsa")!=NULL)
    printf("ECDSA\n");

  return(EXIT_SUCCESS);
}

int init(void)
{
  int err;
  err = ucl_init(init_buffer, 2048);
  if(err!=UCL_OK)
    {
      printf("ERROR for ucl_init %d\n",err);
      return(EXIT_FAILURE);
    }
  return(EXIT_SUCCESS);
}

int main(int argc,char **argv)
{
  int resu;
  printf("SLA parser v%d.%d.%d (build %d) (c)Maxim 2013\n",MAJV,MINV,ZVER,BUILD);

  resu=load_config();
  if(resu!=EXIT_SUCCESS)
    return(EXIT_FAILURE);
  resu=load_args(argc,argv);
  if(resu!=EXIT_SUCCESS)
    return(EXIT_FAILURE);
  if(TRUE==verbose)
    resu=display_config();
  if(resu!=EXIT_SUCCESS)
    return(EXIT_FAILURE);
  if(EXIT_SUCCESS!=init())
    return(EXIT_FAILURE);
  else
    {
      resu=process();
      if(EXIT_SUCCESS!=resu)
	return(EXIT_FAILURE);
      else
	return(EXIT_SUCCESS);
    }
}
