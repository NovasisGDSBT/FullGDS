/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsReportDvsSw2.h
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

#ifndef VRSREPORTDVSSW2_H
#define VRSREPORTDVSSW2_H

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


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/
extern VRS_RESULT  VRSReportDvsSw2_initiate(void);
extern VRS_RESULT  VRSReportDvsSw2_process(void);
extern VRS_RESULT  VRSReportDvsSw2_terminate(UINT32 remainingTime);
extern UINT32      VRSReportDvsSw2_getVersion(void);
extern VRS_SERVICE_STATUS VRSReportDvsSw2_getStatus(void);
extern VRS_RESULT  VRSReportDvsSw2_getErrorCode(void);
extern const char* VRSReportDvsSw2_getErrorString(VRS_RESULT errorCode);

#endif /* VRSREPORTDVSSW2_H */
/******************************************************************************/

