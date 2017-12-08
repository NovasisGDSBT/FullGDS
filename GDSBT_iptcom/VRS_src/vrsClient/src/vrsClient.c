/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsClient.c
*
* DESCRIPTION : 
*
********************************************************************************
* HISTORY  :
* 
*******************************************************************************/


/*******************************************************************************
* DOCUMENTATION INFORMATION */

/* DOCGROUP: vrsCliApiDescription
*\latex
* This API section contains all public functions of the client. For information 
* about which API functions that are of interest for Reporter and Requester 
* developers, see the Writing a Reporter and Writing a Requester in the general 
* information chapter.
*\endlatex
*/

/*******************************************************************************
*  INCLUDES
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "iptcom.h"
#include "vos.h"
#include "dList.h"
#include "vrsCommon.h"
#include "vrsClient.h"
#include "vrsClientPrv.h"
#include "vrsPlatform.h"


/*******************************************************************************
* DEFINES 
*/
#define VRS_CLIENT_MSG_QUEUE_LEN    64          /* Length of incoming message queue */
#define VRS_MAX_GEN_REPORTERS       255         /* Max generic reporters */


/*******************************************************************************
* TYPEDEFS
*/


/*******************************************************************************
* LOCALS
*/
static DLIST                locRequestList           = DLIST_INIT(&locRequestList);
static UINT32               locServerReadyTime       = 0;
static UINT32               locServerProtocolVersion = 0;
static MD_QUEUE             locMdQueue               = 0;
static VRS_SERVICE_STATUS   locServiceStatus         = VRS_SERVICE_STOPPED;
static char*                locPtrDestUri            = 0;
static VRSCLIPRV_CONFIG     locClientConfig          = {0,0,0,{0},{0},{0},
                                                       {{{0},{0}},{{0},{0}},{{0},{0}},{{0},{0}}}};
static IPT_SEM              locSemaphore;            /* protects locRequestList */
static VRS_RESULT           locErrorCode             = VRS_OK;
static UINT32               locNrRegGenReporters     = 0;
static VRS_RESULT           locGenReporterStatus[VRS_MAX_GEN_REPORTERS] = {0};


/*******************************************************************************
* GLOBALS 
*/


/*******************************************************************************
* LOCAL FUNCTIONS
*/


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Parses the configuration file and sets configuration parameters.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT parseConfigFile(void)
{
    VRS_RESULT result = VRS_OK;
    XML_LOCAL xLoc;
    char tag[MAX_TAGLEN+1];
    char attribute[MAX_TOKLEN+1];
    char value[MAX_TOKLEN+1];
    int valueInt,i;
   
    if(iptXmlOpen(&xLoc, locClientConfig.configFile) < 0)
    {
        return result;
    }
    iptXmlEnter(&xLoc);
    if(iptXmlSeekStartTag(&xLoc, VRS_CONFIG_ELEMENT_NAME) == 0)
    {
        iptXmlEnter(&xLoc);
        while(iptXmlSeekStartTagAny(&xLoc, tag, sizeof(tag)) == 0)
        {
            if(strcmp(tag, "client") == 0)
            {
                iptXmlEnter(&xLoc);
                for(i=0; i < VRS_MAX_NO_GROUPS; i++)
                {
                    if(iptXmlSeekStartTagAny(&xLoc, tag, sizeof(tag)) == 0)
                    {
                        if(strcmp(tag, "group") == 0)
                        {
                            iptXmlEnter(&xLoc);
                            if(iptXmlSeekStartTag(&xLoc, "source") == 0)
                            {
                                 /* get attribute data */
                                if(iptXmlGetAttribute(&xLoc, attribute, &valueInt, value) != TOK_ATTRIBUTE)
                                {
                                    result = VRS_FILE_CONFIG_ERROR;
                                    break;
                                }
                                else if(strcmp(attribute, "uri") != 0)
                                {
                                    result = VRS_FILE_CONFIG_ERROR;
                                    break;
                                }
                                else
                                {
                                    strncpy(locClientConfig.destConfig[i].source, value, VRS_MAX_FULL_LABEL_LEN);
                                    locClientConfig.destConfig[i].source[VRS_MAX_FULL_LABEL_LEN-1] = '\0';
                                }
                            }
                            else
                            {
                                result = VRS_FILE_CONFIG_ERROR;
                                break;
                            }

                            if(iptXmlSeekStartTag(&xLoc, "dest") == 0)
                            {
                                /* get attribute data */
                                if(iptXmlGetAttribute(&xLoc, attribute, &valueInt, value) != TOK_ATTRIBUTE)
                                {
                                    result = VRS_FILE_CONFIG_ERROR;
                                    break;
                                }
                                else if(strcmp(attribute, "uri") != 0)
                                {
                                    result = VRS_FILE_CONFIG_ERROR;
                                    break;
                                }
                                else
                                {
                                    strncpy(locClientConfig.destConfig[i].dest, value, VRS_MAX_FULL_LABEL_LEN);
                                    locClientConfig.destConfig[i].dest[VRS_MAX_FULL_LABEL_LEN-1] = '\0';
                                }
                            }
                            else
                            {
                                result = VRS_FILE_CONFIG_ERROR;
                                break;
                            }
                            iptXmlLeave(&xLoc);
                        } /* END group */
                    }
                    else
                    {
                        break;
                    }
                } /* END for */
                iptXmlLeave(&xLoc);
            } /* END client */
        } /* END while */
        iptXmlLeave(&xLoc);
    }
    else
    {
        result = VRS_FILE_CONFIG_ERROR;
    }
    if(iptXmlClose(&xLoc)!= 0)
    {
    }

    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Sets default configuration parameters and reads the VRS configuration file
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT setConfiguration(
    const char* pRootPath)  /* path to where the component shall r/w files */
{
    /* set application root directory */
    if(pRootPath == 0)
    {
        strncpy(locClientConfig.rootPath, "", VRS_MAX_PATH_LEN);
    }
    else if(strlen(pRootPath) == 0)
    {
        strncpy(locClientConfig.rootPath, "", VRS_MAX_PATH_LEN);
    }
    else
    {
        strncpy(locClientConfig.rootPath, pRootPath, VRS_MAX_PATH_LEN);
        if(locClientConfig.rootPath[strlen(locClientConfig.rootPath)-1] != '\\' &&
           locClientConfig.rootPath[strlen(locClientConfig.rootPath)-1] != '/')
        {
            strncat(locClientConfig.rootPath, "/", VRS_MAX_PATH_LEN);
        }
    }
    /* set source uri */
    strncpy(locClientConfig.sourceUri,VRS_CLIENT_NAME,VRS_MAX_FULL_LABEL_LEN);
    strncat(locClientConfig.sourceUri,"@lDev.lCar.lCst",VRS_MAX_FULL_LABEL_LEN);
    /* set config file */
    snprintf(locClientConfig.configFile, VRS_MAX_PATH_LEN,
             "%s"VRS_CONFIG_FILE, locClientConfig.rootPath);
    /* set thread parameters */
    locClientConfig.threadPolicy    = VRS_THREAD_POLICY;
    locClientConfig.threadPriority  = VRS_THREAD_PRIORITY_CLIENT;
    locClientConfig.threadStackSize = VRS_THREAD_STACK_SIZE;

    /* parse config file */
    return parseConfigFile();
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*    Checks if the client has been configured with a destination uri to the 
*    server and sets the client to use it. 
*    Used in the case of multiple servers in one consist.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static void setDestinationUri(void)
{
    VRS_RESULT result;
    VRS_LABEL myDevId;
    VRS_LABEL myCarId;
    VRS_LABEL myCstId;
    char myUri[VRS_FULL_LABEL_LEN] = {0};
    int i=0;

    if(VRS_getOwnIds(myDevId,myCarId,myCstId) != VRS_OK)
    {
        locServiceStatus = VRS_SERVICE_ERROR;
    }
    else
    {
        UINT8 grpCfgCntr = 0;

        /* construct the own device uri */
        strncat(myUri,myDevId,VRS_MAX_LABEL_LEN);
        strncat(myUri,".",1);
        strncat(myUri,myCarId,VRS_MAX_LABEL_LEN);
        strncat(myUri,".",1);
        strncat(myUri,myCstId,VRS_MAX_LABEL_LEN);

        for(i=0; i < VRS_MAX_NO_GROUPS; i++)
        {
            if(strlen(locClientConfig.destConfig[i].source) > 0 && strlen(locClientConfig.destConfig[i].dest) > 0)
            {
                ++grpCfgCntr;
                if((result = VRS_compareUri(locClientConfig.destConfig[i].source, myUri)) == VRS_OK)
                {
                    if((result = VRS_convertLabelsToIds(locClientConfig.destConfig[i].dest)) != VRS_OK)
                    {
                    }
                    else if((result = VRS_convertLTagsToOwnIds(locClientConfig.destConfig[i].dest)) != VRS_OK)
                    {
                    }
                    else
                    {
                        locPtrDestUri = locClientConfig.destConfig[i].dest;
                    }
                    break;
                }
                else if(result == VRS_DATA_NOT_FOUND)
                {
                    /* no match found continue */
                }
                else
                {
                    /* unexpected error */
                    break;
                }
            }
        }
        if(i >= VRS_MAX_NO_GROUPS && grpCfgCntr > 0 && locPtrDestUri == 0)
        {
        }
    }
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Remove Train tag from URI.
*
* RETURNS:
*   Nothing.
*/
static void truncateUri(
    char* pUri)   /* Null terminated data uri string */
{
    char* pTmp;
    int ii=0;

    pTmp = strchr(pUri,'.');
    while (pTmp != NULL)
    {
        ii++;
        if (ii == 4)
        {
            *pTmp = '\0';
        }
        pTmp=strchr(pTmp+1,'.');
    }
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Sets public error code.
*
* RETURNS:
*   Nothing.
*/
static void setErrorCode(
    VRS_RESULT errorCode)   /* error code primarily for communication errors */
{
    locErrorCode = errorCode;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Adds request information to the locRequestList list, used to keep track of 
*   outstanding requests.
*
* RETURNS:
*   Reference value > 0.
*/
static UINT32 addRequestInfo(
    UINT16 serviceID)    /* service id */
{
    VRSCLIPRV_REQUEST* pTmpPrv;
    UINT32 result = 0;  /* 0 means error */

    if (IPTVosGetSem(&locSemaphore, IPT_WAIT_FOREVER) != IPT_OK)
    {
    }
    else if(dListValidate(&locRequestList) != VRS_OK)
    {
        IPTVosPutSem(&locSemaphore);
    }
    else if((pTmpPrv = (VRSCLIPRV_REQUEST*)IPTVosMalloc(sizeof(VRSCLIPRV_REQUEST))) == 0)
    {
        IPTVosPutSem(&locSemaphore);
    }
    else
    {
        /* add information */
        memset(pTmpPrv, 0, sizeof(VRSCLIPRV_REQUEST));
        pTmpPrv->UID = IPTVosGetUniqueNumber();
        pTmpPrv->SID = serviceID;
        pTmpPrv->uType = General;

        if(dListAdd(&locRequestList, (DNODE*)pTmpPrv) != VRS_OK)
        {
            IPTVosFree((BYTE*)pTmpPrv);
        }
        else
        {
            result = pTmpPrv->UID;
        }
        IPTVosPutSem(&locSemaphore);
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Adds callback report information to the locRequestList list, used to keep 
*   track of outstanding reports.
*
* RETURNS:
*   Reference value > 0.
*/
static UINT32 addReportInfoCallback(
    UINT16               serviceID,    /* service id */
    VRS_REPORT_CALLBACK  pCallback,    /* Pointer to callback function */
    void*                pCallbackRef) /* Pointer to callback reference */
{
    VRSCLIPRV_REQUEST* pTmpPrv;
    UINT32 result = 0;  /* 0 means error */

    if (IPTVosGetSem(&locSemaphore, IPT_WAIT_FOREVER) != IPT_OK)
    {
    }
    else if(dListValidate(&locRequestList) != VRS_OK)
    {
        IPTVosPutSem(&locSemaphore);
    }
    else if((pTmpPrv = (VRSCLIPRV_REQUEST*)IPTVosMalloc(sizeof(VRSCLIPRV_REQUEST))) == 0)
    {
        IPTVosPutSem(&locSemaphore);
    }
    else
    {
        /* add information */
        memset(pTmpPrv, 0, sizeof(VRSCLIPRV_REQUEST));
        pTmpPrv->UID = IPTVosGetUniqueNumber();
        pTmpPrv->SID = serviceID;
        pTmpPrv->uType = ReportCallback;
        pTmpPrv->u.ciRep.pCallback = pCallback;
        pTmpPrv->u.ciRep.pCallbackRef = pCallbackRef;

        if(dListAdd(&locRequestList, (DNODE*)pTmpPrv) != VRS_OK)
        {
        }
        else
        {
            result = pTmpPrv->UID;
        }
        IPTVosPutSem(&locSemaphore);
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Retrieves a pointer to the request information from the locRequestList. 
*
* RETURNS:
*   Pointer to the request information if successful, otherwise NULL.
*/
static VRSCLIPRV_REQUEST* getRequestInfo(
    UINT32 refValue)    /* Reference value */
{
    VRSCLIPRV_REQUEST* pTmpPrv = 0;

    if(refValue == 0)
    {
    }
    else if (IPTVosGetSem(&locSemaphore, IPT_WAIT_FOREVER) != IPT_OK)
    {
    }
    else if(dListValidate(&locRequestList) != VRS_OK)
    {
        IPTVosPutSem(&locSemaphore);
    }
    else
    {
        pTmpPrv = (VRSCLIPRV_REQUEST*)dListFirst(&locRequestList);

        while(pTmpPrv != 0)
        {
            if(pTmpPrv->UID == refValue)
            {
                break; /* found information */
            }
            pTmpPrv = (VRSCLIPRV_REQUEST*)dListNext((DNODE*)pTmpPrv);
        }
        if(pTmpPrv != 0)
        {
        }
        else
        {
        }
        IPTVosPutSem(&locSemaphore);
    }
    return pTmpPrv;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Frees memory used for request information.
*   Note that no semaphore protection is used by this function.
*
* RETURNS:
*   Nothing.
*/
static void freeRequestInfo(VRSCLIPRV_REQUEST* pPrv)
{
    if(pPrv != 0)
    {
        switch(pPrv->uType)
        {
            case General:
            case ReportCallback:
                break;
            case RequestCallback:
                if(pPrv->u.ciReq.pFilterUri != 0)
                {
                    IPTVosFree((BYTE*)(pPrv->u.ciReq.pFilterUri));
                }
                if(pPrv->u.ciReq.pDestURI)
                {
                    IPTVosFree((BYTE*)(pPrv->u.ciReq.pDestURI));
                }
                break;
            case Undefined:
                break;    
            default:
                break;
        }
        if(dListDelete(&locRequestList, (DNODE*)pPrv) != VRS_OK)
        {
        }
        else
        {
        }
        IPTVosFree((BYTE*)pPrv);
    }
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Removes request information from the locRequestList.
*
* RETURNS:
*   Nothing.
*/
static void remRequestInfo(
    UINT32 refValue)    /* Reference value */
{
    if(refValue == 0)
    {
    }
    else if(IPTVosGetSem(&locSemaphore, IPT_WAIT_FOREVER) != IPT_OK)
    {
    }
    else if(dListValidate(&locRequestList) != VRS_OK)
    {
        IPTVosPutSem(&locSemaphore);
    }
    else
    {
        VRSCLIPRV_REQUEST* pTmpPrv = (VRSCLIPRV_REQUEST*)dListFirst(&locRequestList);

        while(pTmpPrv != 0)
        {
            if(pTmpPrv->UID == refValue)
            {
                break; /* found information */
            }
            pTmpPrv = (VRSCLIPRV_REQUEST*)dListNext((DNODE*)pTmpPrv);
        }
        if(pTmpPrv != 0)
        {
            /* remove information */
            freeRequestInfo(pTmpPrv);
        }
        else
        {
            /* no informaiton found */
        }
        IPTVosPutSem(&locSemaphore);
    }
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Clears all the request information from the locRequestList list.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT removeAllRequestInfo(void)
{
    VRS_RESULT result;

    if (IPTVosGetSem(&locSemaphore, IPT_WAIT_FOREVER) != IPT_OK)
    {
        return VRS_SEMAPHORE_ERROR;
    }
    else if((result = dListValidate(&locRequestList)) != VRS_OK)
    {
        IPTVosPutSem(&locSemaphore);
        return result;
    }
    else
    {
        VRSCLIPRV_REQUEST* pTmpPrv = (VRSCLIPRV_REQUEST*)dListFirst(&locRequestList);

        while(pTmpPrv != 0)
        {
            freeRequestInfo(pTmpPrv);
            pTmpPrv = (VRSCLIPRV_REQUEST*)dListFirst(&locRequestList);
        }
        /* verify that list is empty and still valid */
        if(dListCount(&locRequestList) != 0)
        {
            result = VRS_MEMORY_INVALID;
        }
        else
        {
            result = dListValidate(&locRequestList);
        }
    }
    IPTVosPutSem(&locSemaphore);
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Sends a write data request to the server.
* \latex
* \begin{verbatim}    
*    Message format:
*       -----------------------------------------
*      | header | uriLen | uri | dataLen | data |
*       -----------------------------------------
* bytes    8       2       x       2       x   
* \end{verbatim}
* \endlatex    
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT sendWriteVersionDataRequest(
    const char*         pUri,         /* pointer to null terminated uri string */
    UINT16              uriLen,       /* length of the uri including null */
    const char*         pData,        /* pointer to null terminated data string */
    UINT16              dataLen,      /* length of the data including null */
    VRS_REPORT_CALLBACK pCallback,    /* Pointer to callback function */
    void*               pCallbackRef) /* Pointer to callback reference */
{
    char *pSendBuf=NULL, *pSendBufCurrPos=NULL;
    UINT32 sendBufLen = 0;
    INT32 sendResult = 0;
    VRS_RESULT result;
    UINT32 refVal;
    void* pTmpCallbackRef;

    /* 
    * validate arguments
    * checking status of client and server and validate thr data input
    * is already done in calling routine VRSClient_report. Only callback
    * information needs to be validated. 
    */
    if(pCallback == NULL && pCallbackRef == NULL)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }

    /* calculate the needed buffer size for the complete msg */
    sendBufLen = VRS_HEADER_TOTAL_SIZE + sizeof(UINT16) + uriLen + sizeof(UINT16) + dataLen;
    if(sendBufLen > VRS_MAX_URI_N_DATA_SIZE)
    {
        return VRS_DATA_BAD_SIZE;
    }

    /* check if it is a standard or a generic reporter that is trying to report data */
    if(pCallback != NULL)
    {
        /* standard reporter */
        pTmpCallbackRef = pCallbackRef;
    }
    else
    {
        /* generic reporter, check if the generic reporter is registered */
        if((*((UINT32*)pCallbackRef)) == 0)
        {
            /* unregistered */ 
            if(locNrRegGenReporters < VRS_MAX_GEN_REPORTERS)
            {
                pTmpCallbackRef = (void*)++locNrRegGenReporters;
                locGenReporterStatus[locNrRegGenReporters-1] = VRS_COMMUNICATION_INVALID;
                (*((UINT32*)pCallbackRef)) = (UINT32)pTmpCallbackRef;
            }
            else
            {
                return VRS_MEMORY_ALLOCATION_ERROR;
            }
        }
        else
        {
            /* registered */
            if((*((UINT32*)pCallbackRef)) <= locNrRegGenReporters)
            {
                /* check if a report is already in process */
                if(locGenReporterStatus[(*(UINT32*)pCallbackRef)-1] == VRS_COMMUNICATION_INVALID)
                {
                    /* report already in process */
                    return VRS_COMMUNICATION_INVALID;
                }
                else
                {
                    locGenReporterStatus[(*(UINT32*)pCallbackRef)-1] = VRS_COMMUNICATION_INVALID;
                    pTmpCallbackRef = (void*)(*(UINT32*)pCallbackRef);
                }
  
            }
            else
            {
                return VRS_DATA_NOT_FOUND;
            }
        }
    }

    /* allocate memory for the send buffer */
    pSendBuf = (char*)IPTVosMalloc(sendBufLen);
    if(pSendBuf == NULL)
    {
        return VRS_MEMORY_ALLOCATION_ERROR;
    }

    /* fill header */
    if((result = VRS_fillHeader(pSendBuf, sendBufLen, VRS_SID_WRITE_VERSION_DATA_REQUEST, 
                                (UINT16)(sizeof(UINT16) + uriLen + sizeof(UINT16) + dataLen))) != VRS_OK)
    {
        IPTVosFree((BYTE*)pSendBuf);
        return result;
    }
    else if((refVal = addReportInfoCallback(VRS_SID_WRITE_VERSION_DATA_REQUEST, pCallback, pTmpCallbackRef)) == 0)
    {
        IPTVosFree((BYTE*)pSendBuf);
        return VRS_MESSAGE_ERROR;
    }
    else
    {
        pSendBufCurrPos = pSendBuf + VRS_HEADER_TOTAL_SIZE;

        /* Copy URI Length and URI to send buffer */
        SetUINT16(pSendBufCurrPos, 0, uriLen);
        memcpy(pSendBufCurrPos + sizeof(UINT16), pUri, uriLen);
        pSendBufCurrPos = pSendBufCurrPos + sizeof(UINT16) + uriLen;

        /* Copy Data Length and Data to send buffer */
        SetUINT16(pSendBufCurrPos, 0, dataLen);
        memcpy(pSendBufCurrPos + 2, pData, dataLen);

        /* Send message to server comid */
        sendResult = MDComAPI_putRequestMsgQ(
            VRS_SERVER_COMID,           /* ComID */
            pSendBuf,                   /* Data set */
            sendBufLen,                 /* Size of data */
            1,                          /* Number of expected replies */
            0,                          /* Time-out value in milliseconds. 0 = default value */
            locMdQueue,                 /* Queue for communication result */
            (void*)refVal,              /* Caller reference value (optional) */
            0,                          /* Topo counter */
            locPtrDestUri,              /* Overriding destination URI */
            locClientConfig.sourceUri); /* Overriding of source URI */
    }
    /* De-allocate the allocated send buffer */
    IPTVosFree((BYTE*)pSendBuf);

    if(sendResult != IPT_OK)
    {
        remRequestInfo(refVal);
        return VRS_COMMUNICATION_ERROR;
    }
    else
    {
    }
    return VRS_OK;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Sends a server ready time request to the server.
* \latex
* \begin{verbatim}    
*    Message format:
*       ------------------------------
*      |version| servId | buffLen (0) |
*       ------------------------------
* bytes    4       2         2     
* \end{verbatim}
* \endlatex    
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT sendServerReadyTimeRequest(void)
{
    char buffer[VRS_HEADER_TOTAL_SIZE];
    UINT32 refVal;
    INT32 result;

    if((result = VRS_fillHeader(buffer, sizeof(buffer), VRS_SID_SERVER_READY_TIME_REQUEST, 0)) != VRS_OK)
    {
        return result;
    }
    else if((refVal = addRequestInfo(VRS_SID_SERVER_READY_TIME_REQUEST)) == 0)
    {
        return VRS_MESSAGE_ERROR;
    }
    /* Send message to server comid */
    result = MDComAPI_putRequestMsgQ(
        VRS_SERVER_COMID,           /* ComID */
        buffer,                     /* Data set */
        sizeof(buffer),             /* Size of data */
        1,                          /* Number of expected replies */
        0,                          /* Time-out value in milliseconds. 0 = default value*/
        locMdQueue,                 /* Queue for communication result*/
        (void*)refVal,              /* Caller reference value (optional)*/
        0,                          /* Topo counter*/
        locPtrDestUri,              /* Overriding destination URI */
        locClientConfig.sourceUri); /* Overriding of source URI */
    if(result != IPT_OK)
    {
        remRequestInfo(refVal);
        result = VRS_COMMUNICATION_ERROR;
    }
    else
    {
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Processes incoming write version data reply messages.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT processWriteVersionDataReply(
    const MSG_INFO* pMsgInfo,   /* message information */
    char* pData,                /* pointer to data */
    UINT32 dataLength)          /* length of data */
{
    VRS_RESULT result;
    VRSCLIPRV_REQUEST* pReqInfo = 0;
    UINT16 dataSize;

    if((result = VRS_validateHeader(pData, dataLength, 0, &dataSize)) != VRS_OK)
    {
    }
    else if(dataSize != sizeof(UINT32))
    {
    }
    else if((pReqInfo = getRequestInfo((UINT32)pMsgInfo->pCallerRef)) == 0)
    {
        result = VRS_DATA_NOT_FOUND;
    }
    else
    {
        /* get the result */
        result = (VRS_RESULT)VRS_memGet32((UINT8*)pData + VRS_HEADER_TOTAL_SIZE);

        if(pReqInfo->u.ciRep.pCallback != 0)
        {
            /* make callback */
            pReqInfo->u.ciRep.pCallback(result, pReqInfo->u.ciRep.pCallbackRef);
        }
        else if(pReqInfo->u.ciRep.pCallbackRef != 0)
        {
            /* Uppdate the status for the generic reporter */
            UINT32 idx;
            idx = (UINT32)pReqInfo->u.ciRep.pCallbackRef;
            if(idx == 0 || idx > locNrRegGenReporters || idx > VRS_MAX_GEN_REPORTERS)
            {
                result = VRS_DATA_NOT_FOUND;
            }
            else
            {
                locGenReporterStatus[idx-1] = result;
            }      
        }
        else
        {
            result = VRS_MEMORY_POINTER_ERROR;
        }
        remRequestInfo((UINT32)pMsgInfo->pCallerRef);
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Processes incoming server ready time reply messages.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT processServerReadyTimeReply(
    const MSG_INFO* pMsgInfo,   /* message information */
    char* pData,                /* pointer to data */
    UINT32 dataLength)          /* length of data */
{
    VRS_RESULT result;
    UINT16 dataSize;
    UINT8 protVer;
    UINT8 protRel;
    UINT8 protUpd;
    UINT8 protEvo;


    if((result = VRS_validateHeader(pData, dataLength, 0, &dataSize)) != VRS_OK)
    {
    }
    else if(dataSize != sizeof(UINT32))
    {
    }
    else
    {
        if(pMsgInfo->msgType == MD_MSGTYPE_RESPONSE)
        {
            if(getRequestInfo((UINT32)pMsgInfo->pCallerRef) == 0)
            {
                result = VRS_DATA_NOT_FOUND;
            }
            else
            {
                remRequestInfo((UINT32)pMsgInfo->pCallerRef);
            }
        }
        else if(locPtrDestUri != 0)
        {
            if(VRS_strCmp(pMsgInfo->srcURI, locPtrDestUri, strlen(locPtrDestUri)) != 0)
            {
                return result;
            }
        }
        /* update the servers protocol version */
        if (result != VRS_DATA_NOT_FOUND)
        {
            GetUINT8(&protVer, pData, VRS_HEADER_VERSION_OFFSET);
            GetUINT8(&protRel, pData, VRS_HEADER_RELEASE_OFFSET);
            GetUINT8(&protUpd, pData, VRS_HEADER_UPDATE_OFFSET);
            GetUINT8(&protEvo, pData, VRS_HEADER_EVOLUTION_OFFSET);
            locServerProtocolVersion = (protVer<<24) + (protRel<<16) + (protUpd<<8) + protEvo;
        }
        /* update the URI to the server */
        if(locPtrDestUri == 0 && result != VRS_DATA_NOT_FOUND)
        {
            strncpy(locClientConfig.destConfig[0].source, "*", VRS_MAX_FULL_LABEL_LEN);
            strncpy(locClientConfig.destConfig[0].dest, pMsgInfo->srcURI, VRS_MAX_FULL_LABEL_LEN);
            locPtrDestUri = locClientConfig.destConfig[0].dest;
        }
        /* get the server ready time */
        GetUINT32(&locServerReadyTime, pData, VRS_HEADER_TOTAL_SIZE);
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Processes incoming error reply messages.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT processCommunicationErrors(
    const MSG_INFO* pMsgInfo)   /* message information */
{
    static int srvRdyTmAttCntr = 0;
    VRSCLIPRV_REQUEST* pReqInfo = 0;
    VRS_RESULT result = VRS_COMMUNICATION_ERROR;

    if((pReqInfo = getRequestInfo((UINT32)pMsgInfo->pCallerRef)) == 0)
    {
        /* an old error result may be received when client is terminated and started again */
    }
    else if(pReqInfo->SID == VRS_SID_SERVER_READY_TIME_REQUEST && locServerReadyTime != 0)
    {
        /* a server multicast has been received before the client reply */
        remRequestInfo((UINT32)pMsgInfo->pCallerRef);
        result = VRS_OK;
    }
    else if(pReqInfo->SID == VRS_SID_SERVER_READY_TIME_REQUEST && locServerReadyTime == 0 && ++srvRdyTmAttCntr < 2)
    {
        /* handle iptcom sequence number zero, incase this device is restarted and the server is not */
        remRequestInfo((UINT32)pMsgInfo->pCallerRef);
        result = sendServerReadyTimeRequest();
    }
    else
    {
        VRS_SERVICE_STATUS tmpServiceStatus = locServiceStatus;

        /* check severity of the error */
        switch (pMsgInfo->resultCode)
        {
            /*   MD_SEND_OK          :    0 = Correct acknowledge received */
            case MD_RECEIVE_OK       : /* 0 = Expected number received */
                result = VRS_MESSAGE_BAD_HEADER;
                break;
            case MD_NO_ACK_RECEIVED  : /* -1 = No Acknowledge received */
            case MD_RESP_NOT_RECEIVED: /* -2 = Responses not received */
            case MD_RESP_MISSING     : /* -3 = All responses not received */
            case MD_SEND_FAILED      : /* -4 = Sending failed */
            case MD_NO_LISTENER      : /* -5 = No listener */
            case MD_NO_BUF_AVAILABLE : /* -6 = No buffer/memory available */
                result = VRS_COMMUNICATION_TIMEOUT;
                break;
            case MD_WRONG_DATA       : /* -7 = Wrong data, e.g. the receiver couldn't interpret the data */
                tmpServiceStatus = VRS_SERVICE_ERROR;
                result = VRS_COMMUNICATION_ERROR;
                break;
            default:
                result = VRS_COMMUNICATION_ERROR;
                break;
        }

        /* check if a callback shall be made */
        if(pReqInfo->uType == RequestCallback && pReqInfo->u.ciReq.pCallback != 0)
        {
            pReqInfo->u.ciReq.pCallback(pReqInfo->u.ciReq.requestType, result, 0,
                                    NULL, NULL, pReqInfo->u.ciReq.pCallbackRef);
        }
        else if(pReqInfo->uType == ReportCallback)
        {
            if( pReqInfo->u.ciRep.pCallback != 0)
            {
                /* Uppdate the status for standard reporter */
                pReqInfo->u.ciRep.pCallback(result, pReqInfo->u.ciRep.pCallbackRef);
            }
            else if(pReqInfo->u.ciRep.pCallbackRef != 0)
            {
                /* Uppdate the status for generic reporter */
                UINT32 idx;
                idx = (UINT32)pReqInfo->u.ciRep.pCallbackRef;
                if(idx > 0 && idx <= locNrRegGenReporters && idx <= VRS_MAX_GEN_REPORTERS)
                {
                    locGenReporterStatus[idx-1] = result;
                } 
            }
        }
        else if(pReqInfo->uType == General)
        {
            /* server ready time request faild, set server ready time to 0 */
            locServerReadyTime = 0;
        }

        /* check if service status shall be changed and cleanup */
        if(tmpServiceStatus != locServiceStatus && locServiceStatus == VRS_SERVICE_RUNNING)
        {
            locServiceStatus = tmpServiceStatus;
        }
        remRequestInfo((UINT32)pMsgInfo->pCallerRef);
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Processes incoming message data (MD) packets. MD is handled differently 
*   depending on the Service ID.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
static VRS_RESULT process(
    const MSG_INFO* pMsgInfo,   /* message information */
    char* pData,                /* pointer to data */
    UINT32 dataLength)          /* length of data */
{
    VRS_RESULT result = VRS_OK;

    if(pMsgInfo == NULL)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    /*
     * pData and dataLength is zero for timeouts
     * they are verified before usage in VRS_validateHeader
     */
    if(pMsgInfo->msgType == MD_MSGTYPE_RESULT && pMsgInfo->resultCode != 0)
    {
        result = processCommunicationErrors(pMsgInfo);
    }
    else if(pMsgInfo->msgType == MD_MSGTYPE_RESPONSE && pMsgInfo->comId == VRS_SERVER_COMID)
    {
        UINT16 serviceId;

        if(VRS_validateHeader(pData, dataLength, &serviceId, 0) == VRS_OK)
        {
            switch(serviceId)
            {
                case VRS_SID_WRITE_VERSION_DATA_REPLY:
                    result = processWriteVersionDataReply(pMsgInfo, pData, dataLength);
                    break;
                case VRS_SID_SERVER_READY_TIME_REPLY:
                    result = processServerReadyTimeReply(pMsgInfo, pData, dataLength);
                    break;
                case VRS_SID_HEADER_ERROR:
                    result = processCommunicationErrors(pMsgInfo);
                    result = VRS_MESSAGE_BAD_HEADER;
                    break;
                default:
                    result = VRS_MESSAGE_BAD_HEADER;
                    break;
            }
        }
        else
        {
            result = VRS_MESSAGE_BAD_HEADER;
        }
    }
    else if(pMsgInfo->msgType == MD_MSGTYPE_DATA && pMsgInfo->comId == VRS_CLIENT_COMID)
    {
        UINT16 serviceId;

        if(VRS_validateHeader(pData, dataLength, &serviceId, 0) == VRS_OK)
        {
            switch(serviceId)
            {
                case VRS_SID_SERVER_READY_TIME:
                    result = processServerReadyTimeReply(pMsgInfo, pData, dataLength);
                    break;
                default:
                    result = VRS_MESSAGE_BAD_HEADER;
                    break;
            }
        }
        else
        {
            result = VRS_MESSAGE_BAD_HEADER;
        }
    }
    else
    {
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliInternalGroup
*
* DESCRIPTION:
*   Waits for that the TDC has started and then it adds a message data URI 
*   listener. After that it waits for incoming messages from the VRS server.
*
* RETURNS:
*   Nothing.
*/
static void receiveData(void)
{
    int res = 0;
    UINT32 size = 0;
    char *pRecBuf=NULL;
    MSG_INFO msgInfo;
    UINT32 startSec = IPVosGetSecTimer();
    VRS_RESULT result;
    static int errorReported = 0;
    
    pRecBuf = NULL; /* Use buffer allocated by IPTCom */

    if(locServiceStatus == VRS_SERVICE_RUNNING)
    {
        return;
    }
    /* wait for communication layer */
    while(VRS_getCommunicationState() != VRS_OK && locServiceStatus == VRS_SERVICE_STARTING)
    {
        if(startSec+360 < IPVosGetSecTimer())
        {
            locServiceStatus = VRS_SERVICE_ERROR;
        }
        IPTVosTaskDelay(100);
    }
    if(locServiceStatus == VRS_SERVICE_STARTING)
    {
        /* TDC has started */
        if((result = MDComAPI_addUriListenerQ(locMdQueue, 0, VRS_CLIENT_LISTEN)) != IPT_OK)
        {
            if(MDComAPI_destroyQueue(locMdQueue) != IPT_OK)
            {
            }
            locMdQueue = 0;
            locServiceStatus = VRS_SERVICE_ERROR;
        }
        /* convert the lTags in our the source URI */
        else if(VRS_convertLTagsToOwnIds(locClientConfig.sourceUri) != VRS_OK)
        {
            locServiceStatus = VRS_SERVICE_ERROR;
        }
        else
        {
            locServiceStatus = VRS_SERVICE_RUNNING;
            setDestinationUri();
            /* send server ready time request */
            if(sendServerReadyTimeRequest() != VRS_OK)
            {
                locServiceStatus = VRS_SERVICE_ERROR;
            }
        }
    }
    while(locServiceStatus == VRS_SERVICE_RUNNING)
    {
        res = MDComAPI_getMsg(
            locMdQueue,   /* Queue ID */
            &msgInfo,     /* Message Info */  
            &pRecBuf,     /* Pointer to pointer to data buffer */
            &size,        /* Number of received bytes */
            IPT_WAIT_FOREVER); /* Wait for result */
        if(res == MD_QUEUE_NOT_EMPTY)
        {
            /* Handle received data */
            res = process(&msgInfo, pRecBuf, size);  
            setErrorCode(res);
            /* De-allocate buffer allocated by IPTCom */
            if(pRecBuf != NULL)
            {
                if((res = MDComAPI_freeBuf(pRecBuf)) != IPT_OK)
                {
                    locServiceStatus = VRS_SERVICE_ERROR;
                    break;
                }
                pRecBuf = NULL;
            }
        }
        else
        {
            if (errorReported < 3)
            {
                if (errorReported <3)
                {
                    errorReported += 1;
                }
                IPTVosTaskDelay(500);
            }
        }
    }
    /* remove request info */
    if(removeAllRequestInfo() != VRS_OK)
    {
        locServiceStatus = VRS_SERVICE_ERROR;
    }
    /* inform reporters about shut down */
    locServerReadyTime = 0;

    /* the md thread is exiting check why */
    switch(locServiceStatus)
    {
        case VRS_SERVICE_STOPPING:
            locServiceStatus = VRS_SERVICE_STOPPED;
            break;
        case VRS_SERVICE_STOPPED:
        case VRS_SERVICE_STARTING:
        case VRS_SERVICE_RUNNING:
        case VRS_SERVICE_ERROR:
        default:
            setErrorCode(VRS_THREAD_ERROR);
            break;
    }
}


/*******************************************************************************
* GLOBAL FUNCTIONS
*/


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Initialization method for the client. Sets the VRS specific IPT-Com 
*   configuration for communication with server and client (ComId 210 and 211) 
*   and creates message data queue. The pRootPath parameter shall specify which 
*   directory the VRS configuration file can be read from and where log files 
*   are allowed to be created. 
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSClient_initiate(
    const char* pRootPath)  /* path to where the component shall r/w files */
{
    static UINT8 firstTimeExecuted = 0;
    VOS_THREAD_ID receiveDataThread = 0;
    VRS_RESULT result;
    IPT_CONFIG_EXCHG_PAR exchg;

    if(firstTimeExecuted == 0)
    {
        ++firstTimeExecuted;
        /* create semaphores */
        if (IPTVosCreateSem(&locSemaphore, IPT_SEM_FULL) != IPT_OK)
        {
            result = VRS_SEMAPHORE_NOT_CREATED;
            locServiceStatus = VRS_SERVICE_ERROR;
            setErrorCode(result);
            return result;
        }
    }
    /* check versions and if called more then once */
    if((result = VRS_checkVersion(VRS_SDK_VERSION_FULL)) != VRS_OK)
    {
        setErrorCode(result);
        return result;
    }
    else if(locServiceStatus == VRS_SERVICE_STOPPED)
    {
        locServiceStatus = VRS_SERVICE_STARTING;
    }
    else
    {
        /* do not set non fatal fault */
        return VRS_COMP_ALREADY_STARTED;
    }

    /* read and set configurable options */
    if((result = setConfiguration(pRootPath)) != VRS_OK)
    {
        setErrorCode(result);
        return result;
    }
    
    /* 
    Set default configuration for communication with server and client, 
    Note: if there is already a configuration in IPTCom configuration database for 
    the comid it can not be over written 
    */

    /* set common parameters */
    exchg.datasetId                  = 0;
    exchg.comParId                   = 2;          /* Always use default com parameters for MD */        
    exchg.mdRecPar.pSourceURI        = "";
    exchg.pdRecPar.pSourceURI        = NULL;
    exchg.pdRecPar.validityBehaviour = IPT_INVALID_KEEP;
    exchg.pdRecPar.timeoutValue      = 0;
    exchg.pdSendPar.pDestinationURI  = NULL;
    exchg.pdSendPar.cycleTime        = 0;
    exchg.pdSendPar.redundant        = 0;
    /* add server msg */
    exchg.comId                      = VRS_SERVER_COMID;
    exchg.mdSendPar.pDestinationURI  = VRS_SERVER_DEST;
    if(iptConfigAddExchgPar(&exchg) == IPT_OK)
    {
    }
    /* add client msg */
    exchg.comId                      = VRS_CLIENT_COMID;
    exchg.mdSendPar.pDestinationURI  = VRS_CLIENT_DEST;
    if(iptConfigAddExchgPar(&exchg) == IPT_OK)
    {
    }

    /* create md queues, listeners and launch threads */
    if((locMdQueue = MDComAPI_createQueue(VRS_CLIENT_MSG_QUEUE_LEN)) == 0)
    {
        locServiceStatus = VRS_SERVICE_ERROR;
        result = VRS_COMMUNICATION_ERROR;
    }
    else if((receiveDataThread = IPTVosThreadSpawn(VRS_CLIENT_NAME, 
                            locClientConfig.threadPolicy, 
                            locClientConfig.threadPriority, 
                            locClientConfig.threadStackSize, 
                            (IPT_THREAD_ROUTINE)receiveData, 
                            NULL)) == 0)
    {
        locServiceStatus = VRS_SERVICE_ERROR;
        result = VRS_THREAD_NOT_CREATED;
    }
    else
    {
    }
    setErrorCode(result);
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Reports the VRS data to the VRS server. The URI argument shall specify the 
*   element name and element instance. The own ID part of the URI string is 
*   optional. For any missing IDs the own IDs will be appended. If car numbering
*   is used instead of car id it will be replaced with the corresponding car ID.
*   If local tags are used in the URI it will be replaced with the own ID.
* \\\\
* \latex
*   Examples:
* \begin{itemize}
*  \item
*    If the URI string is \textit{"software.i1"} all the own IDs would be 
*    appended resulting in the string\\
*    \textit{"software.i1@ownDevId.ownCarId.ownCstId"}.
*  \item
*    If the URI string is \textit{"hardware.i1@mio"} the car and consist
*    IDs would be appended resulting in the string\\
*    \textit{"hardware.i1@mio.ownCarId.ownCstId"}.
*  \item
*    If the URI string is \textit{"hardware.i1@mio.car12345"} the consist
*    ID would be appended resulting in the string\\
*    \textit{"hardware.i1@mio.car12345.ownCstId"}.
*  \item
*    If the URI string is \textit{"hardware.i1@mio.car01.lCst"} the
*    consist ID for 'car01' (ex car23456) would be appended resulting in the string
*    \textit{"hardware.i1@mio.car23456.ownCstId"}.
* \end{itemize}
* \endlatex
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT  VRSClient_report(
    const char*         pUri,          /* Null terminated data uri string */
    const char*         pData,         /* Null terminated version data string */
    VRS_REPORT_CALLBACK pCallback,     /* callback function */
    void*               pCallbackRef)  /* callback reference */
{
    UINT16 uriLen = 0, dataLen = 0;
    VRS_FULL_LABEL pTmpUri;
    VRS_RESULT result;


    /* check status of client and server */
    if(locServiceStatus != VRS_SERVICE_RUNNING)
    {
        return VRS_COMP_CLI_NOT_STARTED;
    }
    else if(locServerReadyTime == 0)
    {
        return VRS_COMP_SRV_NOT_STARTED;
    }
    else if(pUri == NULL)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    /* create a null terminated temporary full uri copy */
    strncpy(pTmpUri, pUri, VRS_MAX_FULL_LABEL_LEN);
    pTmpUri[VRS_MAX_FULL_LABEL_LEN-1] = '\0';
    /* if train tag is present, remove it */
    truncateUri(pTmpUri);
    /* convert labels and tags to Ids */
    if((result = VRS_convertLabelsToIds(pTmpUri)) != VRS_OK)
    {
        return result;
    }
    else if((result = VRS_convertLTagsToOwnIds(pTmpUri)) != VRS_OK)
    {
        return result;
    }
    /* append own ids */
    else if((result = VRS_appendOwnIds2Uri(pTmpUri)) != VRS_OK)
    {
        return result;
    }
    /* validate input */
    else if((result = VRS_validateUri(pTmpUri, 0)) != VRS_OK)
    {
        return result;
    }
    else if((result = VRS_validateData(pData, 0)) != VRS_OK)
    {
        return result;
    }

    /* calculate uri and data length */
    uriLen = (UINT16)(strlen(pTmpUri) + 1);
    dataLen = (UINT16)(strlen(pData) + 1);

    return sendWriteVersionDataRequest(pTmpUri, uriLen, pData, 
                                       dataLen, pCallback, pCallbackRef);
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Gets the server startup time in seconds after 1970 (time when the server became 
*   ready to receive incoming messages).
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSClient_getReadyTime(
    UINT32* pReadyTime)    /* time server became ready (seconds since 1970) */
{
    VRS_RESULT result = VRS_EXCEPTION;

    if(pReadyTime == NULL) {
        result = VRS_MEMORY_POINTER_ERROR;
    }
    else
    {
        *pReadyTime = 0;
        switch(locServiceStatus)
        {
            case VRS_SERVICE_RUNNING:
                *pReadyTime = locServerReadyTime;
                result = VRS_OK;
                break;
            case VRS_SERVICE_STARTING:
            case VRS_SERVICE_STOPPING:
            case VRS_SERVICE_STOPPED:
                result = VRS_COMP_CLI_NOT_STARTED;
                break;
            case VRS_SERVICE_ERROR:
            default:
                result = VRS_SERVICE_ERROR;
                break;
        }
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   The function gets the last (communication) error that has occurred.
*   A successful function call or a call to this function does not reset the
*   error code. The error code will be reset to zero if the cause disappears.
*   
*   Note: This function shall not be used to check for errors in other
*   function calls.
*
* RETURNS:
*   Last error code.
*/
VRS_RESULT VRSClient_getErrorCode(void)
{
    return locErrorCode;
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Gets descriptive string for the error code.
*
* RETURNS:
*   Pointer to an error string.
*/
const char* VRSClient_getErrorString(
    VRS_RESULT errorCode)   /* error code to get description for */
{
    return VRS_getErrorString(errorCode);
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Gets the SW version as a 32-bit unsigned integer of the format:
*   0xVVRRUUEE (version, release, update, evolution).
*
* RETURNS:
*   Own SW version.
*/
UINT32 VRSClient_getVersion(void)
{
    return VRS_SDK_VERSION_FULL;
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Checks if the used header version is compatible with the used component.
*   Uses the full version define VRS_SDK_VERSION_FULL as parameter. For more 
*   information about VRS_SDK_VERSION_FULL, see chapter VRS Common Functionality. 
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSClient_checkVersion(
    UINT32 vrsVRUE) /* full VRS version to check (VRS_SDK_VERSION_FULL) */
{
    return VRS_checkVersion(vrsVRUE);
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Gets the service status of the client (the state the client is in). For more
*   information about VRS_SERVICE_STATUS, see chapter VRS Common Functionality. 
*
* RETURNS:
*   Own service status.
*/
VRS_SERVICE_STATUS VRSClient_getStatus(void)
{
    return locServiceStatus;
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Gets the intended server's communication protocol version as a 32-bit unsigned integer
*   in the format:
*   0xVVRRUUEE (version, release, update, evolution).
*
* RETURNS:
*   Intended server's communication protocol version.
*/
UINT32 VRSClient_getServerProtocolVersion(void)
{
    return locServerProtocolVersion;
}


/*******************************************************************************
* DOCGROUP: vrsCliApiGroup
*
* DESCRIPTION:
*   Terminate method for the client. Cleans up after the client (created 
*   semaphores, MD queue, MD listener and stops created thread). The 
*   remainingTime parameter shall specify how much time remains before the 
*   device powers down. 
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSClient_terminate(
    UINT32 remainingTime)   /* maximum remaining time in ms before shutdown */
{
    UINT32 startTime = IPTVosGetMilliSecTimer();
    VRS_RESULT result = VRS_OK;
    int iptComRes;

    
    /* kill receiveData thread */
    if(locServiceStatus == VRS_SERVICE_RUNNING)
    {
        locServiceStatus = VRS_SERVICE_STOPPING;
    }
    else
    {
        result = VRS_COMP_CLI_NOT_STARTED;
    }

    /* destroy semaphores, md queues, listeners and stop threads */
    if(locMdQueue != 0)
    {
        MD_QUEUE tmpMdQueue = locMdQueue;

        locMdQueue = 0; /* stop other threads from using the queue */
        MDComAPI_removeListenerQ(tmpMdQueue);
        while((iptComRes = MDComAPI_destroyQueue(tmpMdQueue)) == (int)IPT_QUEUE_IN_USE &&
              remainingTime > (IPTVosGetMilliSecTimer() - startTime))
        {
            IPTVosTaskDelay(1);
        }
        if(iptComRes != IPT_OK)
        {
            result = VRS_COMMUNICATION_TIMEOUT;
        }
        /* yield to give message thread execution time */
        if(remainingTime > (IPTVosGetMilliSecTimer() - startTime))
        {
            IPTVosTaskDelay(1);
        }
        /* wait for thread to die */
        while(VRSClient_getStatus() != VRS_SERVICE_STOPPED && 
              remainingTime > (IPTVosGetMilliSecTimer() - startTime))
        {
            IPTVosTaskDelay(1);
        }
    }
    return result;
}
