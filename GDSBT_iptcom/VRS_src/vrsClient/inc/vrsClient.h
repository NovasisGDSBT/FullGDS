/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsClient.h
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

/* DOCGROUP: vrsTypedefCmnApiDescription
*\latex
* This section lists the most commonly used VRS specific structure and types,
* for more information see the VRS header files.
*\endlatex
*/
#ifndef VRSCLIENT_H
#define VRSCLIENT_H

/*******************************************************************************
*  INCLUDES
*/
#include "vrs.h"


/*******************************************************************************
*  DEFINES
*/


/*******************************************************************************
*  TYPEDEFS
*/

typedef enum
{
    Undef    = 0x00,
    All      = 0x03,
    All2     = 0x0A,
    UriOnly  = 0x0c,
    NrOf     = 0x30,
    NrOfFrom = 0xc0
} VRS_REQUEST_TYPE;


typedef void (*VRS_REQUEST_CALLBACK) (
    VRS_REQUEST_TYPE reqType, /* request type (All/All2/UriOnly/NrOf/NrOfFrom) */
    VRS_RESULT       result,  /* result code 0 ok otherwise error */
    UINT16           noLeft,  /* nr of matching entries left to read */
    char*            pUri,    /* pointer to uri string (null on errors) */
    char*            pData,   /* pointer to data or null if only reading uris */
    void*            pRef);   /* pointer to callback reference */

/* 
* DOCGROUP: vrsCmnApiTypedef 
*/
typedef void (*VRS_REPORT_CALLBACK) (
    VRS_RESULT       result,  /* result code 0 ok otherwise error */
    void*            pRef);   /* pointer to callback reference */


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/

extern VRS_RESULT VRSClient_initiate(const char* pRootPath);
extern VRS_RESULT  VRSClient_report(const char*         pUri, 
                                    const char*         pData,
                                    VRS_REPORT_CALLBACK pCallback,
                                    void*               pCallbackRef);
extern VRS_RESULT  VRSClient_getReadyTime(UINT32* pReadyTime);
extern VRS_RESULT  VRSClient_getErrorCode(void);
extern const char* VRSClient_getErrorString(VRS_RESULT errorCode);
extern UINT32      VRSClient_getVersion(void);
extern VRS_RESULT  VRSClient_checkVersion(UINT32 vrsVRUE);
extern VRS_SERVICE_STATUS VRSClient_getStatus(void);
extern UINT32      VRSClient_getServerProtocolVersion(void);
extern VRS_RESULT  VRSClient_terminate(UINT32 remainingTime);

#endif /* VRSCLIENT_H */
/******************************************************************************/
