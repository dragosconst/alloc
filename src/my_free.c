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
		printf("no suitable heap found in free function\n");
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
	// daca e ultimul bloc din heap, il "sterg", adica il scot din lista de blocuri si il transform efectiv in memoria unmapped din heap
	if(block->next == NULL)
	{
		heap->free_end_size += block->size + sizeof(d_block);
		heap->free_size += block->size + sizeof(d_block);
		if(block->prev)
			block->prev->next = NULL;
		block->size = 0; // asta s-ar putea sa fie necesar pt functia search_for_free_block
	}
	// no double free protection, in standard am vazut ca nu cere
}
