

#include "my_alloc.h"

d_block*
_unlock_search_for_free_block(size_t size)
{
	// ne uitam daca e prea mare sa aiba ce cauta in bins first of all
	if(size > VBIG_BLOCK_SIZE * 2) // adica nu e in niciun tip de bin
		return NULL;

	size = aligned_size(size); // alinierea ma ajuta mai ales la implementarea binsurilor
	int bin_index = get_bin_type(size);
	d_block* bin_block = pseudo_bins[bin_index];
	if(bin_block && size >= BIG_BLOCK_SIZE)
	{	// la large bins trebuie facut si un split si un search mai comprehensiv
		bin_block = find_best_fit(size, bin_block);
		if(bin_block)//;	// e posibil ca find best fit sa dea fail
		{
			remove_block_from_bin(bin_block);
			if(get_heap_of_block(bin_block)->all_size == bin_block->size)
				free_heaps--;
			bin_block->free = 0;
			/*SPLIT !!!*/ bin_block = split_block(size, bin_block);
			return bin_block;
		}
	}
	if(!bin_block)
	{
		// poate totusi putem lua un bin mai mare si sa-l splituim
		bin_index = get_closest_bin_type(size);
		if(bin_index < 0) // nu exista literalmente niciun bin pe care-l putem lua
			return NULL;
		if(bin_index <= 63)
			bin_block = pseudo_bins[bin_index];
		else
			bin_block = find_best_fit(size, pseudo_bins[bin_index]);

		if(get_heap_of_block(bin_block)->all_size == bin_block->size)
			free_heaps--;

		remove_block_from_bin(bin_block);
		/*SPLIT !!!*/ bin_block = split_block(size, bin_block);
		bin_block->free = 0;
		return bin_block;
	}


	if(get_heap_of_block(bin_block)->all_size == bin_block->size)
		free_heaps--;
	bin_block->free = 0;
	remove_block_from_bin(bin_block);
	return bin_block;
}
