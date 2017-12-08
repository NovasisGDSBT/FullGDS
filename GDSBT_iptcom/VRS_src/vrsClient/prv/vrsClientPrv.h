/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsClientPrv.h
*
* DESCRIPTION : 
*
********************************************************************************
* HISTORY  :
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

#ifndef VRSCLIENTPRV_H
#define VRSCLIENTPRV_H

/*******************************************************************************
*  INCLUDES
*/


/*******************************************************************************
*  DEFINES
*/
#define VRS_MAX_NO_GROUPS 4


/*******************************************************************************
*  TYPEDEFS
*/
typedef struct {
    VRS_FULL_LABEL source;
    VRS_FULL_LABEL dest;
} VRSCLIPRV_DEST_CONFIG;

typedef struct {
    INT32 threadPolicy;
    INT32 threadPriority;
    INT32 threadStackSize;
    char  rootPath[VRS_MAX_PATH_LEN];
    char  configFile[VRS_MAX_PATH_LEN];
    VRS_FULL_LABEL sourceUri;
    VRSCLIPRV_DEST_CONFIG destConfig[VRS_MAX_NO_GROUPS];
} VRSCLIPRV_CONFIG;

typedef enum
{
    Undefined       = 0,
    General         = 0x03,
    ReportCallback  = 0x0C,
    RequestCallback = 0x30
} VRSCLIPRV_UNION_TYPE;

typedef struct
{
    VRS_REPORT_CALLBACK  pCallback;
    void*                pCallbackRef;
} VRSCLIPRV_REP_INFO_CLB;

typedef struct
{
    UINT16               maxReplySize;
    VRS_REQUEST_TYPE     requestType;
    char*                pFilterUri;
    VRS_REQUEST_CALLBACK pCallback;
    void*                pCallbackRef;
    char*                pDestURI;
} VRSCLIPRV_REQ_INFO_CLB;

typedef struct
{
    DNODE           node;       /* NOTE: node must be first entry */
    UINT32          UID;        /* unique identifier */
    UINT16          SID;        /* service id */
    VRSCLIPRV_UNION_TYPE uType; /* type of union */
    union
    {
        VRSCLIPRV_REP_INFO_CLB  ciRep; /* Report callback information */
        VRSCLIPRV_REQ_INFO_CLB  ciReq; /* Request callback information */
    } u;
} VRSCLIPRV_REQUEST;


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/

#endif /* VRSCLIENTPRV_H */
/******************************************************************************/
