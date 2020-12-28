#include "my_alloc.h"

void*
my_alloc(size_t size)
{
	if(size <= 0)
		return NULL;

	d_heap* heap;
	d_block* block;

	if(!heap_top)
	{
		heap = create_heap(size);
		if(heap == NULL)
		{
			return NULL;
		}
		heap->next = NULL;
		heap->prev = NULL;
		heap_top = heap;

		block = append_block(size, heap);
		if(!block)
		{
			printf("unpsecified error");
			return NULL;
		}
		//printf("am alocat la pozitia: %p", block);
		return (block + 1);
	}
	else
	{
		heap = search_for_free_heap(size);
		if(heap)
		{
			block = search_for_free_block(size, heap);
			if(!block)
			{
				block = append_block(size, heap);
			}
			if(!block) // eroare de fragmentare
			{
				printf("eroare de fragmentare!\n");
				return NULL;
			}
			return (block + 1);
		}
		else
		{
			if(size > SMALL_BLOCK_SIZE)
			{
				heap = create_heap(size);
				if(heap == NULL)
				{
					return NULL;
				}
				// heap top swap
				heap_top->next = heap;
				heap->prev = heap_top;
				heap->next = NULL;
				heap_top = heap;
			}
			else
			{	// pt blocuri mici e probabil mai eficient doar sa largim heap-ul vechi
				heap = heap_of_same_type(size);
				if(heap == NULL) // nu avem heap de tipul small\vsmall
				{	// deci tot trebuie creat heap-ul
					heap = create_heap(size);
					if(heap == NULL)
					{
						return NULL;
					}
					// heap top swap
					heap_top->next = heap;
					heap->prev = heap_top;
					heap->next = NULL;
					heap_top = heap;
				}
				else
				{
					heap = expand_heap(heap, size);
					if(heap == NULL)
						return NULL;
				}
			}
			block = append_block(size, heap);
			if(!block)
			{
				printf("unpsecified error\n");
				return NULL;
			}
			return (block + 1);
		}
	}
}
