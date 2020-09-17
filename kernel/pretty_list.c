/*****************************************************************************
MIT License

Copyright (c) 2020 Yahia Farghaly Ashour

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

/*
 * Author   : Yahia Farghaly Ashour
 *
 * Purpose  : PrettyOS sorted doubly linked list implementation.
 * 				The sorting is in ascending order.
 *
 * 				It's similar to xList implementation from FreeRTOS code except
 * 				the List structure is different.
 *
 * 				This picture shows the xList structure top overview
 * 				https://www.aosabook.org/images/freertos/freertos-figures-full-ready-list.png
 *
 * Language:  C
 *
 * Set 1 tab = 4 spaces for better comments readability.
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include "pretty_os.h"
#include "pretty_shared.h"

#if(OS_AUTO_CONFIG_INCLUDE_LIST == OS_CONFIG_ENABLE)
void
list_Init(List * const list)
{
	list->head = list->end = ((List_Item*)0U);
	list->itemsCnt = 0U;
}

void
listItem_Init(List_Item * const listItem)
{
	listItem->prev = listItem->next = ((List_Item*)0U);
	listItem->pList = listItem->pOwner = ((void*)0U);
	listItem->itemVal = 0U;
}

void
listItemInsert (List * const list, List_Item * const listItem)
{

	List_Item* pCurr, *pPrev;

	if(list->itemsCnt == 0U)
	{
		list->head = list->end = listItem;
		listItem->prev = listItem->next = (void*)0U;
		listItem->pList = list;
		(list->itemsCnt)++;
		return;
	}

	pCurr = (List_Item*)(list->head);
	pPrev = pCurr->prev;

	for(;pCurr && (pCurr->itemVal <= listItem->itemVal);)
	{
		/*	Loop Till find the element which is bigger than the inserted one. 	*/
		pPrev = pCurr;
		pCurr = pCurr->next;
	}

    listItem->next 	= pCurr;
    listItem->prev	= pPrev;

	if(pPrev && pCurr)
	{
		/* Insert In the middle					*/
	    pPrev->next 	= listItem;
	    pCurr->prev 	= listItem;
	}
	else
	{
		/* Insert Before Head					*/
		if(pCurr == list->head)
		{
			pCurr->prev 	= listItem;
		    list->head 		= listItem;
		}
		/* Insert After Tail					*/
		if(pPrev == list->end)
		{
		    list->end->next = listItem;
		    list->end 		= listItem;
		}
	}
		/* Remember which list you're belong to	*/
    listItem->pList = list;

    (list->itemsCnt)++;
}

CPU_tWORD ListItemRemove( List_Item * const pItemToRemove )
{

    List * const pList = pItemToRemove->pList;

    if(pItemToRemove->next)
    {
    	pItemToRemove->next->prev = pItemToRemove->prev;
    }

    if(pItemToRemove->prev)
    {
    	pItemToRemove->prev->next = pItemToRemove->next;
    }

    if( pList->end == pItemToRemove )
    {
    	pList->end = pItemToRemove->prev;
    }

    if( pList->head == pItemToRemove )
    {
    	pList->head = pItemToRemove->next;
    }

    pItemToRemove->pList = ((List*)0U);
    (pList->itemsCnt)--;

    if(pList->itemsCnt == 0)
    {
    	list_Init(pList);
    }

    return pList->itemsCnt;
}

#endif		/* OS_AUTO_CONFIG_INCLUDE_LIST	*/
