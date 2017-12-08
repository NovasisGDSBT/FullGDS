/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsCommon.h
*
* DESCRIPTION : 
*
********************************************************************************
* HISTORY  :
* 
* 
*******************************************************************************/


/*******************************************************************************
* DOCUMENTATION INFORMATION */

/* DOCGROUP: VRSAPIComponent
*    VRS API Information
*/

/* DOCGROUP: VRSDesignComponent
*    VRS Design Description
*/

#ifndef VRSCOMMON_H
#define VRSCOMMON_H

/*******************************************************************************
*  INCLUDES
*/
#include "iptcom.h"
#include "vrs.h"


/*******************************************************************************
*  DEFINES
*/
/* protocol version */
#define VRS_PROTOCOL_VERSION    1
#define VRS_PROTOCOL_RELEASE    0
#define VRS_PROTOCOL_UPDATE     1
#define VRS_PROTOCOL_EVOLUTION  0

/* Component name definitions */
#define VRS_SERVER_NAME     "vrsServer" 
#define VRS_CLIENT_NAME     "vrsClient"

/* ComID definitions*/
#define VRS_CLIENT_COMID        211
#define VRS_SERVER_COMID        210

/* Default destination definitions */
#define VRS_SERVER_DEST     VRS_SERVER_NAME"@grpAll.aCar.lCst"
#define VRS_CLIENT_DEST     VRS_CLIENT_NAME"@grpAll.aCar.lCst"
#define VRS_CLIENT_LISTEN   VRS_CLIENT_DEST
/* 
* VRS Header
*       ------------------------------------------
*      | protocol version | serviceId | dataSize |
*      ------------------------------------------
* bytes         4              2           2
*/
#define VRS_HEADER_VERSION_OFFSET      0    /* 1 Byte protocol version */
#define VRS_HEADER_RELEASE_OFFSET      1    /* 1 Byte protocol revision */
#define VRS_HEADER_UPDATE_OFFSET       2    /* 1 Byte protocol update */
#define VRS_HEADER_EVOLUTION_OFFSET    3    /* 1 Byte protocol evolution */
#define VRS_HEADER_SERVICEID_OFFSET    4    /* 2 Bytes service ID */
#define VRS_HEADER_DATASIZE_OFFSET     6    /* 2 Bytes data size */
#define VRS_HEADER_TOTAL_SIZE          8u   /* 8 Bytes in total */

/*
* VRS ServiceID
* Client uses lower byte (e.g. 0x0002)
* and server higher byte (e.g. 0x0200)
* (all are Big Endian)
*/
#define VRS_SID_UNDEFINED                     0x0000
#define VRS_SID_WRITE_VERSION_DATA_REQUEST    0x0001
#define VRS_SID_WRITE_VERSION_DATA_REPLY      0x0100
#define VRS_SID_SERVER_READY_TIME_REQUEST     0x0002
#define VRS_SID_SERVER_READY_TIME_REPLY       0x0200
#define VRS_SID_READ_VERSION_DATA_REQUEST     0x0003
#define VRS_SID_READ_VERSION_DATA_REPLY       0x0300
#define VRS_SID_READ_URI_ONLY_REQUEST         0x0004
#define VRS_SID_READ_URI_ONLY_REPLY           0x0400
#define VRS_SID_READ_NR_OF_REQUEST            0x0005
#define VRS_SID_READ_NR_OF_REPLY              0x0500
#define VRS_SID_READ_VERSION_DATA_2_REQUEST   0x0006
#define VRS_SID_READ_VERSION_DATA_2_REPLY     0x0600
#define VRS_SID_SERVER_READY_TIME             VRS_SID_SERVER_READY_TIME_REPLY
#define VRS_SID_HEADER_ERROR                  0xffff

/* maximum length and sizes */
#define VRS_MAX_PATH_LEN        255
#define VRS_MAX_FULL_LABEL_LEN  VRS_MAX_PATH_LEN
#define VRS_MAX_DATA_LEN        65535
#define VRS_MAX_REPLY_SIZE      (60*1024)   /* limit 60k on css and nrtos */
/*
* NOTE: the total size of uri and data is limited by max reply size, the vrs
*       header and read version data reply message field sizes
*
* Read version data reply format (bytes):
* ------------------------------------------------------------------------------
* | header | result | noMatch | noEntries | uriLen | uriStr | dataLen | dataStr
* ------------------------------------------------------------------------------
*    8         4        2          2          2        X         2         X
*/
#define VRS_MAX_URI_N_DATA_SIZE (VRS_MAX_REPLY_SIZE-VRS_HEADER_TOTAL_SIZE*12)

/* thread defaults */
#define VRS_THREAD_STACK_SIZE   0x8000
#define VRS_THREAD_CYCLE        5000    /* cycle time in milli seconds */

/* configuration */
#define VRS_CONFIG_NAME         "vrsConfig"
#define VRS_CONFIG_FILE         VRS_CONFIG_NAME".xml"
#define VRS_CONFIG_ELEMENT_NAME VRS_CONFIG_NAME

/* label defines */
#define VRS_MAX_LABEL_LEN       IPT_MAX_LABEL_LEN   /* incl. terminating '\0' */
#define VRS_MAX_NO_LABELS       3
#define VRS_FULL_LABEL_LEN      (VRS_MAX_LABEL_LEN*VRS_MAX_NO_LABELS)

/* filter */
#define VRS_LABEL_ALLCST        "aCst"
#define VRS_LABEL_ALLCAR        "aCar"
#define VRS_LABEL_ALLDEV        "aDev"
#define VRS_LABEL_ALLINSTANCE   "aInst"
#define VRS_LABEL_ALLELEMENT    "aElem"

/* local tags */
#define VRS_LABEL_LOCALCST      "lCst"
#define VRS_LABEL_LOCALCAR      "lCar"
#define VRS_LABEL_LOCALDEV      "lDev"

/* default ids for labels when iptcom is emulated */
#define VRS_LABEL_EMUCST        "cstIdNA"
#define VRS_LABEL_EMUCAR        "carIdNA"
#define VRS_LABEL_EMUDEV        "devIdNA"

/* time-out values */
#define VRS_NO_WAIT         0
#define VRS_WAIT_FOREVER    -1


/*******************************************************************************
*  TYPEDEFS
*/
typedef char VRS_LABEL[VRS_MAX_LABEL_LEN];
typedef char VRS_FULL_LABEL[VRS_MAX_FULL_LABEL_LEN];

/*******************************************************************************
*  MACROS
*/


/*******************************************************************************
* NODOCGROUP: 
*
* DESCRIPTION:
*   Macros for set and get UINT8, UINT16 and UINT32 between UINT8 buffers.
*   To avoid alignment and big/little endian problems. Corresponding function
*   declarations:
* \latex
* \begin{verbatim}
*  void GetUINTnn(UINTnn* pDest, UINT8* pSource, UINT16 offset);
*  void SetUINTnn(UINT8*  pDest, UINT16 offset,  UINTnn dataVal);
* \end{verbatim}
* \endlatex
*
* RETURNS:
*   Nothing
*/
#define  SetUINT8(a, o, d) (*((UINT8*)((void*)((UINT8*)a + o))) = d)
#define SetUINT16(a, o, d) VRS_memSet16((UINT8*)a + o, (UINT16)d)
#define SetUINT32(a, o, d) VRS_memSet32((UINT8*)a + o, (UINT32)d)

#define  GetUINT8(d, a, o) (*d = (UINT8)*((UINT8*)((void*)((UINT8*)a + o))))
#define GetUINT16(d, a, o) (*d = VRS_memGet16((UINT8*)a + o))
#define GetUINT32(d, a, o) (*d = VRS_memGet32((UINT8*)a + o))


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/
extern const char* VRS_getServiceStatusString(VRS_SERVICE_STATUS serviceStatus);
extern const char* VRS_getErrorString(VRS_RESULT errorCode);
extern VRS_RESULT  VRS_fillHeader(char* pHeader, UINT32 dataLength,
                    UINT16 serviceId, UINT16 dataSize);
extern VRS_RESULT  VRS_validateHeader(const char* pHeader, UINT32 dataLength,
                       UINT16* pServiceId, UINT16* pDataSize);
extern VRS_RESULT  VRS_validateUri(const char* pUri, UINT16 uriLen);
extern VRS_RESULT  VRS_validateData(const char* pData, UINT16 dataLen);
extern VRS_RESULT  VRS_getCommunicationState(void);
extern VRS_RESULT  VRS_getOwnIds(VRS_LABEL devId, VRS_LABEL carId,
                       VRS_LABEL cstId);
extern VRS_RESULT  VRS_checkVersion(UINT32 vrsVRUE);
extern VRS_RESULT  VRS_convertLabelsToIds(VRS_FULL_LABEL pFullLabel);
extern VRS_RESULT  VRS_convertLTagsToOwnIds(VRS_FULL_LABEL pFullLabel);
extern VRS_RESULT  VRS_compareUri(const char* pUriMask, const char* pUri);
extern VRS_RESULT  VRS_appendOwnIds2Uri(VRS_FULL_LABEL pFullLabel);
extern void        VRS_memSet16(void* pDst, UINT16 val);
extern void        VRS_memSet32(void* pDst, UINT32 val);
extern UINT16      VRS_memGet16(void* pSrc);
extern UINT32      VRS_memGet32(void* pSrc);
extern int         VRS_strCmp(const char* str1, const char* str2, size_t num);
extern char*       VRS_strStr(char* pStringToBeSearched, 
                                    const char* pSubstringToSearchFor);

#endif /* VRSCOMMON_H */
/******************************************************************************/
