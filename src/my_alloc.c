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
		//printf("heap %p has %ld left\n", heap, heap->free_end_size);
		//printf("am alocat la pozitia: %p", block);
		return (block + 1);
	}
	else
	{
		//printf("asking for %ld space\n", size);
		heap = search_for_free_heap(size);
		if(heap)
		{
			block = search_for_free_block(size, heap);
			if(!block)
			{
				block = append_block(size, heap);
			}
			if(!block) // eroare de fragmentare; normal, nu ar trebui sa ajunga niciodata aici, biggest_fblock previne situatii de genu
			{
				printf("eroare de fragmentare!\n");
				return NULL;
			}
			return (block + 1);
		}
		else
		{
			//printf("no suitable heap found\n");
			heap = create_heap(size);
			if(heap == NULL)
			{
				printf("heap error in malloc");
				return NULL;
			}
			// heap top swap
			heap_top->next = heap;
			heap->prev = heap_top;
			heap->next = NULL;
			heap_top = heap;

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
