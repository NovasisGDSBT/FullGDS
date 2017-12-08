/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsReportDvsSw2.c
*
* DESCRIPTION : 
*
********************************************************************************
* HISTORY  :
* 
*******************************************************************************/


/*******************************************************************************
* DOCUMENTATION INFORMATION */

/* DOCGROUP: vrsRepDvsSw2ApiDescription
*\latex
* This section lists the public C functions. 
*\endlatex
*/


/*******************************************************************************
*  INCLUDES
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "iptcom.h"
#include "vrsClient.h"
#include "vrsReportDvsSw2.h"
#include "vrsPlatform.h"


/*******************************************************************************
* DEFINES 
*/
#define VRS_COMP_NAME   "vrsReportDvsSw2" /* only declare in c file */


/*******************************************************************************
* TYPEDEFS
*/
static const char ownDataUri[] = "software.i2@";

/*******************************************************************************
* GLOBALS 
*/


/*******************************************************************************
* LOCALS
*/
static UINT32             locTmpClientRdyTime = 0;
static UINT32             locLastClientRdyTime = 0;
static VRS_RESULT         locErrorCode = VRS_OK;
static VRS_SERVICE_STATUS locServiceStatus = VRS_SERVICE_STOPPED;
static char*              locDvsDataBuffer = 0;


/*******************************************************************************
* LOCAL FUNCTIONS
*/


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2InternalGroup
*
* DESCRIPTION:
*   Callback function for VRSClient_report(). The Client calls this function
*   with the result of the last report.
*
* RETURNS:
*   Nothing.
*/
static void reportDvsSw2Callback(
    VRS_RESULT  result,   /* result code 0 ok otherwise error */
    void*       pRef)     /* pointer to callback reference */
{
    (void)pRef;
    if (result == VRS_COMMUNICATION_TIMEOUT)
    {
        locLastClientRdyTime = 0;
    }
    else
    {
        /* de-allocate SW request buffer */
        vrsPlatformReleaseSwinfoBuffer(locDvsDataBuffer);
        locDvsDataBuffer = 0;
    }
    locErrorCode = result;
}


/*******************************************************************************
* GLOBAL FUNCTIONS
*/


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2ApiGroup
*
* DESCRIPTION:
*   Initiate method for the Reporter. Adds the component to the monitor, nMon.
*   Also performs a version check of the own version.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSReportDvsSw2_initiate(void) 
{
    static UINT8 firstTimeExecuted = 0;
    VRS_RESULT result;

    if(firstTimeExecuted == 0)
    {
        ++firstTimeExecuted;
    }
    /* check versions and if called more then once */
    if((result = VRSClient_checkVersion(VRS_SDK_VERSION_FULL)) != VRS_OK)
    {
    }
    else if(locServiceStatus == VRS_SERVICE_STOPPED)
    {
        locTmpClientRdyTime = 0;
        locLastClientRdyTime = 0;
        locErrorCode = result;
        locServiceStatus = VRS_SERVICE_RUNNING;
    }
    else
    {
        result = VRS_COMP_ALREADY_STARTED;
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2ApiGroup
*
* DESCRIPTION:
*   Process method for the Reporter. Checks if version data should be reported.
*   If so, collects the version data and reports it to the Server by using the
*   Client API.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSReportDvsSw2_process(void)
{
    VRS_RESULT        vrsRes = 0;
    VRS_RESULT        result = 0;
    UINT32            swActSize = 0;
    char*             pSWInfo = NULL;
    static INT8       reportErr = 0;

    /* check status of reporter */
    if(locServiceStatus != VRS_SERVICE_RUNNING)
    {
        return VRS_COMP_NOT_STARTED;
    }
    /* get client ready timestamp */
    if ((vrsRes = VRSClient_getReadyTime(&locTmpClientRdyTime)) != VRS_OK)
    {
        locErrorCode = vrsRes;
    }
    else if ((locTmpClientRdyTime != locLastClientRdyTime) && (locTmpClientRdyTime != 0))
    {
        if (VRSClient_getServerProtocolVersion() < VRS_DL_SUPPORT)
        {
            if (reportErr == 0)
            {
                reportErr++;
            }
            vrsRes = VRS_MESSAGE_WRONG_VERSION;
            locErrorCode = vrsRes;
            return vrsRes;
        }
        else if (locDvsDataBuffer == NULL)
        {
            /* call DVS for software info */
            result = vrsPlatformGetSwinfo(0, &swActSize, &pSWInfo);
            if (result != VRS_OK || swActSize == 0 || pSWInfo == NULL)
            {
                if (pSWInfo != NULL)
                {
                    vrsPlatformReleaseSwinfoBuffer(pSWInfo);
                    pSWInfo = NULL;
                }
                vrsRes = VRS_DATA_INVALID;
                locErrorCode = vrsRes;
                return vrsRes;
            }
            locDvsDataBuffer = pSWInfo;
        }
        else
        {
            pSWInfo = locDvsDataBuffer;
        }

        /* report data */
        vrsRes = VRSClient_report(ownDataUri, pSWInfo, reportDvsSw2Callback, 0);
        /* update local timestamp, only if VRSClient_report was successful */
        if (vrsRes == VRS_OK)
        {
            locLastClientRdyTime = locTmpClientRdyTime;
        }
        else
        {
        }
        locErrorCode = vrsRes;
    }
    else
    {
        vrsRes = locErrorCode;
    }
    return vrsRes;
}


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2ApiGroup
*
* DESCRIPTION:
*   Terminate method for Reporter. Cleans up after the Reporter. 
*   The remainingTime parameter shall specify how much time remains before the 
*   device powers down.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSReportDvsSw2_terminate(
    UINT32 remainingTime)   /* maximum remaining time in ms before shutdown */
{
    VRS_RESULT result = VRS_OK;

    (void)remainingTime;   /* to remove compiler warning */
    if(locServiceStatus == VRS_SERVICE_RUNNING)
    {
        locServiceStatus = VRS_SERVICE_STOPPED;
        locTmpClientRdyTime = 0;
        locLastClientRdyTime = 0;
    }
    else
    {
        result = VRS_COMP_NOT_STARTED;
    }
    return result;
}


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2ApiGroup
*
* DESCRIPTION:
*   This function returns the last error that has occurred.
*   A successful function call or a call to this function does not reset the
*   error code. The error code will be reset to zero if the cause disappears.
*   
*   Note: This function shall not be used to check for errors in other
*   function calls.
*
* RETURNS:
*   0 if OK, otherwise error or warning.
*/
VRS_RESULT VRSReportDvsSw2_getErrorCode(void)
{
    return locErrorCode;
}


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2ApiGroup
*
* DESCRIPTION:
*   Gets a descriptive string for the error code.
*
* RETURNS:
*   Pointer to error string.
*/
const char* VRSReportDvsSw2_getErrorString(
    VRS_RESULT errorCode)   /* error code to get description for */
{
    return VRSClient_getErrorString(errorCode);
}


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2ApiGroup
*
* DESCRIPTION:
*   Gets the SW version as a 32-bit unsigned integer of the format:
*   0xVVRRUUEE (version, release, update, evolution).
*
* RETURNS:
*   The own SW version.
*/
UINT32 VRSReportDvsSw2_getVersion(void) 
{
    return VRS_SDK_VERSION_FULL;
}


/*******************************************************************************
* DOCGROUP: vrsRepDvsSw2ApiGroup
*
* DESCRIPTION:
*   Gets the service status of the DVS SW2 Reporter (the state the reporter is in).\\
*   For more information about VRS_SERVICE_STATUS, see chapter VRS Common Functionality. 
*
* RETURNS:
*   Own service status.
*/
VRS_SERVICE_STATUS VRSReportDvsSw2_getStatus(void)
{
    return locServiceStatus;
}
