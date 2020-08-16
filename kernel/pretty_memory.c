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
 * Author   : 	Yahia Farghaly Ashour
 *
 * Purpose  :	PrettyOS Memory Management Implementation.
 *
 * 				Memory Allocation and Deallocation routines similar to the known C functions "malloc()"
 * 				and "free()". Except the implementation here is deterministic and being in a constant time,
 * 				using memory partitions scheme.
 *
 * 				The memory management implementation here consists of a number of partitions,
 * 				definable by OS_CONFIG_MEMORY_PARTITION_COUNT. where every partition is divided into a fixed
 * 				number of memory blocks of a specific size determined when the partition is created.
 * 
 * 
 * 
 *				  +------->   Partition#01  <--------+
 *				  |                                  |
 *				  v                                  v
 *				  +----+----+----+----+----+----+----+
 *				  |    |    |    |    |    |    |    |
 *				  |    |    |    |    |    |    |    |
 *				  |    |    |    |    |    |    |    |
 *				  +----+----+----+----+----+----+----+
 *				  ^                    ^   ^
 *				  |                    |   |
 *				  +                    |   |
 *				start address          |   |
 *									   +   +
 *                                     Block
 *
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

#if(OS_CONFIG_MEMORY_EN == OS_CONFIG_ENABLE)

static OS_MEMORY  OS_Mem_PartitionPool[OS_CONFIG_MEMORY_PARTITION_COUNT];
static OS_MEMORY* volatile pMemoryPartitionFreeList;

/*
 * Function:  OS_MemoryPartitionCreate
 * ----------------------------------
 * Create a fixed-sized memory partition which will be managed by prettyOS.
 *
 * Arguments    :  partitionBaseAddr	is the starting address of the memory partition.
 *
 *				   blockCount			is the number of the memory blocks of the created memory partition.
 *
 *				   blockSizeInBytes		is the number of bytes per memory block.
 *
 * Returns      :  == ((OS_MEMORY*)0U)  if memory partition creation fails in case of invalid of arguments or no free partition.
 * 				   != ((OS_MEMORY*)0U)  is the created memory partition.
 *
 * 				   OS_ERRNO = { OS_ERR_NONE, OS_ERR_MEM_INVALID_ADDR, OS_ERR_MEM_INVALID_BLOCK_SIZE }
 */
OS_MEMORY*
OS_MemoryPartitionCreate (void* partitionBaseAddr, OS_MEMORY_BLOCK blockCount, OS_MEMORY_BLOCK blockSizeInBytes)
{
	OS_MEMORY* 	pMemoryPart;
	CPU_t08U* 	pBlock;
	void** 		pLinkAddr;
	CPU_SR_ALLOC();

	if(partitionBaseAddr == OS_NULL(void))							/* Must be a valid pointer.													*/
	{
		OS_ERR_SET(OS_ERR_MEM_INVALID_ADDR);
		return OS_NULL(OS_MEMORY);
	}

	if(blockSizeInBytes < sizeof(void*))							/* At least, The block of a partition must have the space of a pointer.		*/
	{
		OS_ERR_SET(OS_ERR_MEM_INVALID_BLOCK_SIZE);
		return OS_NULL(OS_MEMORY);
	}

	OS_CRTICAL_BEGIN();

	pMemoryPart = pMemoryPartitionFreeList;							/* Allocate a memory partition structure.									*/
	if(pMemoryPart != OS_NULL(OS_MEMORY))
	{
		pMemoryPartitionFreeList = (OS_MEMORY*)pMemoryPartitionFreeList->nextFreeBlock;
	}

	OS_CRTICAL_END();

	if(pMemoryPart == OS_NULL(OS_MEMORY))							/* If no available memory partition structure is ...						*/
	{
		OS_ERR_SET(OS_ERR_MEM_INVALID_ADDR);
		return OS_NULL(OS_MEMORY);									/* ... return NULL.															*/
	}

	pLinkAddr 	= (void**)partitionBaseAddr;						/* Get the address of the first memory block in the memory partition.		*/
	pBlock		= (CPU_t08U*)partitionBaseAddr;						/* Get the address of the first byte in the first memory block.				*/

	/* 										Create an implicit linked list of free memory blocks.												*/
	/*

	                                            +-> blockSizeInBytes  <-+



	                   +-----------------------+------------------------+-------------------------+----------------------------+
	                   |     ||                |     ||                 |     ||                  |     ||                     |
	                   | &pBlock#01 +          | &pBlock#02 +           |  &pBlock#03 +           | &pBlock#04 +               +----> NULL
	                   |     ||     |          |     ||     |           |     ||      |           |     ||     |               |
	                   |     ||     |          |     ||     |           |     ||      |           |     ||     |               |
	                   +-----------------------+------------------------+-------------------------+----------------------------+
	                                |                       |                         |                        |
	                   ^            |          ^            |           ^             |           ^            |               ^
	                   |            |          |            |           |             |           |            |               |
	                   |            |          |            |           |             |           |            |               |
	                   +            |          +            |           +             |           +            |               +
	   +-----------> pBlock         +----->  pBlock#01      +------>  pBlock#02       +------> pBlock#03       +-----------> pBlock#04
	   |
	   |
	   |
	   |
	   +
	pLinkAddr

	*/
	for(CPU_t32U i = 0; i < (blockCount - 1U);++i)
	{
		pBlock 		+= (blockSizeInBytes);							/* Get the address of the next memory block ...								*/
		*pLinkAddr 	= (void*)pBlock;								/* ... Store this address in the current memory block ...					*/
		pLinkAddr	= (void**)pBlock;								/* Then move to the next memory block.										*/
	}

	*pLinkAddr		= ((void*)0U);									/* Last memory block address points to NULL.								*/

	OS_CRTICAL_BEGIN();

	pMemoryPart->partitionBaseAddr  = partitionBaseAddr;			/* The being of the partition.												*/
	pMemoryPart->nextFreeBlock      = partitionBaseAddr;			/* The next address of the memory block.									*/
	pMemoryPart->blockSize          = blockSizeInBytes;				/* Block size in bytes.														*/
	pMemoryPart->blockCount         = blockCount;					/* Number of blocks inside this partition.									*/
	pMemoryPart->blockFreeCount     = blockCount;					/* Number of free block inside this partition. 								*/

	OS_CRTICAL_END();

	OS_ERR_SET(OS_ERR_NONE);
	return (pMemoryPart);
}

/*
 * Function:  OS_MemoryAllocateBlock
 * ---------------------------------
 * Allocate a free block from a valid memory partition.
 *
 * Arguments    :  pMemoryPart		is a pointer to a valid memory partition structure.
 *
 * Returns      :  == ((void*)0U)  if no free memory block is available or invalid pointer of memory partition structure.
 * 				   != ((void*)0U)  is the requested block of memory.
 *
 * 				   OS_ERRNO = { OS_ERR_NONE, OS_ERR_MEM_INVALID_ADDR, OS_ERR_MEM_NO_FREE_BLOCKS }
 */
void*
OS_MemoryAllocateBlock (OS_MEMORY* pMemoryPart)
{
	void* pBlock;
	CPU_SR_ALLOC();

	if(pMemoryPart == OS_NULL(OS_MEMORY))							/* Must be a valid pointer.													*/
	{
		OS_ERR_SET(OS_ERR_MEM_INVALID_ADDR);
		return OS_NULL(OS_MEMORY);
	}

	OS_CRTICAL_BEGIN();

	if(pMemoryPart->blockFreeCount > 0U)							/* See if there are any available blocks.									*/
	{
		pBlock = pMemoryPart->nextFreeBlock;						/* Point to the next free memory block.										*/
		pMemoryPart->nextFreeBlock = *(void**)pBlock;				/* Set the next free memory block to point to the next new free block.		*/
		--(pMemoryPart->blockFreeCount);							/* Decrease the number of free blocks by 1 .								*/
		OS_CRTICAL_END();
		OS_ERR_SET(OS_ERR_NONE);
		return (pBlock);
	}

	OS_CRTICAL_END();
	OS_ERR_SET(OS_ERR_MEM_NO_FREE_BLOCKS);							/* No free memory block in this partition.									*/
	return OS_NULL(void);											/* Return NULL.																*/
}

/*
 * Function:  OS_MemoryRestoreBlock
 * --------------------------------
 * Free/Restore a block to a valid memory partition.
 *
 * Arguments    :  	pMemoryPart		is a pointer to a valid memory partition structure.
 *
 * 					pBlock			is a pointer to the released block of the partition pointed by 'pMemoryPart'
 *
 * Returns      :	None.
 *
 * 				   	OS_ERRNO = { OS_ERR_NONE, OS_ERR_MEM_INVALID_ADDR, OS_ERR_MEM_FULL_PARTITION }
 *
 * Note(s)		:	This function is not aware if the returned block is the actually block which is allocated
 * 					from the given partition 'pMemoryPart'.
 * 					So, Caution must be considered. Otherwise, the software can be crashed.
 */
void
OS_MemoryRestoreBlock (OS_MEMORY* pMemoryPart, void* pBlock)
{
	CPU_SR_ALLOC();

	if(pMemoryPart == OS_NULL(OS_MEMORY))							/* Must be a valid pointer.													*/
	{
		OS_ERR_SET(OS_ERR_MEM_INVALID_ADDR);
		return;
	}

	if(pBlock == OS_NULL(void))										/* Must be a valid pointer.													*/
	{
		OS_ERR_SET(OS_ERR_MEM_INVALID_ADDR);
		return;
	}

	OS_CRTICAL_BEGIN();

	if(pMemoryPart->blockFreeCount < pMemoryPart->blockCount)		/* Assert that not all blocks are returned.									*/
	{
		*(void**)pBlock = pMemoryPart->nextFreeBlock;				/* Insert the free block into the free memory block list.					*/
		pMemoryPart->nextFreeBlock = pBlock;						/* Adjust the new free memory block list head. 								*/
		++(pMemoryPart->blockFreeCount);							/* Increase the number of free blocks by 1 .								*/
		OS_CRTICAL_END();
		OS_ERR_SET(OS_ERR_NONE);
		return;
	}

	OS_CRTICAL_END();
	OS_ERR_SET(OS_ERR_MEM_FULL_PARTITION);							/* ALL memory blocks have returned to the memory partition.					*/
	return;
}

/*
 * Function:  OS_Memory_Init
 * --------------------
 * Initialize The Memory Partition Manager.
 *
 * Arguments    :  None.
 *
 * Returns      :  None.
 *
 * Note(s)      :   1) This function is for internal use.
 */
void OS_Memory_Init (void)
{
	OS_MemoryByteClear((CPU_t08U*)&OS_Mem_PartitionPool[0], sizeof(OS_Mem_PartitionPool));			/* Clear Structure data.						*/

	for(CPU_t32U idx = 0; idx < OS_CONFIG_MEMORY_PARTITION_COUNT - 1; ++idx)						/* Point to a chain of OS Memory Structures.	*/
	{
		OS_Mem_PartitionPool[idx].nextFreeBlock = (void*)&OS_Mem_PartitionPool[idx+1];
	}

	OS_Mem_PartitionPool[OS_CONFIG_MEMORY_PARTITION_COUNT - 1].nextFreeBlock = OS_NULL(void);

	pMemoryPartitionFreeList = &OS_Mem_PartitionPool[0];											/* Point to the first Free OS Memory Structure.	*/
}

#endif	/* OS_CONFIG_MEMORY_EN 	*/
