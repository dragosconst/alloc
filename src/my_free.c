/*
	Codul functiei free se regaseste aici.
*/

#include "my_alloc.h"

void
my_free(void* ptr)
{
	pthread_mutex_lock(&global_mutex);
	printf("im freeing %p\n", (char*)ptr - sizeof(d_block));
	show_all_bins();
	show_all_heaps();
	if(!ptr || !heap_top)
	{
		printf("ptr is %p and heap_top is %p\n", ptr, heap_top);
		pthread_mutex_unlock(&global_mutex);
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
		pthread_mutex_unlock(&global_mutex);
		return;
	}
	printf("passed validation\n");
	printf("some data about block: add %p size %zd free %d last %d\n", block, block->size, block->free, block->last);
	// block urile imense trebuie date inapoi la sistem
	if(block->last && block->size > VBIG_BLOCK_SIZE)
	{
		free_heap_to_os(block);
		show_all_heaps();
		pthread_mutex_unlock(&global_mutex);
		return;
	}
	printf("free.c: before merging, size is %zd\n", block->size);
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
	printf("free.c: got past merges, size is %zd\n", block->size);
	if(block->last && block->size > VBIG_BLOCK_SIZE)
	{	// 3 * pagesize nu e tocmai un nr mare, dar l am ales arbitrar ca sa pot testa usor daca elibereaza catre OS
		printf("current heap size is %zd\n", get_heap_of_block(block)->all_size);
		free_some_to_os(block);
		printf("free.c: trimming block, new heap size is %zd\n", get_heap_of_block(block)->all_size);
	}

	d_heap* my_heap = get_heap_of_block(block);
	if(my_heap->all_size == block->size)
		free_heaps++;
	if(free_heaps > 2) //2 e ales arbitrar
	{
		show_all_heaps();
		scan_and_kill_heaps(my_heap); // omor heap-urile in plus
	}

	printf("free.c: searching for bins\n");
	// insert free block in bin
	insert_block_in_bin(block);
	show_all_bins();
	show_all_heaps();
	pthread_mutex_unlock(&global_mutex);
	// no double free protection, in standard am vazut ca nu cere
}

