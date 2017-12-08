/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : dList.h
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

#ifndef DLIST_H
#define DLIST_H

/*******************************************************************************
*  INCLUDES
*/


/*******************************************************************************
*  DEFINES
*/
#define DLIST_INIT(pDLIST)  {{0,0,(DNODE*)pDLIST},0}


/*******************************************************************************
*  TYPEDEFS
*/
typedef struct dNode
{
    struct dNode* pNext;    /* Pointer to the next node in the list */
    struct dNode* pPrev;    /* Pointer to the previous node in the list */
    struct dNode* pSelf;    /* Pointer to the own node */
} DNODE;

/* NOTE: the DLIST data structure shall be handled as if it was hidden */
typedef struct
{
    DNODE node;             /* Internal header list node */
    INT32 count;            /* Internal number of nodes in the list */
} DLIST;


/*******************************************************************************
*  GLOBAL FUNCTIONS
*/
extern DNODE*   dListFind(const DLIST* pList, const DNODE* pNode);
extern DNODE*   dListFirst(const DLIST* pList);
extern DNODE*   dListLast(const DLIST* pList);
extern DNODE*   dListNext(const DNODE* pNode);
extern DNODE*   dListPrevious(const DNODE* pNode);
extern INT32    dListAdd(DLIST* pList, DNODE* pNode);
extern INT32    dListCount(const DLIST* pList);
extern INT32    dListDelete(DLIST* pList, DNODE* pNode);
extern INT32    dListInit(DLIST* pList);
extern INT32    dListInsert(DLIST* pList, DNODE* pPrev, DNODE* pNode);
extern INT32    dListValidate(const DLIST* pList);

#endif /* DLIST_H */
/******************************************************************************/
