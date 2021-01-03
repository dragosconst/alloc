/*
	Codul functiei free se regaseste aici.
*/

#include "my_alloc.h"

void
my_free(void* ptr)
{
	printf("im freeing %p\n", (char*)ptr - sizeof(d_block));
	if(pthread_mutex_lock(&global_mutex))
		while(1); // nimeni nu tre sa se atinga de block pana free nu e gata
	if(!ptr || !heap_top)
	{
		printf("ptr is %p and heap_top is %p\n", ptr, heap_top);
		if(pthread_mutex_unlock(&global_mutex)) while(1);
		return;
	}
	//show_all_heaps();
	d_block* block = (d_block*)((char*)ptr - sizeof(d_block));
	printf("free.c: going into add validation\n");
	if(!is_valid_addr(block))
	{
		printf("free pe valori non-freeable %p\n", block);
		d_heap* heap  = get_heap_of_block(block);
		printf("heap e %p\n", heap);
		printf("size %ld\n", block->size);
		//while(1);
		if(pthread_mutex_unlock(&global_mutex)) while(1);
		return;
	}
	//printf("passed validation\n");
	// block urile imense trebuie date inapoi la sistem
	if(block->last && block->size > VBIG_BLOCK_SIZE)
	{
		free_heap_to_os(block);
		show_all_heaps();
		if(pthread_mutex_unlock(&global_mutex)) while(1);
		return;
	}
	//printf("free.c: before merging, size is %zd\n", block->size);
	//show_all_heaps();
	// merging
	d_block* prev_block = get_prev_block(block);
	d_block* next_block = get_next_block(block);
	printf("prev is %p next is %p\n", prev_block, next_block);
	if(prev_block) // blocul de dinainte era free
	{
		//printf("prev merge\n");
		block = merge_blocks(prev_block, block);
		block->free = 0; // pt merge-ul urmator, daca il face
		show_all_bins();
	}
	if(next_block)
	{
		//printf("next merge\n");
		block = merge_blocks(block, next_block);
	}
	block->free = 1;
	show_all_bins();
	show_all_heaps();
	//printf("free.c: got past merges, size is %zd\n", block->size);
	if(block->last && block->size > VBIG_BLOCK_SIZE)
	{	// 3 * pagesize nu e tocmai un nr mare, dar l am ales arbitrar ca sa pot testa usor daca elibereaza catre OS
		printf("current heap size is %zd\n", get_heap_of_block(block)->all_size);
		free_some_to_os(block);
		printf("free.c: trimming block, new heap size is %zd\n", get_heap_of_block(block)->all_size);
	}

	//printf("free.c: searching for bins\n");
	// insert free block in bin
	insert_block_in_bin(block);
	show_all_bins();
	show_all_heaps();
	if(pthread_mutex_unlock(&global_mutex)) while(1);
	// no double free protection, in standard am vazut ca nu cere
}
