/*
	Codul functiei free se regaseste aici.
*/

#include "my_alloc.h"

void
my_free(void* ptr)
{
	//printf("im freeing\n");
	if(MALLOC_ATOMIC)
		pthread_mutex_lock(&global_mutex); // nimeni nu tre sa se atinga de block pana free nu e gata
	if(!ptr || !heap_top)
	{
		if(MALLOC_ATOMIC)
			pthread_mutex_unlock(&global_mutex);
		return;
	}
	//show_all_heaps();
	d_block* block = (d_block*)((char*)ptr - sizeof(d_block));
	//printf("free.c: going into add validation\n");
	if(!is_valid_addr(block))
	{
		printf("free pe valori non-freeable %p\n", block);
		d_heap* heap  = get_heap_of_block(block);
		printf("heap e %p\n", heap);
		printf("size %ld\n", block->size);
		//while(1);
		if(MALLOC_ATOMIC)
			pthread_mutex_unlock(&global_mutex);
		return;
	}
	//printf("passed validation\n");
	block->free = 1;
	// block urile imense trebuie date inapoi la sistem
	if(block->size > VBIG_BLOCK_SIZE * 2)
	{
		free_heap_to_os(block);
		if(MALLOC_ATOMIC)
			pthread_mutex_unlock(&global_mutex);
		return;
	}
	printf("free.c: before merging, size is %zd\n", block->size);
	//show_all_heaps();
	// merging
	d_block* prev_block = get_prev_block(block);
	if(prev_block && !is_valid_addr(prev_block))
		printf("ceva e busit grav\n");
	d_block* next_block = get_next_block(block);
	if(next_block && !is_valid_addr(next_block))
		printf("ceva chiar e busit grav next\n");
	if(prev_block) // blocul de dinainte era free
	{
		printf("prev merge\n");
		block = merge_blocks(prev_block, block);
	}
	if(next_block)
	{
		printf("next merge\n");
		block = merge_blocks(block, next_block);
	}
	show_all_heaps();
	//printf("free.c: got past merges, size is %zd\n", block->size);
	if(block->last && block->size > VBIG_BLOCK_SIZE * 2)
	{	// 3 * pagesize nu e tocmai un nr mare, dar l am ales arbitrar ca sa pot testa usor daca elibereaza catre OS
		free_some_to_os(block);
		//printf("free.c: trimming block, new size is %zd\n", block->size);
	}

	//printf("free.c: searching for bins\n");
	// insert free block in bin
	int bin_index = get_bin_type(block->size);
	// TODO: sorting pe large bins
	if(!pseudo_bins[bin_index])
	{
		pseudo_bins[bin_index] = block;
		block->next = block->prev = block;
	}
	else
	{
		d_block* bin_first = pseudo_bins[bin_index];
		bin_first->prev->next = block;
		block->next = bin_first;
		block->prev = bin_first->prev;
		bin_first->prev = block;
	}
	show_all_heaps();
	if(MALLOC_ATOMIC)
		pthread_mutex_unlock(&global_mutex);
	// no double free protection, in standard am vazut ca nu cere
}
