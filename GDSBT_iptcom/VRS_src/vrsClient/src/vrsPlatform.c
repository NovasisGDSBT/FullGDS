/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsPlatform.c
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
#include "iptcom.h"
#include "vrs.h"
#include "vrsClient.h"
#include "vrsReportDvsHw2.h"
#include "vrsReportDvsSw2.h"
#include "vrsCommon.h"


/*******************************************************************************
*  DEFINES
*/


/*******************************************************************************
*  TYPEDEFS
*/


/*******************************************************************************
*  GLOBALS
*/


/*******************************************************************************
*  LOCALS
*/


/*******************************************************************************
*  LOCAL FUNCTIONS
*/


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/

/*******************************************************************************
* DOCGROUP: vrsRepDvsHw2InternalGroup
*
* DESCRIPTION:
*   This is an example of a cyclic task for starting and running the VRS Hardware
*   Reporter version 2 on the platform. The cycle time of this task should not
*   be set to less than 5s. Once HW info has been sent to the VRS Server by this
*   reporter there is no need to report HW info unless the VRS Server is restarted
*   or if the device running the reporter is restarted. So the cycle time will be 
*   the reaction time for the reporter to discover if the VRS server has restarted.
*   
* RETURNS:
*   None
*/
void vrsPlatformReportDvsHw2Task(void)
{
    static UINT8 firstTimeExecuted = 0;
    static VRS_RESULT vrsRes;
    static UINT8 callProcess = 1;

    if(firstTimeExecuted == 0)
    {
        ++firstTimeExecuted;
        vrsRes = VRSReportDvsHw2_initiate();
    }

    if ((vrsRes == VRS_OK) && (callProcess == 1))
    {
        /* call process periodicly */
        vrsRes = VRSReportDvsHw2_process();
        if (vrsRes != VRS_OK)
        {
            vrsRes = VRSReportDvsHw2_terminate(0);
            callProcess = 0;
        }
    }
}

/*******************************************************************************
* DESCRIPTION:
*   This function should read HW version information from the local download
*   and versioning system on the platform. HW version data should be returned in
*   the HW info buffer as XML data according to the schema file unit2_01000000.xsd. 
*
*   The function provides access to hardware version information of all hardware
*   components belonging to the target device.
*
*   The function may be used with static buffer provided by the caller of the
*   routine as well as with dynamic buffer provided by the interface itself. In
*   the latter case, the buffer provided by the function must be freed explicitly
*   by a call of the function vrsPlatformReleaseHwinfoBuffer().
*
* PARAMETERS
*   bufferSize:  If the HW info buffer is provided by the caller of the 
*                function, the bufferSize parameter has to
*                specify the size of this user provided buffer. If the 
*                HW info buffer shall be provided by the function itself,
*                the bufferSize parameter has to be set to zero.
*
*   pActSize:    The parameter pActSize holds the pointer to a user provided variable
*                which will hold the actual number of bytes delivered by the
*                function as hardware identification string. The value of the
*                size variable includes the string termination zero character.
*
*   ppBuffer:    The parameter ppBuffer holds the pointer to a user provided variable
*                which holds the pointer to the retrieved HW info data buffer.
*                If the  function shall be used with static memory
*                buffers (i.e. a buffer provided by the caller of the function), the
*                parameter ppBuffer shall be initialized with the pointer to that buffer
*                before the routine is called. 
*                If the memory for the HW info buffer shall be allocated dynamically
*                by the interface itself, the ppBuffer variable shall be initialized
*                by a NULL pointer before the routine is called. In the latter case,
*                the ppBuffer variable will hold the pointer to the dynamically allocated
*                buffer on successful return from the function.
*
*
* RETURNS:
*   VRS_OK when HW version info successfully written to buffer.
*   VRS_ERROR when failure
*/
VRS_RESULT vrsPlatformGetHwinfo(UINT32 bufferSize, UINT32 *pActSize, char **ppBuffer)
{
    /******************************************************************************
    *   ADD CODE FOR PLATFORM (START)
    ******************************************************************************/
    (void)bufferSize;
    (void)pActSize;
    (void)ppBuffer;
    /*****************************************************************************
    *   ADD CODE FOR PLATFORM (END)
    ******************************************************************************/

    return VRS_OK;
}


/*******************************************************************************
* DESCRIPTION:
*   This function should free the memory allocated for the HW info buffer.
*
* RETURNS:
*   VRS_OK when HW version info buffer successfully released.
*   VRS_ERROR when failure
*/
void vrsPlatformReleaseHwinfoBuffer(char* pBuffer)
{
    /******************************************************************************
    *   ADD CODE FOR PLATFORM (START)
    ******************************************************************************/
    (void)pBuffer;
    /*****************************************************************************
    *   ADD CODE FOR PLATFORM (END)
    ******************************************************************************/
}


/*******************************************************************************
*
* DESCRIPTION:
*   This is an example of a cyclic task for starting and running the VRS Software
*   Reporter version 2 on the platform. The cycle time of this task should not
*   be set to less than 5s. Once SW info has been sent to the VRS Server by this
*   reporter there is no need to report SW info unless the VRS Server is restarted
*   or if the device running the reporter is restarted. So the cycle time will be 
*   the reaction time for the reporter to discover if the VRS server has restarted.
*   
* RETURNS:
*   None
*/
void vrsPlatformReportDvsSw2Task(void)
{
    static UINT8 firstTimeExecuted = 0;
    static VRS_RESULT vrsRes;
    static UINT8 callProcess = 1;

    if(firstTimeExecuted == 0)
    {
        ++firstTimeExecuted;
        vrsRes = VRSReportDvsSw2_initiate();
    }

    if ((vrsRes == VRS_OK) && (callProcess == 1))
    {
        /* call process periodicly */
        vrsRes = VRSReportDvsSw2_process();
        if (vrsRes != VRS_OK)
        {
            vrsRes = VRSReportDvsSw2_terminate(0);
            callProcess = 0;
        }
    }
}


/*******************************************************************************
* DESCRIPTION:
*   This function should read SW version information from the local download
*   and versioning system on the platform. SW version data should be returned in
*   the SW info buffer as XML data according to the schema file system2_01000000.xsd. 
*
*   The function provides access to software version information of all software
*   components belonging to the target device.
*
*   The function may be used with static buffer provided by the caller of the
*   routine as well as with dynamic buffer provided by the interface itself. In
*   the latter case, the buffer provided by the function must be freed explicitly
*   by a call of the function vrsPlatformReleaseHwinfoBuffer().
*
* PARAMETERS
*   bufferSize:  If the SW info buffer is provided by the caller of the 
*                function, the bufferSize parameter has to
*                specify the size of this user provided buffer. If the 
*                SW info buffer shall be provided by the function itself,
*                the bufferSize parameter has to be set to zero.
*
*   pActSize:    The parameter pActSize holds the pointer to a user provided variable
*                which will hold the actual number of bytes delivered by the
*                function as hardware identification string. The value of the
*                size variable includes the string termination zero character.
*
*   ppBuffer:    The parameter ppBuffer holds the pointer to a user provided variable
*                which holds the pointer to the retrieved SW info data buffer.
*                If the  function shall be used with static memory
*                buffers (i.e. a buffer provided by the caller of the function), the
*                parameter ppBuffer shall be initialized with the pointer to that buffer
*                before the routine is called. 
*                If the memory for the SW info buffer shall be allocated dynamically
*                by the interface itself, the ppBuffer variable shall be initialized
*                by a NULL pointer before the routine is called. In the latter case,
*                the ppBuffer variable will hold the pointer to the dynamically allocated
*                buffer on successful return from the function.
*
*
* RETURNS:
*   VRS_OK when SW version info successfully written to buffer.
*   VRS_ERROR when failure
*/
VRS_RESULT vrsPlatformGetSwinfo(UINT32 bufferSize, UINT32 *pActSize, char **ppBuffer)
{
    /******************************************************************************
    *   ADD CODE FOR PLATFORM (START)
    ******************************************************************************/
    (void)bufferSize;
    (void)pActSize;
    (void)ppBuffer;
    /*****************************************************************************
    *   ADD CODE FOR PLATFORM (END)
    ******************************************************************************/

    return VRS_OK;
}


/*******************************************************************************
* DESCRIPTION:
*   This function should free the memory allocated for the SW info buffer.
*
* RETURNS:
*   VRS_OK when SW version info buffer successfully released.
*   VRS_ERROR when failure
*/
void vrsPlatformReleaseSwinfoBuffer(char* pBuffer)
{
    /******************************************************************************
    *   ADD CODE FOR PLATFORM (START)
    ******************************************************************************/
     (void)pBuffer;
    /*****************************************************************************
    *   ADD CODE FOR PLATFORM (END)
    ******************************************************************************/
}

/******************************************************************************/
