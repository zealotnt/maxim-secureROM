/**
 * @defgroup mlsHsm
 * @brief
 *
 * @{
 */

/**
 * @file mlsHsmInjection.h
 * @brief mlsHsm component
 *
 * @date 	$ Jan, 2017
 * @author	$
 */
#ifndef _MLS_HSM_INJECTION
#define _MLS_HSM_INJECTION

#ifdef __cplusplus
extern "C"
{
#endif

/********** Include section ***************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <defs.h>
#include <string.h>

#include "cryptoki.h"
#include "ctutil.h"

/********** Constant  and compile switch definition section *******************/

/********** Type definition section *******************************************/

/********** Macro definition section*******************************************/

/********** Function declaration section **************************************/
void mlsHsmPrintInfo();

int mlsHsmOpenConnection(CK_BYTE slotId, CK_SESSION_HANDLE* pSession);

int mlsHsmCloseConnection(CK_SESSION_HANDLE hSession);

int mlsHsmGetKey(CK_SESSION_HANDLE hSession,
                 const char* label,
                 CK_OBJECT_HANDLE* phPriKey,
                 CK_OBJECT_HANDLE* phPubKey);

int mlsECDSASignP256r1Sha256(CK_SESSION_HANDLE hSession,
                             CK_OBJECT_HANDLE hPriKey,
                             CK_BYTE* data,
                             CK_SIZE dataLen,
                             CK_BYTE** ppSign,
                             CK_SIZE* pSignLen);

void mlsGetECDSAPubkey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hPublicKey);

#ifdef __cplusplus
}
#endif

#endif /* _MLS_HSM_INJECTION */
