/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrsReportDvsHw2.h
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

#ifndef VRSREPORTDVSHW2_H
#define VRSREPORTDVSHW2_H

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

extern VRS_RESULT  VRSReportDvsHw2_initiate(void);
extern VRS_RESULT  VRSReportDvsHw2_process(void);
extern VRS_RESULT  VRSReportDvsHw2_terminate(UINT32 remainingTime);
extern UINT32      VRSReportDvsHw2_getVersion(void);
extern VRS_SERVICE_STATUS VRSReportDvsHw2_getStatus(void);
extern VRS_RESULT  VRSReportDvsHw2_getErrorCode(void);
extern const char* VRSReportDvsHw2_getErrorString(VRS_RESULT errorCode);

#endif /* VRSREPORTDVSHW2_H */
/******************************************************************************/

