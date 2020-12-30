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
	if(size < BIG_BLOCK_SIZE / 2)
	{
		heap = mmap(NULL, 4 * getpagesize(), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		heap->all_size = 4 * getpagesize() - sizeof(d_heap);
	}
	else
	{ 	// la large bins si chestii mai mari de large bins nu prea pot optimiza cu prealocari
		heap = mmap(NULL, size + sizeof(d_heap), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		heap->all_size = closest_page_size(size + sizeof(d_heap)) - sizeof(d_heap);
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
		heap_top->next = heap;
		heap_top = heap;
	}

	// la final, trebuie adaugat un block aliniat in heap, care cuprinde tot heap-ul
	size_t align_size = heap->all_size - sizeof(d_block);
	if(align_size % 8 == 0 && align_size <= VBIG_BLOCK_SIZE * 2)
	{
		size_t offset = 8 - (sizeof(d_block) % 8);
		offset += 8 - (sizeof(d_block) % 8);
		offset %= 8;
		align_size -= offset;
	}
	d_block* all_heap = (void*)heap + sizeof(d_heap);
	all_heap->size = align_size;
	printf("heap.c: new heap size is %ld\n", all_heap->size);
	all_heap->free = 1;
	all_heap->last = 1;

	// acum, sa il bagam in bin-ul corespunzator (daca incape intr-un bin)
	int bin_index = get_bin_type(all_heap->size);
	printf("heap.c: placing in %d bin\n", bin_index);
	if(bin_index >= 0 && !pseudo_bins[bin_index])
	{
		pseudo_bins[bin_index] = all_heap;
		all_heap->next = all_heap->prev = all_heap;
	}
	else if(bin_index >= 0)
	{
		d_block* first_bin = pseudo_bins[bin_index];
		// TODO: maybe sort large bins
		first_bin->prev->next = all_heap;
		all_heap->next = first_bin;
		all_heap->prev = first_bin->prev;
		first_bin->prev = all_heap;
	}

	return heap;
}
