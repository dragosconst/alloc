/*
	Heap-urile reprezinta portiuni de memorie mapped la proces. Cand un heap e creat,
	e facut un block care contine tot spatiul de pe heap si e pus in bin-ul corespunzator.
*/

#include "my_alloc.h"

d_heap*
create_heap(size_t size)
{
	d_heap* heap;
	size = aligned_size(size);
	// pt size-uri in smallbin size, putem prealoca heap
	if(size < PREALLOC_THRESHOLD)
	{
		heap = mmap(NULL, 4 * getpagesize(), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		heap->all_size = 4 * getpagesize() - sizeof(d_heap) - sizeof(d_block);
	}
	else
	{ 	// la large bins si chestii mai mari de large bins nu prea pot optimiza cu prealocari
		heap = mmap(NULL, size + sizeof(d_block) + sizeof(d_heap), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		heap->all_size = closest_page_size(size + sizeof(d_block) + sizeof(d_heap)) - sizeof(d_heap) - sizeof(d_block);
	}
	printf("heap.c: newly created heap has size %zd\n", heap->all_size);
	if(!heap_top)
	{
		heap_top = heap;
		heap->next = heap;
		heap->prev = heap;
	}
	else
	{
		heap->next = heap_top->next;
		heap->prev = heap_top;
		heap_top->next->prev = heap;
		heap_top->next = heap;
		heap_top = heap;
	}

	// la final, trebuie adaugat un block aliniat in heap, care cuprinde tot heap-ul
	size_t align_size = heap->all_size;
	d_block* top_block = (void*)heap + sizeof(d_heap); // blocul cu tot size-ul cerut de user in el
	top_block->size = align_size;
	//printf("heap.c: new heap size is %ld\n", top_block->size);
	top_block->free = 1;
	top_block->last = 1;
	insert_block_in_bin(top_block);
	return heap;
}
