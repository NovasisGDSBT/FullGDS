/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : vrs.h
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

#ifndef VRS_H
#define VRS_H

/*******************************************************************************
*  INCLUDES
*/


/*******************************************************************************
*  DEFINES
*/
#define VRS_SDK_VERSION   0
#define VRS_SDK_RELEASE   0
#define VRS_SDK_UPDATE    0
#define VRS_SDK_EVOLUTION 0
#define VRS_SDK_VERSION_FULL \
    ((VRS_SDK_VERSION<<24) + (VRS_SDK_RELEASE<<16) + (VRS_SDK_UPDATE<<8) + VRS_SDK_EVOLUTION)
#define VRS_SDK_VERSION_STRING \
    VRS_DTOA(VRS_SDK_VERSION)"."VRS_DTOA(VRS_SDK_RELEASE)"." \
    VRS_DTOA(VRS_SDK_UPDATE)"."VRS_DTOA(VRS_SDK_EVOLUTION)
#define VRS_DL_SUPPORT 0x01000100

/* help macros for creating the version string */
#define VRS_DTOA_HLP(V) #V
#define VRS_DTOA(V)     VRS_DTOA_HLP(V)

/* Error code: VRS type definitions */
#define VRS_TYPE_WARNING    0x40000000
#define VRS_TYPE_ERROR      0x80000000
#define VRS_TYPE_EXCEPTION  0xc0000000
/* Error code: VRS global/local definitions */
#define VRS_ERR_LOCAL       0x00008000
/* Error encoding:

    <type>_<instance><function>_>global/local>_<sub_code>

    name            size        comment
    ----------------------------------------------------------------------------
    type            2 bits
    instance        6 bits      only available via c++ api
    function        8 bits      only available via c++ api
    global/local    1 bit
    sub_code        15 bits     0 (0x0) to 32767 (0x7fff)

   Internal sub code semantics:
    sub code     0 indicates that no errors are detected
    sub code <1000 are unspecified errors (only use during development)
    sub code  1000 is reserved for component (COMP) errors
    sub code  2000 is reserved for communication (COM) errors
    sub code  3000 is reserved for memory (MEM) errors
    sub code  4000 is reserved for semaphore (SEM) errors
    sub code  5000 is reserved for thread (THREAD) errors
    sub code  6000 is reserved for file (FILE) errors
    sub code  7000 is reserved for data (DATA) errors
    sub code  8000 is reserved for message (MSG) errors
*/
#define VRS_OK                      (VRS_RESULT)0
#define VRS_ERROR                   (VRS_RESULT)(VRS_TYPE_ERROR     | VRS_ERR_LOCAL |    1)
#define VRS_WARNING                 (VRS_RESULT)(VRS_TYPE_WARNING   | VRS_ERR_LOCAL |    2)
#define VRS_EXCEPTION               (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL |    3)
#define VRS_COMP_ALREADY_STARTED    (VRS_RESULT)(VRS_TYPE_WARNING   | VRS_ERR_LOCAL | 1001)
#define VRS_COMP_WRONG_VERSION      (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 1002)
#define VRS_COMP_NOT_STARTED        (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 1003)
#define VRS_COMP_CLI_NOT_STARTED    (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 1004)
#define VRS_COMP_SRV_NOT_STARTED    (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 1005)
#define VRS_COMMUNICATION_ERROR     (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 2001)
#define VRS_COMMUNICATION_TIMEOUT   (VRS_RESULT)(VRS_TYPE_WARNING   | VRS_ERR_LOCAL | 2002)
#define VRS_COMMUNICATION_INVALID   (VRS_RESULT)(VRS_TYPE_WARNING   | VRS_ERR_LOCAL | 2003)
#define VRS_MEMORY_ALLOCATION_ERROR (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 3001)
#define VRS_MEMORY_POINTER_ERROR    (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 3002)
#define VRS_MEMORY_INVALID          (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 3003)
#define VRS_SEMAPHORE_NOT_CREATED   (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 4001)
#define VRS_SEMAPHORE_TAKEN         (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 4002)
#define VRS_SEMAPHORE_ERROR         (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 4003)
#define VRS_THREAD_NOT_CREATED      (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 5001)
#define VRS_THREAD_ERROR            (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 5002)
#define VRS_FILE_ACCESS_ERROR       (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 6001)
#define VRS_FILE_CONFIG_ERROR       (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 6002)
#define VRS_DATA_INVALID            (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 7001)
#define VRS_DATA_BAD_SIZE           (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 7002)
#define VRS_DATA_NOT_FOUND          (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 7003)
#define VRS_DATA_DUPLICATED         (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 7004)
#define VRS_DATA_STRING_ERROR       (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 7005)
#define VRS_MESSAGE_ERROR           (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 8001)
#define VRS_MESSAGE_BAD_HEADER      (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 8002)
#define VRS_MESSAGE_WRONG_VERSION   (VRS_RESULT)(VRS_TYPE_EXCEPTION | VRS_ERR_LOCAL | 8003)


/*******************************************************************************
*  TYPEDEFS
*/

/* 
* DOCGROUP: vrsCmnApiTypedef 
*/
typedef INT32 VRS_RESULT;

/* 
* DOCGROUP: vrsCmnApiTypedef 
*/
/* vrs service states */
typedef enum
{
    VRS_SERVICE_STOPPED  = 0x0000,  /*     0 */
    VRS_SERVICE_STOPPING = 0x000f,  /*    15 */
    VRS_SERVICE_STARTING = 0x00f0,  /*   240 */
    VRS_SERVICE_RUNNING  = 0x0f00,  /*  3840 */
    VRS_SERVICE_ERROR    = 0xf000   /* 61440 */
} VRS_SERVICE_STATUS;


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/

#endif /* VRS_H */
/******************************************************************************/
