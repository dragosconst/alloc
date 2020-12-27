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
		printf("free pe valori non-freeable\n");
		return;
	}
	block->free = 1;
	d_heap* heap = get_heap_of_block(block);
	if(!heap)
	{
		printf("unspecified free error\n");
		return;
	}
	heap->free_size += block->size;
	// merging
	if(block->prev && block->prev->free) // blocul de dinainte era free
	{
		block = merge_blocks(block->prev, block);
		heap->free_size += sizeof(d_block); // un block de metadate poate fi suprascris dupa merge
	}
	if(block->next && block->next->free)
	{
		block = merge_blocks(block, block->next);
		heap->free_size += sizeof(d_block);
	}
	// no double free protection, in standard am vazut ca nu cere
}
