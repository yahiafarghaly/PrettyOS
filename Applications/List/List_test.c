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
 * Author   	: 	Yahia Farghaly Ashour
 *
 * Purpose  	:	Test the PrettyOS's list data structure.
 *
 * Language		:  	C
 */

/*
*******************************************************************************
*                               Includes Files                                *
*******************************************************************************
*/
#include <bsp.h>
#include <pretty_os.h>
#include <pretty_shared.h>
#include <uartstdio.h>

void App_Hook_TaskIdle(void)
{

}

int main()
{
	List orderedQueue;
	List_Item* pListItem, *pListTemp;
	List_Item item[20];

	for(long i = 0; i < 20; i++)
	{
		listItem_Init(&item[i]);
	}

	list_Init(&orderedQueue);

	for(long i = 10; i >= 0; i--)
	{
		item[i].itemVal = i;
		listItemInsert(&orderedQueue,&item[i]);
	}

	printf("List: ");

	pListItem = orderedQueue.head;

	for(;pListItem;)
	{
		printf("%d ", pListItem->itemVal);
		pListItem = pListItem->next;
	}

	printf("\n");
	printf("Removing List[5] = %d\n",item[5].itemVal);

	ListItemRemove(&item[5]);

	printf("Removing List[0] = %d\n",item[0].itemVal);

	ListItemRemove(&item[0]);

	printf("Removing List[10] = %d\n",item[10].itemVal);

	ListItemRemove(&item[10]);

	printf("List: ");

	pListItem = orderedQueue.head;

	for(;pListItem;)
	{
		printf("%d ", pListItem->itemVal);
		pListItem = pListItem->next;
	}

	printf("\n");

	printf("Remove all\n");

	pListItem = orderedQueue.head;

	for(;orderedQueue.head;)
	{
		if(pListItem)
			printf("Removed %d\n",pListItem->itemVal);
		pListTemp = pListItem->next;
		ListItemRemove(pListItem);
		pListItem = pListTemp;
	}

	printf("List: ");

	pListItem = orderedQueue.head;

	for(;pListItem;)
	{
		printf("%d ", pListItem->itemVal);
		pListItem = pListItem->next;
	}

	printf("\n");
}
