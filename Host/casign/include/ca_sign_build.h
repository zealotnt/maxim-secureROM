#ifndef _MXIM_CA_SIGN_BUILD_INCLUDE
#define _MXIM_CA_SIGN_BUILD_INCLUDE


#ifdef _MXIM_HSM

	#include "scp_definitions.h"

	//------------------------
	//-- MXIMUCLI LIBRARY 
	//------------------------
	#include "MXIMUCL.h"

	//------------------------
	//-- MXIMNCIPHER LIBRARY 
	//------------------------
	#include "MXIMHSMCLI.h"


	//------------------------
	//-- CONSTANTS DEFINITION
	//------------------------

	#define INIFILE "ca_sign_build.ini"

	#define _MXIM_ECDSA_SIGNATURE_LEN	64

	#define ECDSA_MODULUS_LEN 32
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

	#define _MXIM_UCL_LIBRARY_PATH						 ".\\libucl.dll"

	//----------------------------------------------------------------------------------------------------------------
	using namespace MXIM::HSM::nCipher;

	MXIMUcl			g_objMXIMUCLLibrary(_MXIM_UCL_LIBRARY_PATH);
	CMXHSMCLI		g_objMXHSMCLI;
	
#endif

	//-----------------------
	// CONSTANTS
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	#define _MXIM_NCIPHER_CA_SIGN_BUILD_APP_INIFILE		 ".\\ca_sign_build.ini"
	#define _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_MAJ		1
	#define _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_MIN		5
	#define _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_Z		5
	#define _MXIM_NCIPHER_CA_SIGN_BUILD_APP_VER_BUILD	1

	#define _MXIM_MAX_STRING					10000


	char			g_tcHSMRSALabelKey[_MXIM_MAX_STRING];
	char			g_tcHSMECDSALabelKey[_MXIM_MAX_STRING];

	char			g_tcQuorum_K[_MXIM_MAX_STRING];
	char			g_tcQuorum_N[_MXIM_MAX_STRING];



#endif
