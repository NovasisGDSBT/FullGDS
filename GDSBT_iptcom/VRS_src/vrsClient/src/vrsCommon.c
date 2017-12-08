/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsCommon.c
*
* DESCRIPTION :
*
********************************************************************************
* HISTORY  :
*
*******************************************************************************/


/*******************************************************************************
*  INCLUDES
*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "iptcom.h"
#include "vos.h"
//#include <tdcSyl.h>
#include "tdcApi.h"
#include "vrsCommon.h"
//#include "vrsDbgUtil.h"
//#include "vrsTstUtil.h"
//#include "nMon.h"

#include "dledsPlatformDefines.h"
/*******************************************************************************
* DEFINES
*/
#define VRS_COMP_NAME   "vrsCommon" /* only declare in c file */


/*******************************************************************************
* MACROS
*/
#define ADD_2_SERVICE_STATE_TABLE(sStateArg) {sStateArg, #sStateArg}
/* remember to add new states to this table */
#define SERVICE_STATE_TABLE \
        { \
            ADD_2_SERVICE_STATE_TABLE(VRS_SERVICE_STOPPED ), \
            ADD_2_SERVICE_STATE_TABLE(VRS_SERVICE_STOPPING), \
            ADD_2_SERVICE_STATE_TABLE(VRS_SERVICE_STARTING), \
            ADD_2_SERVICE_STATE_TABLE(VRS_SERVICE_RUNNING ), \
            ADD_2_SERVICE_STATE_TABLE(VRS_SERVICE_ERROR   ), \
        }


/*******************************************************************************
* TYPEDEFS
*/
typedef struct
{
    VRS_RESULT errCode;
    char       *errStr;
} VRS_ERR_STRUCT;

typedef struct
{
    VRS_SERVICE_STATUS  stat;
    char*               str;
} VRS_STAT_STRUCT;

typedef enum
{
    Init = 0,
    ElemName,
    ElemDelim,
    ElemInst,
    AtDelim,
    DevId,
    DevDelim,
    CarId,
    CarDelim,
    CstId,
    Valid,
    Error
} VRS_DATA_URI_FQDN;


/*******************************************************************************
* GLOBALS
*/


/*******************************************************************************
* LOCALS
*/
/* constants */
static const char serviceStatusUndefined[] = "UNKNOWN";
static const VRS_STAT_STRUCT serviceStatusTable[] = SERVICE_STATE_TABLE;
static const char errorTextUndefined[] = "Unknown Error Code";
static const VRS_ERR_STRUCT vrsErrorDescriptions[] =
{
    {VRS_OK                     ,   "OK"                        },
    {VRS_ERROR                  ,   "Unspecified error"         },
    {VRS_WARNING                ,   "Unspecified warning"       },
    {VRS_EXCEPTION              ,   "Unspecified exception"     },
    {VRS_COMP_ALREADY_STARTED   ,   "Component Already Started" },
    {VRS_COMP_WRONG_VERSION     ,   "Wrong Component Version"   },
    {VRS_COMP_NOT_STARTED       ,   "Component Not Started"     },
    {VRS_COMP_CLI_NOT_STARTED   ,   "Client Not Started"        },
    {VRS_COMP_SRV_NOT_STARTED   ,   "Server Not Started"        },
    {VRS_COMMUNICATION_ERROR    ,   "Communication Error"       },
    {VRS_COMMUNICATION_TIMEOUT  ,   "Communication Timeout"     },
    {VRS_COMMUNICATION_INVALID  ,   "Communication Invalid"     },
    {VRS_MEMORY_ALLOCATION_ERROR,   "Memory Allocation Failed"  },
    {VRS_MEMORY_POINTER_ERROR   ,   "Pointer Error"             },
    {VRS_MEMORY_INVALID         ,   "Memory Invalid"            },
    {VRS_SEMAPHORE_NOT_CREATED  ,   "Semaphore Create Error"    },
    {VRS_SEMAPHORE_TAKEN        ,   "Semaphore Taken"           },
    {VRS_SEMAPHORE_ERROR        ,   "Semaphore Error"           },
    {VRS_THREAD_NOT_CREATED     ,   "Failed to Create Thread"   },
    {VRS_THREAD_ERROR           ,   "Thread Error"              },
    {VRS_FILE_ACCESS_ERROR      ,   "File Access Error"         },
    {VRS_FILE_CONFIG_ERROR      ,   "Configuration File Error"  },
    {VRS_DATA_INVALID           ,   "Invalid Data"              },
    {VRS_DATA_BAD_SIZE          ,   "Bad Data Size"             },
    {VRS_DATA_NOT_FOUND         ,   "No Matching Data Found"    },
    {VRS_DATA_DUPLICATED        ,   "Data is Duplicated"        },
    {VRS_DATA_STRING_ERROR      ,   "String Faulty"             },
    {VRS_MESSAGE_ERROR          ,   "Message Error"             },
    {VRS_MESSAGE_BAD_HEADER     ,   "Bad Message Header"        },
    {VRS_MESSAGE_WRONG_VERSION  ,   "Wrong Message Version"     },
};
/* variables */
static T_IPT_LABEL locDevId = "";
static T_IPT_LABEL locCarId = "";
static T_IPT_LABEL locCstId = "";
static VRS_SERVICE_STATUS   locServiceStatus  = VRS_SERVICE_STOPPED;
static IPT_SEM              locSemaphore;       /* protects local Ids */


/*******************************************************************************
* LOCAL FUNCTIONS
*/


/*******************************************************************************
* DOCGROUP: vrsCmnInternalGroup
*
* DESCRIPTION:
*   Checks it the parameter is a mark charachter according to the the IPT
*   addressing concept (3EGM019001-0021).
*
* RETURNS:
*   Returns a non-zero value for any of characters -, _, !, ~, *, ', ( and ).
*   For all other characters are 0 returned.
*/
static int isMarkChar(
   int c)
{
    switch(c)
    {
        case '-':
            break;
        case '_':
            break;
        case '!':
            break;
        case '~':
            break;
        case '*':
            break;
        case 0x27:  /* ' */
            break;
        case '(':
            break;
        case ')':
            break;
        default:
            return 0;
    }
    return c;
}


/*******************************************************************************
* DOCGROUP: vrsCmnInternalGroup
*
* DESCRIPTION:
*   Checks format of the URI string.
*
* RETURNS:
*   Returns Error if any inconsistency is detected, Valid if the URI is complete
*   otherwize the last FQDN found in the URI.
*/
static VRS_DATA_URI_FQDN evaluateURI(
    const char* pUri) /* pointer to uri */
{
    VRS_DATA_URI_FQDN fqdn = Init;

    /* validate input */
    if(pUri == NULL)
    {
        fqdn = Error;
    }
    else if(*pUri == 0)
    {
        fqdn = Error;
    }
    else if(strlen(pUri) > VRS_MAX_FULL_LABEL_LEN)
    {
        fqdn = Error;
    }
    else
    {
        const char* pTmp = pUri;

        while(*pTmp != 0)
        {
            switch(fqdn)
            {
                case Init:
                    if(isalpha(*pTmp) == 0)
                        fqdn = Error;
                    else
                        fqdn = ElemName;
                    break;
                case ElemName:
                    if(*pTmp == '.')
                        fqdn = ElemDelim;
                    else if(isalnum(*pTmp) != 0 || isMarkChar(*pTmp) != 0)
                        ;   /* do nada */
                    else
                        fqdn = Error;
                    break;
                case ElemDelim:
                    if(isalnum(*pTmp) == 0)
                        fqdn = Error;
                    else
                        fqdn = ElemInst;
                    break;
                case ElemInst:
                    if(*pTmp == '@')
                        fqdn = AtDelim;
                    else if(isalnum(*pTmp) != 0 || isMarkChar(*pTmp) != 0)
                        ;   /* do nada */
                    else
                        fqdn = Error;
                    break;
                case AtDelim:
                    if(isalnum(*pTmp) == 0)
                        fqdn = Error;
                    else
                        fqdn = DevId;
                    break;
                case DevId:
                    if(*pTmp == '.')
                        fqdn = DevDelim;
                    else if(isalnum(*pTmp) != 0 || isMarkChar(*pTmp) != 0)
                        ;   /* do nada */
                    else
                        fqdn = Error;
                    break;
                case DevDelim:
                    if(isalnum(*pTmp) == 0)
                        fqdn = Error;
                    else
                        fqdn = CarId;
                    break;
                case CarId:
                    if(*pTmp == '.')
                        fqdn = CarDelim;
                    else if(isalnum(*pTmp) != 0 || isMarkChar(*pTmp) != 0)
                        ;   /* do nada */
                    else
                        fqdn = Error;
                    break;
                case CarDelim:
                    if(isalnum(*pTmp) == 0)
                        fqdn = Error;
                    else
                        fqdn = CstId;
                    break;
                case CstId:
                    if(*pTmp == '.')
                        fqdn = Error;
                    else if(isalnum(*pTmp) != 0 || isMarkChar(*pTmp) != 0)
                        ;   /* do nada */
                    else
                        fqdn = Error;
                    break;
                case Valid:
                    return VRS_DATA_INVALID;
                case Error:
                    return VRS_DATA_STRING_ERROR;
                default:
                    return VRS_DATA_INVALID;
            }
            ++pTmp;
        }
        if(fqdn == CstId)
        {
            /* the uri ended with consist id */
            fqdn = Valid;
        }
    }
    return fqdn;
}


/*******************************************************************************
* DOCGROUP: vrsCmnInternalGroup
*
* DESCRIPTION:
*   Checks that character is within range.
*
* RETURNS:
*   Value if withing range, otherwize 0.
*/
static int checkRange(
    UINT8 value,  /* reference value */
    UINT8 minVal, /* min value */
    UINT8 maxVal) /* max value */
{
    if(value < minVal)
    {
        return 0;
    }
    else if(value > maxVal)
    {
        return 0;
    }
    return value;
}


/*******************************************************************************
* DOCGROUP: vrsCmnInternalGroup
*
* DESCRIPTION:
*   Checks that the data is in UTF-8 format (for more information see RFC 3629).
*
* RETURNS:
*   TRUE if data format is valid, else FALSE.
*/
static BOOL isValidUTF8Format(
    const char* pData) /* pointer to uri */
{
    if(pData == 0)
    {
        return FALSE;
    }
    else
    {
        const char* pChar = pData;

        while(*pChar != 0)
        {
            /* UTF8-1 */
            if(checkRange(*pChar, 0x00, 0x7F)) {
                ++pChar;
            }
            /* UTF8-2 */
            else if(checkRange(*pChar, 0xC2, 0xDF) && checkRange(*(pChar+1), 0x80, 0xBF)) {
                pChar += 2;
            }
            /* UTF8-3 */
            else if((UINT8)*pChar == 0xE0 && checkRange(*(pChar+1), 0xA0, 0xBF) && checkRange(*(pChar+2), 0x80, 0xBF)) {
                pChar += 3;
            }
            else if(checkRange(*pChar, 0xE1, 0xEC) && checkRange(*(pChar+1), 0x80, 0xBF)  && checkRange(*(pChar+2), 0x80, 0xBF)) {
                pChar += 3;
            }
            else if((UINT8)*pChar == 0xED && checkRange(*(pChar+1), 0x80, 0x9F) && checkRange(*(pChar+2), 0x80, 0xBF)) {
                pChar += 3;
            }
            else if(checkRange(*pChar, 0xEE, 0xEF) && checkRange(*(pChar+1), 0x80, 0xBF)  && checkRange(*(pChar+2), 0x80, 0xBF)) {
                pChar += 3;
            }
            /* UTF8-4 */
            else if((UINT8)*pChar == 0xF0 && checkRange(*(pChar+1),0x90,0xBF) && checkRange(*(pChar+2),0x80,0xBF) && checkRange(*(pChar+3),0x80,0xBF)) {
                pChar += 4;
            }
            else if(checkRange(*pChar,0xF1,0xF3) && checkRange(*(pChar+1),0x80,0xBF) && checkRange(*(pChar+2),0x80,0xBF) && checkRange(*(pChar+3),0x80,0xBF)) {
                pChar += 4;
            }
            else if((UINT8)*pChar == 0xF4 && checkRange(*(pChar+1),0x80,0x8F) && checkRange(*(pChar+2),0x80,0xBF) && checkRange(*(pChar+3),0x80,0xBF)) {
                pChar += 4;
            }
            else {
                return FALSE;
            }
        }
    }
    return TRUE;
}


/*******************************************************************************
* DOCGROUP: vrsCmnInternalGroup
*
* DESCRIPTION:
*   Converts aCst,aCar,aDev,aInst and aElem to '*'.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
static VRS_RESULT convertAllTagsToWildcard(
    char* pUriMask)         /* pointer to uri mask */
{
    char* pTmpUriMask = NULL;
    char* pTmp = NULL;
    int i = 0, nrOfKeyWords = 0;
    unsigned int len = 0;

    static const char *keyWordMatrix[] =
    {
        VRS_LABEL_ALLELEMENT,
        VRS_LABEL_ALLINSTANCE,
        VRS_LABEL_ALLDEV,
        VRS_LABEL_ALLCAR,
        VRS_LABEL_ALLCST
    };

    /* validate input */
    if(pUriMask == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(strlen(pUriMask) == 0)
    {
        return VRS_DATA_STRING_ERROR;
    }

    pTmpUriMask = pUriMask;
    nrOfKeyWords = sizeof(keyWordMatrix)/sizeof(char*);

    for(i=0;i<nrOfKeyWords;i++)
    {
        pTmp = VRS_strStr(pTmpUriMask,keyWordMatrix[i]);
        do
        {
            if(pTmp != 0)
            {
                /* convert the aDev.... to a '*' */
                len = strlen(pTmpUriMask) - strlen(pTmp);
                pTmpUriMask = pTmpUriMask + len;
                len = strlen(pTmpUriMask + strlen(keyWordMatrix[i]));
                pTmpUriMask[0] = '*';
                pTmpUriMask = pTmpUriMask + 1;
                strncpy(pTmpUriMask, pTmpUriMask + strlen(keyWordMatrix[i])-1, len);
                pTmpUriMask[len] = '\0';

                /* search if there are more occurrences of the keyword */
                pTmp = VRS_strStr(pTmpUriMask,keyWordMatrix[i]);
            }
        }while(pTmp !=0);
    }
    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCmnInternalGroup
*
* DESCRIPTION:
*   Replaces first occurrence of string X in buffer with string Y.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
static VRS_RESULT replace(
    const char*    pStrX,   /* string to replace */
    const char*    pStrY,   /* string to insert */
    VRS_FULL_LABEL pBuffer) /* string buffer */
{
    char buffer[VRS_MAX_FULL_LABEL_LEN] = {0};
    unsigned int len = 0, tmpLen = 0;
    char* pTag = 0;
    char* pTmp = 0;

    if(pStrX == 0 || pStrY == 0 || pBuffer == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    /* search for local device tag */
    pTmp = pBuffer;
    len = 0;
    if((pTag = VRS_strStr(pBuffer, pStrX)) != 0)
    {
        /* copy pre string */
        tmpLen = (unsigned int)(pTag - pTmp);
        len += tmpLen;
        if(len >= VRS_MAX_FULL_LABEL_LEN)
        {
            return VRS_DATA_BAD_SIZE;
        }
        strncat(buffer, pTmp, tmpLen);
        pTmp += tmpLen;
        /* cópy new string */
        pTmp += strlen(pStrX);
        tmpLen = strlen(pStrY);
        len += tmpLen;
        if(len >= VRS_MAX_FULL_LABEL_LEN)
        {
            return VRS_DATA_BAD_SIZE;
        }
        strncat(buffer, pStrY, tmpLen);
        /* copy post string */
        tmpLen = strlen(pTmp);
        len += tmpLen;
        if(len >= VRS_MAX_FULL_LABEL_LEN)
        {
            return VRS_DATA_BAD_SIZE;
        }
        strncat(buffer, pTmp, tmpLen);
        /* copy temp to input buffer */
        strncpy(pBuffer, buffer, VRS_MAX_FULL_LABEL_LEN);
    }
    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCmnInternalGroup
*
* DESCRIPTION:
*   Check if URI matches the URI mask.
*   Note: Only the '*' character is considered to be a wildcard.
*   Note: The comparison is case insensitive.
*
* RETURNS:
*   TRUE if URI matches the filter, otherwise FALSE.
*/
static BOOL checkUri(
    const char* pUriMask,    /* pointer to uri mask */
    const char* pUri)        /* pointer to uri */
{
    BOOL equal = FALSE;
    char *pTempUriMask=NULL,*pTempUri=NULL;

    if(pUriMask == 0)
    {
        /* no uri mask has been supplied */
        return TRUE;
    }
    else if(strlen(pUriMask) == 0)
    {
        /* no uri masker string has been supplied */
        return TRUE;
    }
    else if(pUri == 0)
    {
        return FALSE;
    }
    else if(strlen(pUri) == 0)
    {
        return FALSE;
    }

    /* First make a quick check if they match exactly */
    if(VRS_strCmp(pUriMask, pUri, VRS_MAX_FULL_LABEL_LEN) == 0)
    {
        /* the entry matches exactly */
        return TRUE;
    }

    pTempUriMask = (char *)pUriMask;
    pTempUri  = (char *)pUri;

    /* Compare the entry URI against the URI mask */
    do
    {
        if(tolower(pTempUri[0]) == tolower(pTempUriMask[0]))
        {
            pTempUri++;
            pTempUriMask++;
        }
        else if(pTempUriMask[0] == '*')
        {
            /* First move a step */
            pTempUri++;
            pTempUriMask++;
            /* find the first non wildcard character */
            do
            {
                if(pTempUriMask[0] != '*')
                {
                    break;
                }
                pTempUriMask++;
            }while(strlen(pTempUriMask) > 0);

            if(strlen(pTempUriMask) == 0)
            {
                return TRUE;
            }
            else if(strlen(pTempUri) == 0)
            {
                return FALSE;
            }
            else
            {
                size_t len2Wildcard = 0;
                char* pTmp2StrStr = 0;
                unsigned int i;

                /* count chars to next wildcard and terminate uri mask */
                for(i = 0; i<strlen(pTempUriMask); ++i)
                {
                    if(pTempUriMask[i] == '*')
                    {
                        len2Wildcard = i;
                        pTempUriMask[len2Wildcard] = '\0';
                        break;
                    }
                }
                /* search for uri mask in uri */
                equal = FALSE;
                if((pTmp2StrStr = VRS_strStr(pTempUri, pTempUriMask)) != 0)
                {
                    equal = TRUE;
                    pTempUri = pTmp2StrStr;
                    pTempUri += strlen(pTempUriMask);
                }
                /* restore uri mask and move to next position */
                if(len2Wildcard > 0)
                {
                    pTempUriMask[len2Wildcard] = '*';
                    pTempUriMask += len2Wildcard;
                }
                else
                {
                    pTempUriMask += strlen(pTempUriMask);
                }
                /* check if end of uri mask has been reached */
                if(equal == (BOOL)FALSE || (strlen(pTempUri) > 0 && strlen(pTempUriMask) == 0))
                {
                    return FALSE;
                }
            }
        }
        else
        {
            return FALSE;
        }
    }while(strlen(pTempUri) > 0  && strlen(pTempUriMask) > 0 );

    if(strlen(pTempUriMask) > 0)
    {
        do
        {
            if(pTempUriMask[0] != '*')
            {
                break;
            }
            pTempUriMask++;
        }while(strlen(pTempUriMask));

        if(strlen(pTempUriMask) > 0)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    return TRUE;
}


/*******************************************************************************
* GLOBAL FUNCTIONS
*/


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   This function sets the first 2 bytes of dest to the 16-bit value, converting
*   the 16-bit value from host to big-endian byte order.
*
* RETURNS:
*   Nothing.
*/
void VRS_memSet16(
    void* pDst, /* pointer to destination */
    UINT16 val) /* character to set */
{
    UINT8* pTmp = pDst;

    if(pTmp == 0)
    {
        return;
    }
    *pTmp = (UINT8)((val & 0xFF00) >> 8);
    ++pTmp;
    *pTmp = (UINT8)(val & 0xFF);
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   This function sets the first 4 bytes of dest to the 32-bit value, converting
*   the 32-bit value from host to big-endian byte order.
*
* RETURNS:
*   Nothing.
*/
void VRS_memSet32(
    void* pDst, /* pointer to destination data */
    UINT32 val) /* data to be converted */
{
    UINT8* pTmp = pDst;

    if(pTmp == 0)
    {
        return;
    }
    *pTmp = (UINT8)((val & 0xFF000000) >> 24);
    ++pTmp;
    *pTmp = (UINT8)((val & 0x00FF0000) >> 16);
    ++pTmp;
    *pTmp = (UINT8)((val & 0x0000FF00) >> 8);
    ++pTmp;
    *pTmp = (UINT8)(val & 0xFF);
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   This function gets the first 2 bytes of source to the 16-bit value,
*   converting the 16-bit value from big-endian to host byte order.
*
* RETURNS:
*   A 16-bit he value in host byte order.
*/
UINT16 VRS_memGet16(
    void* pSrc) /* pointer to source data to be converted */
{
    UINT8* pTmp = (UINT8*)pSrc;
    UINT16 tmpVal = 0;

    if(pTmp == 0)
    {
        return 0;
    }
    tmpVal = ((*pTmp) << 8);
    ++pTmp;
    tmpVal = tmpVal + (*pTmp);
    return tmpVal;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   This function gets the first 4 bytes of source to the 32-bit value,
*   converting the 32-bit value from big-endian to host byte order.
*
* RETURNS:
*   A 32-bit he value in host byte order.
*/
UINT32 VRS_memGet32(
    void* pSrc) /* pointer to source data to be converted */
{
    UINT8* pTmp = pSrc;
    UINT32 tmpVal = 0;

    if(pTmp == 0)
    {
        return 0;
    }
    tmpVal = ((*pTmp) << 24);
    ++pTmp;
    tmpVal = tmpVal + ((*pTmp) << 16);
    ++pTmp;
    tmpVal = tmpVal + ((*pTmp) << 8);
    ++pTmp;
    tmpVal = tmpVal + (*pTmp);
    return tmpVal;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Gets a descriptive string for the service status.
*
* RETURNS:
*   Pointer to status string.
*/
const char* VRS_getServiceStatusString(
    VRS_SERVICE_STATUS serviceStatus)   /* service status to get string for */
{
    unsigned int i = 0;

    for(i=0; i < sizeof(serviceStatusTable)/sizeof(VRS_STAT_STRUCT); ++i)
    {
        if(serviceStatusTable[i].stat == serviceStatus)
        {
            return &serviceStatusTable[i].str[12];
        }
    }
    return serviceStatusUndefined;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Gets a descriptive string for the error code.
*
* RETURNS:
*   Pointer to error string.
*/
const char* VRS_getErrorString(
    VRS_RESULT errorCode)   /* error code to get description for */
{
    unsigned int i = 0;

    for (i=0; i < sizeof(vrsErrorDescriptions)/sizeof(VRS_ERR_STRUCT);i++)
    {
        if (vrsErrorDescriptions[i].errCode == errorCode)
        {
            return vrsErrorDescriptions[i].errStr;
        }
    }
    return errorTextUndefined;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Adds header information to a VRS Message (incl. bufferLength). Note that the
*   header does not contain the buffer.
*
* RETURNS:
*   Nothing.
*/
VRS_RESULT VRS_fillHeader(
    char* pHeader,      /* pointer to header to fill */
    UINT32 dataLength,  /* total length of packet including header */
    UINT16 serviceId,   /* service identity */
    UINT16 dataSize)    /* size of service specific part */
{

    if(pHeader == 0) {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(dataLength < VRS_HEADER_TOTAL_SIZE + dataSize) {
        return VRS_DATA_BAD_SIZE;
    }
    else {
        SetUINT8(pHeader, VRS_HEADER_VERSION_OFFSET, VRS_PROTOCOL_VERSION);
        SetUINT8(pHeader, VRS_HEADER_RELEASE_OFFSET, VRS_PROTOCOL_RELEASE);
        SetUINT8(pHeader, VRS_HEADER_UPDATE_OFFSET, VRS_PROTOCOL_UPDATE);
        SetUINT8(pHeader, VRS_HEADER_EVOLUTION_OFFSET, VRS_PROTOCOL_EVOLUTION);
        SetUINT16(pHeader, VRS_HEADER_SERVICEID_OFFSET, serviceId);
        SetUINT16(pHeader, VRS_HEADER_DATASIZE_OFFSET, dataSize);
    }
    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Validates header information to a VRS Message (incl. bufferLength).
*   Note that the header does not contain the buffer.
*
* RETURNS:
*   Nothing.
*/
VRS_RESULT VRS_validateHeader(
    const char* pHeader,    /* pointer to data */
    UINT32      dataLength, /* total length of data */
    UINT16*     pServiceId, /* optional pointer to service identity */
    UINT16*     pDataSize)  /* optional pointer to size of service data */
{
    UINT8 protVer, protRel, protUpd, protEvo;
    UINT16 serviceId, dataSize;

    /* validate arguments */
    if(pHeader == 0) {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(dataLength == 0)
    {
        return VRS_DATA_BAD_SIZE;
    }
    else if(dataLength < VRS_HEADER_TOTAL_SIZE) {
        return VRS_DATA_BAD_SIZE;
    }

    /* read header information */
    GetUINT8(&protVer, pHeader, VRS_HEADER_VERSION_OFFSET);
    GetUINT8(&protRel, pHeader, VRS_HEADER_RELEASE_OFFSET);
    GetUINT8(&protUpd, pHeader, VRS_HEADER_UPDATE_OFFSET);
    GetUINT8(&protEvo, pHeader, VRS_HEADER_EVOLUTION_OFFSET);
    GetUINT16(&serviceId, pHeader, VRS_HEADER_SERVICEID_OFFSET);
    GetUINT16(&dataSize, pHeader, VRS_HEADER_DATASIZE_OFFSET);

    /* validate header information */
    if(protVer != VRS_PROTOCOL_VERSION) {
        return VRS_MESSAGE_WRONG_VERSION;
    }
    else if(protRel > VRS_PROTOCOL_RELEASE) {
        return VRS_MESSAGE_WRONG_VERSION;
    }
    else if(serviceId == VRS_SID_UNDEFINED) {
        return VRS_MESSAGE_BAD_HEADER;
    }
    else if(dataLength != dataSize + VRS_HEADER_TOTAL_SIZE) {
        return VRS_DATA_BAD_SIZE;
    }
    /* set output */
    if(pServiceId != 0) {
        *pServiceId = serviceId;
    }
    if(pDataSize != 0) {
        *pDataSize = dataSize;
    }

    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Verifies that the URI is valid.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
VRS_RESULT VRS_validateUri(
    const char* pUri,   /* pointer to uri string */
    UINT16      uriLen) /* optional length */
{

    if(pUri == NULL) {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(strlen(pUri) == 0) {
        return VRS_DATA_STRING_ERROR;
    }
    else if(uriLen == 0 && strlen(pUri) > VRS_MAX_FULL_LABEL_LEN) {
        return VRS_DATA_STRING_ERROR;
    }
    else if(uriLen > 0 && strlen(pUri)+1 != uriLen) {
        return VRS_DATA_STRING_ERROR;
    }
    else if(evaluateURI(pUri) != Valid) {
        return VRS_DATA_STRING_ERROR;
    }
    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Verifies that the data is valid.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
VRS_RESULT VRS_validateData(
    const char* pData,      /* pointer to data string */
    UINT16      dataLen)    /* optional length */
{
    size_t len = 0;

    if(pData == 0) {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(*pData == 0) {
        return VRS_DATA_STRING_ERROR;
    }
    else if(dataLen > 0 && *(pData + dataLen - 1) != '\0') {
        return VRS_DATA_STRING_ERROR;
    }
    else if(isValidUTF8Format(pData) == (BOOL)FALSE) {
        return VRS_DATA_STRING_ERROR;
    }
    else if((len = strlen(pData) + 1) > VRS_MAX_DATA_LEN) {
        return VRS_DATA_BAD_SIZE;
    }
    else if(dataLen > 0 && len != dataLen) {
        return VRS_DATA_BAD_SIZE;
    }
    /*
     * the vrs server and client shall have no dependencies to the xml format
     * NOTE: for now shall no format verification be made
     * later on may it check for well formed xml but never validate the xml
     */

    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Gets the communication state.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
VRS_RESULT VRS_getCommunicationState(void)
{
    VRS_RESULT result = VRS_COMMUNICATION_ERROR;
    T_TDC_RESULT tdcRes;
    T_IPT_LABEL  devId;
    T_IPT_LABEL  carId;
    T_IPT_LABEL  cstId;

    /* check if first time executed */
    if(locServiceStatus == VRS_SERVICE_STOPPED)
    {
        locServiceStatus = VRS_SERVICE_STARTING;
        /* create semaphores */
        if (IPTVosCreateSem(&locSemaphore, IPT_SEM_FULL) != IPT_OK)
        {
            result = VRS_SEMAPHORE_NOT_CREATED;
            locServiceStatus = VRS_SERVICE_ERROR;
            return result;
        }
    }
    /* check communication status and set own ids */
    if (IPTVosGetSem(&locSemaphore, IPT_WAIT_FOREVER) != IPT_OK)
    {
        result = VRS_SEMAPHORE_ERROR;
    }
    else
    {
        /* check ipt status */
        if(IPTCom_getStatus() != IPTCOM_RUN)
        {
            result = VRS_COMMUNICATION_INVALID;
        }
        else if(IPTCom_isIPTDirEmulated())
        {
            strncpy(locDevId, VRS_LABEL_EMUDEV, VRS_MAX_LABEL_LEN);
            strncpy(locCarId, VRS_LABEL_EMUCAR, VRS_MAX_LABEL_LEN);
            strncpy(locCstId, VRS_LABEL_EMUCST, VRS_MAX_LABEL_LEN);
            locServiceStatus = VRS_SERVICE_RUNNING;
            result = VRS_OK;
        }
        else if((tdcRes = tdcGetOwnIds(devId, carId, cstId)) == TDC_OK)
        {
            strncpy(locDevId, devId, IPT_MAX_LABEL_LEN);
            strncpy(locCarId, carId, IPT_MAX_LABEL_LEN);
            strncpy(locCstId, cstId, IPT_MAX_LABEL_LEN);
            locServiceStatus = VRS_SERVICE_RUNNING;
            result = VRS_OK;
        }
        else
        {
            result = VRS_COMMUNICATION_INVALID;
        }
        IPTVosPutSem(&locSemaphore);
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Gets the own IDs.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
VRS_RESULT VRS_getOwnIds(
    VRS_LABEL devId,  /* device id string */
    VRS_LABEL carId,  /* car id string */
    VRS_LABEL cstId)  /* consist id string */
{
    VRS_RESULT result = VRS_COMMUNICATION_INVALID;

  #if (VRS_MAX_LABEL_LEN != IPT_MAX_LABEL_LEN)
    /* different data types in VRS_LABEL and T_IPT_LABEL should case warnings */
    #error VRS_MAX_LABEL_LEN differs from IPT_MAX_LABEL_LEN
  #endif

    /* check service status */
    if(locServiceStatus != VRS_SERVICE_RUNNING)
    {
        result = VRS_COMP_NOT_STARTED;
    }
    else if (IPTVosGetSem(&locSemaphore, IPT_WAIT_FOREVER) != IPT_OK)
    {
        result =  VRS_SEMAPHORE_ERROR;
    }
    else
    {
        if(devId != NULL)
        {
            strncpy(devId, locDevId, VRS_MAX_LABEL_LEN);
        }
        if(carId != NULL)
        {
            strncpy(carId, locCarId, VRS_MAX_LABEL_LEN);
        }
        if(cstId != NULL)
        {
            strncpy(cstId, locCstId, VRS_MAX_LABEL_LEN);
        }
        result = VRS_OK;
        IPTVosPutSem(&locSemaphore);
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Checks if versions are compatible.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
VRS_RESULT VRS_checkVersion(
    UINT32 vrsVRUE) /* full vrs version to check (VRS_SDK_VERSION_FULL) */
{
    if((VRS_SDK_VERSION != (vrsVRUE >> 24)) || (VRS_SDK_RELEASE < (0xFF & (vrsVRUE >> 16))))
    {
        return VRS_COMP_WRONG_VERSION;
    }
    else
    {
        return VRS_OK;
    }
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Converts carNN to CarID and cstNN to CstID.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
VRS_RESULT VRS_convertLabelsToIds(
    VRS_FULL_LABEL pFullLabel) /* pointer to full label buffer */
{
    T_IPT_LABEL carLabel = {0};
    T_IPT_LABEL carId = {0};
    VRS_RESULT result;
    char* pCar = 0;
    char* pCst = 0;

    /* validate input */
    if(pFullLabel == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(strlen(pFullLabel) == 0)
    {
        return VRS_DATA_STRING_ERROR;
    }
    /* check service status */
    else if (locServiceStatus != VRS_SERVICE_RUNNING)
    {
        return VRS_COMP_NOT_STARTED;
    }
    /* search for consist */
    if((pCst = VRS_strStr(pFullLabel, "cst")) != 0)
    {
        unsigned int cstLen = 0;

        cstLen = strlen(pCst);
        if(cstLen < 5)
        {
            pCst = 0;   /* cstNN can not fit */
        }
        else if(isdigit(pCst[3]) == 0 || isdigit(pCst[4]) == 0)
        {
            pCst = 0;   /* cstNN one of the position Ns is not a digit */
        }
        else if(cstLen == 5)
        {
            /* null is found after cstNN */
        }
        else if(cstLen > 5 && (pCst[5] != '.' && pCst[5] != '*'))
        {
            pCst = 0;   /* the positions after cstNN is not a dot or an ampersand */
        }
        else
        {
            /* the character after cstNN is '.' or '*' */
        }
        if(pCst != 0)
        {
            return VRS_DATA_INVALID;
        }
    }
    /* search for car */
    if((pCar = VRS_strStr(pFullLabel, "car")) != 0)
    {
        unsigned int carLen = 0;

        carLen = strlen(pCar);
        if(carLen < 5)
        {
            pCar = 0;   /* carNN can not fit */
        }
        else if(isdigit(pCar[3]) == 0 || isdigit(pCar[4]) == 0)
        {
            pCar = 0;   /* carNN one of the position Ns is not a digit */
        }
        else if(carLen == 5)
        {
            /* null is found after carNN */
        }
        else if(carLen > 5 && (pCar[5] != '.' && pCar[5] != '*'))
        {
            pCar = 0;   /* the positions after carNN is not a dot or an ampersand */
        }
        else
        {
            /* the character after carNN is '.' or '*' */
        }
        if(pCar != 0)
        {
            T_TDC_RESULT tdcResult = 0;
            UINT8 topoCnt = 0;

            strncpy(carLabel, pCar, 5);
            pCar = carLabel;
            if(IPTCom_isIPTDirEmulated())
            {
                strncpy(carId, VRS_LABEL_EMUCAR, VRS_MAX_LABEL_LEN);
                (void)tdcResult;
                (void)topoCnt;
            }
            else if((tdcResult = tdcLabel2CarId(carId, &topoCnt, pCst, carLabel)) == TDC_OK)
            {
            }
            else if(tdcResult == TDC_UNKNOWN_URI ||
                    tdcResult == TDC_NO_MATCHING_ENTRY ||
                    tdcResult == TDC_INVALID_LABEL)
            {
                return VRS_DATA_NOT_FOUND;
            }
            else
            {
                return VRS_COMMUNICATION_ERROR;
            }
            if((result = replace(carLabel, carId, pFullLabel)) != VRS_OK)
            {
                return result;
            }
        }
    }
    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Converts lDev, lCar and lCst to own device, car and consist IDs.
*
* RETURNS:
*   0 when ok, otherwise error code.
*/
VRS_RESULT VRS_convertLTagsToOwnIds(
    VRS_FULL_LABEL pFullLabel) /* pointer to full label buffer */
{
    VRS_LABEL ownDevId;
    VRS_LABEL ownCarId;
    VRS_LABEL ownCstId;
    VRS_RESULT result;

    /* validate input */
    if(pFullLabel == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(strlen(pFullLabel) == 0)
    {
        return VRS_DATA_STRING_ERROR;
    }
    else if((result = VRS_getOwnIds(ownDevId, ownCarId, ownCstId)) != VRS_OK)
    {
        return result;
    }
    /* search for local tags */
    if((result = replace(VRS_LABEL_LOCALDEV, ownDevId, pFullLabel)) != VRS_OK)
    {
    }
    else if((result = replace(VRS_LABEL_LOCALCAR, ownCarId, pFullLabel)) != VRS_OK)
    {
    }
    else if((result = replace(VRS_LABEL_LOCALCST, ownCstId, pFullLabel)) != VRS_OK)
    {
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Compares a uri string against a uri mask string.
*
* RETURNS:
*   0 if entry matches the uri mask, VRS_DATA_NOT_FOUND if nothing match
*   otherwise error code if an internal error was detected.
*/
VRS_RESULT VRS_compareUri(
    const char* pUriMask,   /* pointer to uri mask string */
    const char* pUri)       /* pointer to uri to check against(data or IPT-COM uri) */
{
    char buffUriMask[VRS_MAX_FULL_LABEL_LEN] = {0};
    char* pTmpUriMask = NULL;
    char* pTmp = NULL;
    unsigned int uriLen = 0;
    VRS_RESULT result;

    /* validate input */
    if(pUriMask == 0)
    {
        return VRS_OK;
    }
    else if(strlen(pUriMask) == 0)
    {
        return VRS_OK;
    }
    else if(pUri == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(strlen(pUri) == 0)
    {
        return VRS_DATA_BAD_SIZE;
    }

    /* first check if the uri mask consists of more that one mask*/
    pTmpUriMask = (char*)pUriMask;
    do
    {
        pTmp = strchr(pTmpUriMask,',');
        if(pTmp != 0)
        {
            uriLen= strlen(pTmpUriMask) - strlen(pTmp);
            strncpy(buffUriMask, pTmpUriMask, uriLen);
            pTmpUriMask = pTmpUriMask + uriLen + 1;
        }
        else
        {
            uriLen = strlen(pTmpUriMask);
            strncpy(buffUriMask, pTmpUriMask, uriLen);
            pTmpUriMask = pTmpUriMask + uriLen;
        }
        buffUriMask[uriLen] = '\0';
        /* convert the mask(s) to only use wild card (*) character */
        if((result = convertAllTagsToWildcard(buffUriMask)) != VRS_OK)
        {
            return result;
        }
        /* convert local tags to own ids */
        if((result = VRS_convertLTagsToOwnIds(buffUriMask)) != VRS_OK)
        {
            return result;
        }
        /* convert labels to ids */
        if((result = VRS_convertLabelsToIds(buffUriMask)) != VRS_OK)
        {
            return result;
        }
        /* compare the uri mask against the uri */
        if(checkUri(buffUriMask, pUri) == (BOOL)TRUE)
        {
            return VRS_OK;
        }
    } while(strlen(pTmpUriMask) > 0);
    return VRS_DATA_NOT_FOUND;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
*   Validates that the URI parameter contains element name and element instance.
*   Appends any missing own ids to the data URI.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRS_appendOwnIds2Uri(
    VRS_FULL_LABEL pFullLabel)  /* pointer to full label buffer */
{
    VRS_RESULT result = VRS_OK;

    /* validate input */
    if(pFullLabel == NULL)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(*pFullLabel == 0)
    {
        return VRS_DATA_STRING_ERROR;
    }
    else if(strlen(pFullLabel) > VRS_MAX_FULL_LABEL_LEN)
    {
        return VRS_DATA_BAD_SIZE;
    }
    else
    {
        size_t lenLeft = VRS_MAX_FULL_LABEL_LEN - strlen(pFullLabel);
        VRS_DATA_URI_FQDN fqdn = evaluateURI(pFullLabel);
        char* pTmp = pFullLabel + strlen(pFullLabel);
        VRS_LABEL ownDevId;
        VRS_LABEL ownCarId;
        VRS_LABEL ownCstId;

        /* validate fqdn */
        if(fqdn < ElemInst || fqdn == Error)
        {
            result = VRS_DATA_STRING_ERROR;
        }
        else if((result = VRS_getOwnIds(ownDevId, ownCarId, ownCstId)) != VRS_OK)
        {
        }
        else if(fqdn == ElemInst)
        {
            if(snprintf(pTmp, lenLeft, "@%s.%s.%s", ownDevId, ownCarId, ownCstId) >= (int)lenLeft)
            {
                result = VRS_DATA_BAD_SIZE;
            }
        }
        else if(fqdn == AtDelim)
        {
            if(snprintf(pTmp, lenLeft, "%s.%s.%s", ownDevId, ownCarId, ownCstId) >= (int)lenLeft)
            {
                result = VRS_DATA_BAD_SIZE;
            }
        }
        else if(fqdn == DevId)
        {
            if(snprintf(pTmp, lenLeft, ".%s.%s", ownCarId, ownCstId) >= (int)lenLeft)
            {
                result = VRS_DATA_BAD_SIZE;
            }
        }
        else if(fqdn == DevDelim)
        {
            if(snprintf(pTmp, lenLeft, "%s.%s", ownCarId, ownCstId) >= (int)lenLeft)
            {
                result = VRS_DATA_BAD_SIZE;
            }
        }
        else if(fqdn == CarId)
        {
            if(snprintf(pTmp, lenLeft, ".%s", ownCstId) >= (int)lenLeft)
            {
                result = VRS_DATA_BAD_SIZE;
            }
        }
        else if(fqdn == CarDelim)
        {
            if(snprintf(pTmp, lenLeft, "%s", ownCstId) >= (int)lenLeft)
            {
                result = VRS_DATA_BAD_SIZE;
            }
        }
        else
        {
            /* do nada uri is complete */
        }
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
* Case insensitive version of strncmp.
* Compares up to num characters of the string str1 to those of the string str2.
* This function starts comparing the first character of each string. If they
* are equal to each other, it continues with the following pairs until the
* characters differ, until a terminating null-character is reached, or until
* num characters match in both strings, whichever happens first.
*
* RETURNS:
* Returns an integral value indicating the relationship between the strings:
* A zero value indicates that the characters compared in both strings are all
* equal. A value greater than zero indicates that the first character that does
* not match has a greater value in str1 than in str2; And a value less than
* zero indicates the opposite.
*/
int VRS_strCmp(
    const char* str1,  /* string to be compared */
    const char* str2,  /* string to be compared */
    size_t      num)   /* Maximum number of characters to compare */
{
    size_t i = 0;

    /* check for null */
    if(str1 == NULL && str2 == NULL)
    {
        return 0;
    }
    else if(str1 == NULL)
    {
        return -1;
    }
    else if(str2 == NULL)
    {
        return 1;
    }

    /* compare the characters in the strings */
    for(i=0; i < num; i++)
    {
        if(str1[i] == 0 && str2[i] == 0)
        {
            return 0;
        }
        else if(str1[i] == 0)
        {
            return -1;
        }
        else if(str2[i] == 0)
        {
            return 1;
        }
        else if(tolower(str1[i]) != tolower(str2[i]))
        {
            if(((int)tolower(str1[i])) > ((int)tolower(str2[i])))
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
    }
    return 0;
}


/*******************************************************************************
* DOCGROUP: vrsCmnPrvGroup
*
* DESCRIPTION:
* Case insensitive version of strstr.
*
* RETURNS:
* A pointer to the first occurrence in pStringToBeSearched of any of the entire
* sequence of characters specified in pSubstringToSearchFor, or a null pointer
* if the sequence is not present in pStringToBeSearched.
*/
char* VRS_strStr(
    char* pStringToBeSearched,          /* string to be searched */
    const char* pSubstringToSearchFor)  /* substring to search for */
{
    size_t i = 0,y = 0;
    size_t lenStringToBeSearched = 0;
    size_t lenStringToBeSearchedFor = 0;
    char *pos = NULL;

    /* verify parameters */
    if (pStringToBeSearched   == NULL ||
        pSubstringToSearchFor == NULL)
    {
        return pStringToBeSearched;
    }

    /* get length of the strings */
    lenStringToBeSearchedFor = strlen(pSubstringToSearchFor);
    lenStringToBeSearched    = strlen(pStringToBeSearched);

    /* check for empty substring  */
    if(lenStringToBeSearchedFor == 0)
    {
        /* return input*/
        return pStringToBeSearched;
    }
    else if(lenStringToBeSearchedFor > lenStringToBeSearched)
    {
        /*
        string to be searched for is longer than the string to
        be searched, so the sub stirng can not be found.
        */
        return NULL;
    }

    for(i = 0; i < lenStringToBeSearched; i++)
    {
        if(tolower(pStringToBeSearched[i]) == tolower(pSubstringToSearchFor[y]))
        {
            /* the character i in both strings matches */
            if(y == 0)
            {
                pos = pStringToBeSearched + i;
            }
            y++;
            if(y == lenStringToBeSearchedFor)
            {
                return pos;
            }
        }
        else if(tolower(pStringToBeSearched[i]) == tolower(pSubstringToSearchFor[0]))
        {
            pos = pStringToBeSearched + i;
            y = 1;
        }
        else
        {
             /* the character i in the strings don't  match */
            y = 0;
            pos = NULL;
        }
    }

    return NULL;
}



