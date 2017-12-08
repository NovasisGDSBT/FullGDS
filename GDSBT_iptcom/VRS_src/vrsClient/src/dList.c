/*******************************************************************************
* COPYRIGHT   : (c) 2015 Bombardier Transportation
********************************************************************************
* PROJECT     : VRS-IP SDK
*
* MODULE      : dList.c
*
* DESCRIPTION : 
*
********************************************************************************
* HISTORY  :
* 
*******************************************************************************/


/*******************************************************************************
* DOCUMENTATION INFORMATION */

/* DOCGROUP: vrsDlistCmnApiDescription
*\latex
* The VRS Client component includes a double linked list with self references.
* The dList API supports creating and maintaining a double linked list.
* The user supplies a list descriptor (DLIST) that will contain pointers to
* the first, last and own node as well as the number of nodes in the list.
* The nodes can be any user defined structure as long as space for the DNODE
* pointers are reserved as the structures first elements.
*
* Note: If the list can be accessed from multiple threads, it must be protected,
* e.g. a semaphore.
* \begin{verbatim}
* \end{verbatim}
* \textsc{Empty List:}
* \begin{verbatim}
*        -------------
*        |           |
*        V           |
*      -----------   |
*      |         |   |
*      | SELF --------
*      |         |
*      | HEAD ------------------
*      |         |             |
*      | TAIL ----------       |
*      |         |     |       |
*      | count=0 |     V       V
*      |         |   -----   -----
*      -----------    ---     ---
*                      -       -
* \end{verbatim}
* \textsc{Non Empty List:}
* \begin{verbatim}
*        -------------             -----------     -----------     -----------
*        |           |             |         |     |         |     |         |
*        V           |             V         |     V         |     V         |
*      -----------   |           ---------   |   ---------   |   ---------   |
*      |         |   |           |       |   |   |       |   |   |       |   |
*      | SELF --------           | pSelf -----   | pSelf -----   | pSelf -----
*      |         |               |       |       |       |       |       |
*      | HEAD ------------------>| pNext ------->| pNext ------->| pNext -------
*      |         |               |       |       |       |       |       |     |
*      | TAIL --------     ------- pPrev |<------- pPrev |<------- pPrev |     |
*      |         |   |     |     |       |       |       |       |       |     |
*      | count=3 |   |     V     |       |       |       |   --->|       |     V
*      |         |   |   -----   |       |       |       |   |   |       |   -----
*      -----------   |    ---    |  ...  |       |  ...  |   |   |  ...  |    ---
*                    |     -                                 |                 -
*                    |                                       |
*                    -----------------------------------------
* \end{verbatim}
*\endlatex
*/

/* DOCGROUP: vrsDlistCmnDesignComponent
*    VRS Design Description
*/

/* DOCGROUP: vrsDlistCmnTestComponent
*    VRS Test Description
*/


/*******************************************************************************
*  INCLUDES
*/
#include "iptcom.h"
#include "vrsCommon.h"
#include "dList.h"


/*******************************************************************************
* DEFINES 
*/
/* defines mapping DLIST head, tail and self reference using the internal node */
#define HEAD node.pNext
#define TAIL node.pPrev
#define SELF node.pSelf


/*******************************************************************************
* TYPEDEFS
*/


/*******************************************************************************
* GLOBALS 
*/


/*******************************************************************************
* LOCALS
*/


/*******************************************************************************
* LOCAL FUNCTIONS
*/


/*******************************************************************************
* GLOBAL FUNCTIONS
*/


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function searches for a node in the list. Note that the data contents
*   of the node is not checked at all.
*
* RETURNS:
*   Pointer to the node, or NULL if the node is not found.
*/
DNODE* dListFind(
    const DLIST* pList,   /* pointer to list descriptor */
    const DNODE* pNode)   /* pointer to node to find */
{
    if(pList == 0 || pNode == 0)
    {
        return 0;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return 0;
    }
    else if(pNode != pNode->pSelf)
    {
        /* do nada only nodes that are in any list has the self reference set */
        return 0;
    }
    else
    {
        DNODE* pSearchNode = dListFirst(pList);

        /* search for the node */
        while(pSearchNode != 0)
        {
            if(pSearchNode == pNode)
            {
                break;
            }
            pSearchNode = dListNext(pSearchNode);
        }
        return pSearchNode;
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function returns the first node in the list.
*
* RETURNS:
*   Pointer to the first node, or NULL if the list is empty.
*/
DNODE* dListFirst(
    const DLIST* pList)   /* pointer to list descriptor */
{
    if(pList == 0)
    {
        return 0;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return 0;
    }
    else
    {
        if(pList->HEAD != 0)
        {
            if((pList->HEAD) != (pList->HEAD)->pSelf)
            {
                return 0;
            }
        }
        return (pList->HEAD);
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function returns the last node in the list.
*
* RETURNS:
*   Pointer to the last node, or NULL if the list is empty.
*/
DNODE* dListLast(
    const DLIST* pList)   /* pointer to list descriptor */
{
    if(pList == 0)
    {
        return 0;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return 0;
    }
    else
    {
        if(pList->TAIL != 0)
        {
            if((pList->TAIL) != (pList->TAIL)->pSelf)
            {
                return 0;
            }
        }
        return (pList->TAIL);
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function returns the node after pNode in a list.
*
* RETURNS:
*   Pointer to the next node, or NULL if this was the last node.
*/
DNODE* dListNext(
    const DNODE* pNode)   /* pointer to current node */
{
    if(pNode == 0)
    {
        return 0;
    }
    else if(pNode != pNode->pSelf)
    {
        return 0;
    }
    else
    {
        if(pNode->pNext != 0)
        {
            if((pNode->pNext) != (pNode->pNext)->pSelf)
            {
                return 0;
            }
        }
        return (pNode->pNext);
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function returns a pointer to the node preceding pNode in a list.
*
* RETURNS:
*   Pointer to the previous node, or NULL if this was the first node.
*/
DNODE* dListPrevious(
    const DNODE* pNode)   /* pointer to current node */
{
    if(pNode == 0)
    {
        return 0;
    }
    else if(pNode != pNode->pSelf)
    {
        return 0;
    }
    else
    {
        if(pNode->pPrev != 0)
        {
            if((pNode->pPrev) != (pNode->pPrev)->pSelf)
            {
                return 0;
            }
        }
        return (pNode->pPrev);
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function adds a node to the end of the list.
*
* RETURNS:
*   0 on success, otherwise error.
*/
INT32 dListAdd(
    DLIST* pList,   /* pointer to list descriptor */
    DNODE* pNode)   /* pointer to node to add */
{
    if(pList == 0 || pNode == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(dListFind(pList, pNode) != 0)
    {
        return VRS_DATA_DUPLICATED;
    }
    else
    {
        /* set node pointers */
        pNode->pNext = 0;
        pNode->pSelf = pNode;
        pNode->pPrev = pList->TAIL;

        if(pList->HEAD == 0)
        {
            /* add first entry */
            pList->HEAD = pNode;
            pList->TAIL = pNode;
            ++(pList->count);
            return VRS_OK;
        }
        else
        {
            /* add to end of list */
            (pList->TAIL)->pNext = pNode;
            pList->TAIL = pNode;
            ++(pList->count);
            return VRS_OK;
        }
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function returns the number of nodes in the list.
*
* RETURNS:
*   The number of nodes in the list.
*/
INT32 dListCount(
    const DLIST* pList)   /* pointer to list descriptor */
{
    if(pList == 0)
    {
        return 0;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return 0;
    }
    else
    {
        return (pList->count);
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function deletes a node from the list.
*
* RETURNS:
*   0 on success, otherwise error.
*/
INT32 dListDelete(
    DLIST* pList,   /* pointer to list descriptor */
    DNODE* pNode)   /* pointer to node to delete */
{
    if(pList == 0 || pNode == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(pNode != pNode->pSelf)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(dListFind(pList, pNode) == 0)
    {
        return VRS_DATA_NOT_FOUND;
    }
    else
    {
        if(pList->HEAD == pList->TAIL)
        {
            /* remove last node */
            pList->HEAD = 0;
            pList->TAIL = 0;
            --(pList->count);
        }
        else if(pList->HEAD == pNode)
        {
            /* remove first node */
            pList->HEAD = pNode->pNext;
            (pList->HEAD)->pPrev = 0;
            --(pList->count);
        }
        else if(pList->TAIL == pNode)
        {
            /* remove last node */
            pList->TAIL = pNode->pPrev;
            (pList->TAIL)->pNext = 0;
            --(pList->count);
        }
        else
        {
            /* remove middle node */
            (pNode->pNext)->pPrev = pNode->pPrev;
            (pNode->pPrev)->pNext = pNode->pNext;
            --(pList->count);
        }
        /* prevent node from being used to access the list */
        pNode->pNext = 0;
        pNode->pPrev = 0;
        pNode->pSelf = 0;

        return VRS_OK;
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function initialize a list descriptor to an empty list.
*   As an alternative to calling this function, the list descriptor can be set
*   using a macro where the list variable is defined. \\
*   Example: static DLIST myList = DLIST_INIT(&myList)
*
* RETURNS:
*   0 on success, otherwise error.
*/
INT32 dListInit(
    DLIST* pList)   /* pointer to list descriptor */
{
    if(pList == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else
    {
        pList->HEAD = 0;
        pList->TAIL = 0;
        pList->SELF = (DNODE*)pList;
        pList->count = 0;

        return VRS_OK;
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function inserts a node in the list. The new node is added after the
*   node pPrev. If pPrev is NULL, the node is inserted at the head of the 
*   list.
*
* RETURNS:
*   0 on success, otherwise error.
*/
INT32 dListInsert(
    DLIST* pList,   /* pointer to list descriptor */
    DNODE* pPrev,   /* pointer to previous node */
    DNODE* pNode)   /* pointer to node that shall be added */
{
    if(pList == 0 || pNode == 0 )
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if(dListFind(pList, pNode) != 0)
    {
        return VRS_DATA_DUPLICATED;
    }
    else
    {
        if(pPrev == 0)
        {
            /* set node pointers */
            pNode->pNext = pList->HEAD;
            pNode->pSelf = pNode;
            pNode->pPrev = 0;
            /* insert first in list */
            if(pList->HEAD == 0)
            {
                /* add first entry */
                pList->HEAD = pNode;
                pList->TAIL = pNode;
                ++(pList->count);
                return VRS_OK;
            }
            else
            {
                /* add to start of list */
                (pList->HEAD)->pPrev = pNode;
                pList->HEAD = pNode;
                ++(pList->count);
                return VRS_OK;
            }
        }
        else if(pPrev != pPrev->pSelf)
        {
            return VRS_MEMORY_POINTER_ERROR;
        }
        else if(dListFind(pList, pPrev) == 0)
        {
            /* previous node not found */
            return VRS_DATA_NOT_FOUND;
        }
        else if(pList->TAIL == pPrev)
        {
            /* add to end of list */
            return dListAdd(pList, pNode);
        }
        else
        {
            /* set node pointers */
            pNode->pSelf = pNode;
            /* insert in list */
            pNode->pNext = pPrev->pNext;
            (pPrev->pNext)->pPrev = pNode;
            pNode->pPrev = pPrev;
            pPrev->pNext = pNode;
            ++(pList->count);
            return VRS_OK;
        }
    }
}


/*******************************************************************************
* DOCGROUP: vrsDlistCmnApiGroup
*
* DESCRIPTION:
*   This function validates the list by checking for inconsistencies.
*
* RETURNS:
*   0 if valid list, otherwise error.
*/
INT32 dListValidate(
    const DLIST* pList)   /* pointer to list descriptor */
{
    if(pList == 0)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else if((DNODE*)pList != pList->SELF)
    {
        return VRS_MEMORY_POINTER_ERROR;
    }
    else
    {
        if(pList->HEAD == 0 || pList->TAIL == 0)
        {
            /* list is empty */
            if(pList->HEAD == 0 && pList->TAIL == 0 && pList->count == 0)
            {
                return VRS_OK;
            }
            else if(pList->count != 0)
            {
                return VRS_DATA_INVALID;
            }
            else
            {
                return VRS_MEMORY_POINTER_ERROR;
            }
        }
        else if(pList->HEAD == pList->TAIL)
        {
            /* only one node in list */
            if(pList->count != 1)
            {
                return VRS_DATA_INVALID;
            }
            else if((pList->HEAD)->pNext != 0 || (pList->HEAD)->pPrev != 0)
            {
                return VRS_MEMORY_POINTER_ERROR;
            }
            else if((pList->HEAD) != (pList->HEAD)->pSelf)
            {
                return VRS_MEMORY_POINTER_ERROR;
            }
            else
            {
                return VRS_OK;
            }
        }
        else
        {
            /* several nodes in list */
            DNODE* pTmpNode = dListFirst(pList);
            DNODE* pLastNode = 0;
            INT32 counter = 0;

            if((pList->HEAD)->pPrev != 0 || (pList->TAIL)->pNext != 0)
            {
                /* first and/or last node not null terminated */
                return VRS_MEMORY_POINTER_ERROR;
            }
            else if(pTmpNode == 0)
            {
                /* dListFirst detected empty list or faulty self reference */
                return VRS_MEMORY_INVALID;
            }
            else
            {
                while(pTmpNode != 0)
                {
                    if(pTmpNode->pNext != 0)
                    {
                        if((pTmpNode->pNext)->pPrev != pTmpNode)
                        {
                            /* next and previous references mismatch */
                            return VRS_MEMORY_POINTER_ERROR;
                        }
                    }
                    ++counter;
                    pLastNode = pTmpNode; /* keep track of last node */
                    pTmpNode = dListNext(pTmpNode);
                    /* node self reference are checked in dListNext */
                }
            }
            /* check last node and number of nodes */
            if(dListLast(pList) != pLastNode)
            {
                return VRS_MEMORY_POINTER_ERROR;
            }
            else if(pList->count != counter)
            {
                return VRS_DATA_INVALID;
            }
            else
            {
                return VRS_OK;
            }
        }
    }
}


