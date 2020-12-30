/*
	Codul functiei free se regaseste aici.
*/

#include "my_alloc.h"

void
my_free(void* ptr)
{
	if(!ptr || !heap_top)
		return;

	d_block* block = (void*)ptr - sizeof(d_block);
	if(!is_valid_addr(block))
	{
		printf("free pe valori non-freeable %p\n", block);
		return;
	}
	block->free = 1;
	// block urile imense trebuie date inapoi la sistem
	if(block->size > VBIG_BLOCK_SIZE * 2)
	{
		free_heap_to_os(block);
		return;
	}
	// merging
	d_block* prev_block = get_prev_block(block);
	d_block* next_block = get_next_block(block);
	if(prev_block) // blocul de dinainte era free
	{
		block = merge_blocks(prev_block, block);
	}
	if(next_block)
	{
		block = merge_blocks(block, next_block);
	}

	if(block->last && block->size > 3 * getpagesize())
	{	// 3 * pagesize nu e tocmai un nr mare, dar l am ales arbitrar ca sa pot testa usor daca elibereaza catre OS
		free_some_to_os(block);
		block->size -= 3 * getpagesize();
	}

	// insert free block in bin
	int bin_index = get_bin_type(block->size);
	// TODO: sorting pe large bins
	if(!pseudo_bins[bin_index])
		pseudo_bins[bin_index] = block;
	d_block* bin_first = pseudo_bins[bin_index];
	bin_first->prev->next = block;
	block->next = bin_first;
	block->prev = bin_first->prev;
	bin_first->prev = block;
	// no double free protection, in standard am vazut ca nu cere
}
