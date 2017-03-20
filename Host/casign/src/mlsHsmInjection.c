/**
 * @addtogroup mlsRtc
 * @{
 */

/**
 * @file mlsRtc.c
 * @brief RTC component Implementation
 *
 *
 * @date    $ March, 2016
 * @author  $
 */

/********** Include section ***************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <defs.h>
#include <string.h>

#include "cryptoki.h"
#include "ctutil.h"

/********** Local Constant and compile switch definition section **************/

/********** Local Type definition section *************************************/

/********** Local Macro definition section ************************************/
/**
 * Macro used to check a given condition. If the condition is not true, print
 * an error message exit.
 */
#define CHECK_COND(COND)                                             \
    if (!(COND))                                                     \
    {                                                                \
        fprintf(stderr, "%s(%d): Condition '%s' does not hold\n",    \
                __FILE__, __LINE__, #COND);                          \
        exit(EXIT_FAILURE);                                          \
    }

/**
 * Macro called after a P11 function call to check the return value. If the
 * return value != CKR_OK print an error and exit.
 */
#define CHECK_P11_ERROR(RV)                                          \
    if (rv != CKR_OK)                                                \
    {                                                                \
        fprintf(stderr, "%s(%d): PKCS#11 Failure 0x%08lx\n",          \
                __FILE__, __LINE__, RV);                             \
        exit(EXIT_FAILURE);                                          \
    }

/********** Local (static) variable definition section ************************/
static void findEccPrivateKey(CK_SESSION_HANDLE hSession,
                              const char* label,
                              CK_OBJECT_HANDLE* phKey);

static void findEccPublicKey(CK_SESSION_HANDLE hSession,
                             const char* label,
                             CK_OBJECT_HANDLE* phKey);

static void sha256HashData(CK_SESSION_HANDLE hSession,
                           CK_BYTE* pData,
                           CK_SIZE dataLen,
                           CK_BYTE** ppHash,
                           CK_SIZE* pHashLen);

/********** Local function definition section *********************************/
void dump_hex(char *string, CK_BYTE *array, unsigned int len)
{
    unsigned int i;
    printf("%s\r\n", string);
    for (i = 0; i < len; i++)
    {
        printf("0x%02X, ", array[i]);
    }
    printf("\r\n");
}

static void findEccPrivateKey(CK_SESSION_HANDLE hSession,
                              const char* label,
                              CK_OBJECT_HANDLE* phKey)
{
    CK_RV rv = CKR_OK;

    static CK_OBJECT_CLASS keyClass = CKO_PRIVATE_KEY;
    static CK_KEY_TYPE keyType = CKK_EC;

    CK_ATTRIBUTE findAttr[] =
    {
        {CKA_LABEL,         NULL,       0}, /* Must be first */
        {CKA_CLASS,         &keyClass,  sizeof(keyClass)},
        {CKA_KEY_TYPE,      &keyType,   sizeof(keyType)}
    };
    CK_COUNT findAttrCount = NUMITEMS(findAttr);

    CK_COUNT keyCount = 0;

    findAttr[0].pValue = (CK_VOID_PTR)label;
    findAttr[0].valueLen = strlen(label);

    rv = C_FindObjectsInit(hSession, findAttr, findAttrCount);
    CHECK_P11_ERROR(rv);

    rv = C_FindObjects(hSession, phKey, 1, &keyCount);
    CHECK_P11_ERROR(rv);
    CHECK_COND(keyCount >= 1);

    rv = C_FindObjectsFinal(hSession);
    CHECK_P11_ERROR(rv);
}

static void findEccPublicKey(CK_SESSION_HANDLE hSession,
                             const char* label,
                             CK_OBJECT_HANDLE* phKey)
{
    CK_RV rv = CKR_OK;

    static CK_OBJECT_CLASS keyClass = CKO_PUBLIC_KEY;
    static CK_KEY_TYPE keyType = CKK_EC;

    CK_ATTRIBUTE findAttr[] =
    {
        {CKA_LABEL,     NULL,       0}, /* Must be first */
        {CKA_CLASS,     &keyClass,  sizeof(keyClass)},
        {CKA_KEY_TYPE,  &keyType,   sizeof(keyType)},
    };
    CK_COUNT findAttrCount = NUMITEMS(findAttr);

    CK_COUNT keyCount = 0;

    findAttr[0].pValue = (CK_VOID_PTR)label;
    findAttr[0].valueLen = strlen(label);

    rv = C_FindObjectsInit(hSession, findAttr, findAttrCount);
    CHECK_P11_ERROR(rv);

    rv = C_FindObjects(hSession, phKey, 1, &keyCount);
    CHECK_P11_ERROR(rv);
    printf("rv=%d\r\n", (int)rv);
    printf("keyCount=%d\r\n", (int)keyCount);
    CHECK_COND(keyCount >= 1);

    rv = C_FindObjectsFinal(hSession);
    CHECK_P11_ERROR(rv);
}

static void sha256HashData(CK_SESSION_HANDLE hSession,
                           CK_BYTE* pData,
                           CK_SIZE dataLen,
                           CK_BYTE** ppHash,
                           CK_SIZE* pHashLen)
{
    CK_RV rv = CKR_OK;

    CK_MECHANISM mech = {CKM_SHA256, NULL, 0};

    CK_BYTE *pHash = NULL;
    CK_SIZE hashLen = 0;

    rv = C_DigestInit(hSession, &mech);
    CHECK_P11_ERROR(rv);

    rv = C_Digest(hSession, pData, dataLen, NULL, &hashLen);
    CHECK_P11_ERROR(rv);

    pHash = (CK_BYTE *)malloc(hashLen);
    CHECK_COND(pHash != NULL);

    rv = C_Digest(hSession, pData, dataLen, pHash, &hashLen);
    CHECK_P11_ERROR(rv);

    *ppHash = pHash;
    *pHashLen = hashLen;
}

/********** Global function definition section ********************************/
void mlsHsmPrintInfo()
{
    printf("SafeNet HSM\r\n");
    return;
}

int mlsHsmOpenConnection(CK_BYTE slotId, CK_SESSION_HANDLE* pSession, CK_CHAR* pUserPin)
{
    CK_RV rv = CKR_OK;
    CK_COUNT userPinLen;

    printf("mlsHsmOpenConnection(): slotId=%d\r\n", slotId);
    rv = C_Initialize(NULL);
    CHECK_P11_ERROR(rv);

    rv = C_OpenSession(slotId,
                       CKF_SERIAL_SESSION | CKF_RW_SESSION,
                       NULL,
                       NULL,
                       pSession);
    CHECK_P11_ERROR(rv);

    userPinLen = (CK_COUNT)strlen((char *)pUserPin);
    rv = C_Login(*pSession, CKU_USER, pUserPin, userPinLen);
    CHECK_P11_ERROR(rv);

    return CKR_OK;
}

int mlsHsmCloseConnection(CK_SESSION_HANDLE hSession)
{
    CK_RV rv = CKR_OK;

    rv = C_CloseSession(hSession);
    CHECK_P11_ERROR(rv);

    rv = C_Finalize(NULL);
    CHECK_P11_ERROR(rv);

    return CKR_OK;
}

int mlsHsmGetKey(CK_SESSION_HANDLE hSession,
                 const char* label,
                 CK_OBJECT_HANDLE* phPriKey,
                 CK_OBJECT_HANDLE* phPubKey)
{
    findEccPublicKey(hSession, label, phPubKey);
    findEccPrivateKey(hSession, label, phPriKey);
    return 0;
}

int mlsECDSAVerifyP256r1Sha256(CK_SESSION_HANDLE hSession,
                               CK_OBJECT_HANDLE hPubKey,
                               CK_BYTE* data,
                               CK_SIZE dataLen,
                               CK_BYTE* pSign,
                               CK_SIZE signLen)
{
    CK_RV rv;
    CK_MECHANISM mech = {CKM_ECDSA, NULL, 0};

    /* Hash the data */
    CK_BYTE* pHash = NULL;
    CK_SIZE hashLen = 0;
    sha256HashData(hSession, data, dataLen, &pHash, &hashLen);

    rv = C_VerifyInit(hSession, &mech, hPubKey);
    CHECK_P11_ERROR(rv);

    rv = C_Verify(hSession, pHash, hashLen, pSign, signLen);
    if (rv == CKR_SIGNATURE_INVALID)
    {
        printf("Signature invalid. Signature was:\n");
    }
    else if (rv == CKR_OK)
    {
        printf("Valid\n");
    }
    else
    {
        CHECK_P11_ERROR(rv);
    }

    return rv;
}

int mlsECDSASignP256r1Sha256(CK_SESSION_HANDLE hSession,
                             CK_OBJECT_HANDLE hPriKey,
                             CK_BYTE* data,
                             CK_SIZE dataLen,
                             CK_BYTE** ppSign,
                             CK_SIZE* pSignLen)
{
    CK_RV rv = CKR_OK;
    CK_MECHANISM mech = {CKM_ECDSA, NULL, 0};
    CK_BYTE *pSign = NULL;
    CK_SIZE signLen = 0;

    /* Hash the data */
    CK_BYTE* pHash = NULL;
    CK_SIZE hashLen = 0;
    sha256HashData(hSession, data, dataLen, &pHash, &hashLen);

    /* Sign the hash result. */
    rv = C_SignInit(hSession, &mech, hPriKey);
    CHECK_P11_ERROR(rv);

    rv = C_Sign(hSession, pHash, hashLen, NULL, &signLen);
    CHECK_P11_ERROR(rv);

    pSign = (CK_BYTE *)malloc(signLen);
    CHECK_COND(pSign != NULL);

    rv = C_Sign(hSession, pHash, hashLen, pSign, &signLen);
    CHECK_P11_ERROR(rv);

    *ppSign = pSign;
    *pSignLen = signLen;

    return CKR_OK;
}

void mlsGetECDSAPubkey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hPublicKey)
{
    CK_BYTE *params = NULL;
    CK_ULONG params_len = 0;

    CK_BYTE *point = NULL;
    CK_ULONG point_len = 0;

    CK_ATTRIBUTE* pAttr = NULL;

    CK_ATTRIBUTE publicKeyGetTpl[] =
    {
        { CKA_ECDSA_PARAMS, params, 0 },
        { CKA_EC_POINT, point, 0 },
    };
    CK_COUNT publicKeyGetTplSize = sizeof(publicKeyGetTpl) / sizeof(CK_ATTRIBUTE);

    C_GetAttributeValue(hSession, hPublicKey, publicKeyGetTpl, publicKeyGetTplSize);

    pAttr = FindAttribute(CKA_ECDSA_PARAMS, publicKeyGetTpl, publicKeyGetTplSize);
    params_len = pAttr->valueLen;
    params = malloc(pAttr->valueLen);
    pAttr->pValue = params;

    pAttr = FindAttribute(CKA_EC_POINT, publicKeyGetTpl, publicKeyGetTplSize);
    point_len = pAttr->valueLen;
    point = malloc(pAttr->valueLen);
    pAttr->pValue = point;

    C_GetAttributeValue(hSession, hPublicKey, publicKeyGetTpl, publicKeyGetTplSize);
    dump_hex("param dump:", params, params_len);
    dump_hex("Point dump:", point, point_len);
}
