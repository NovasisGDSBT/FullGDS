/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsPlatform.h
*
* DESCRIPTION : This file contains HW specific declarations and needs to be
*               updated by users of the VRS-IP SDK.
*
********************************************************************************
* HISTORY  :
* 
*******************************************************************************/


#ifndef VRSPLATFORM_H
#define VRSPLATFORM_H

/*******************************************************************************
*  INCLUDES
*/


/*******************************************************************************
*  DEFINES
*/

/* 
 * These defines are used when calling the IPT-COM SDK function IPTVosThreadSpawn
 * and need to be defined of the user to be adapted to the platform. For more
 * information see IPT-COM documentation and actual platform code added in the
 * IPT-COM SDK file vos.c for the function IPTVosThreadSpawn.
 */
/******************************************************************************
*   ADD CODE FOR PLATFORM (START)
******************************************************************************/
#define VRS_THREAD_POLICY             1

/* Add value and remove comments
#define VRS_THREAD_POLICY             <value>
*/
#ifndef VRS_THREAD_POLICY
  #error "VRS_THREAD_POLICY not defined"
#endif

#define VRS_THREAD_PRIORITY_CLIENT    100

/* Add value and remove comments
#define VRS_THREAD_PRIORITY_CLIENT    <value>
*/
#ifndef VRS_THREAD_PRIORITY_CLIENT
  #error "VRS_THREAD_PRIORITY_CLIENT not defined"
#endif
/*****************************************************************************
*   ADD CODE FOR PLATFORM (END)
******************************************************************************/



/*******************************************************************************
*  TYPEDEFS
*/


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/
VRS_RESULT vrsPlatformGetHwinfo(UINT32 bufferSize, UINT32 *pActSize, char **ppBuffer);
void vrsPlatformReleaseHwinfoBuffer(char* pBuffer);
void vrsPlatformReportDvsHw2Task(void);
VRS_RESULT vrsPlatformGetSwinfo(UINT32 bufferSize, UINT32 *pActSize, char **ppBuffer);
void vrsPlatformReleaseSwinfoBuffer(char* pBuffer);
void vrsPlatformReportDvsSw2Task(void);

#endif /* VRSPLATFORM_H */
/******************************************************************************/
