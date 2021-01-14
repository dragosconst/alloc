/*
	Codul functiei free se regaseste aici.
*/

#include "my_alloc.h"

void
my_free(void* ptr)
{
	pthread_mutex_lock(&global_mutex);
	//printf("im freeing %p\n", (char*)ptr - sizeof(d_block));
	show_all_bins();
	show_all_heaps();
	if(!ptr || !heap_top)
	{
		pthread_mutex_unlock(&global_mutex);
		return;
	}
	//show_all_heaps();
	d_block* block = (d_block*)((char*)ptr - sizeof(d_block));
	//printf("free.c: going into add validation\n");
	d_heap* my_heap;
	if(!(my_heap = get_heap_of_block(block)))
	{
		pthread_mutex_unlock(&global_mutex);
		return;
	}
	//printf("passed validation\n");
	// block urile imense trebuie date inapoi la sistem
	if(block->last && block->size > VBIG_BLOCK_SIZE)
	{
		free_heap_to_os(block);
		show_all_heaps();
		pthread_mutex_unlock(&global_mutex);
		return;
	}
	//printf("free.c: before merging, size is %zd\n", block->size);
	// merging
	d_block* prev_block = get_prev_block(block);
	d_block* next_block = get_next_block(block);
	//printf("prev is %p next is %p\n", prev_block, next_block);
	if(prev_block) // blocul de dinainte era free
	{
		//printf("prev merge\n");
		block = merge_blocks(prev_block, block);
		block->free = 0; // pt merge-ul urmator, daca il face
	}
	if(next_block)
	{
		//printf("next merge\n");
		block = merge_blocks(block, next_block);
	}
	block->free = 1;
	//printf("free.c: got past merges, size is %zd\n", block->size);
	if(block->last && block->size > VBIG_BLOCK_SIZE)
	{	// ideea e ca daca putem, il transformam intr-un block care poate fi bagat intr-un bin
		free_some_to_os(block);
	}

	if(my_heap->all_size == block->size)
		free_heaps++;
	if(free_heaps > 2) //2 e ales arbitrar
	{
		show_all_heaps();
		scan_and_kill_heaps(my_heap); // omor heap-urile in plus
	}

	//printf("free.c: searching for bins\n");
	// insert free block in bin
	insert_block_in_bin(block);
	pthread_mutex_unlock(&global_mutex);
	// no double free protection, in standard am vazut ca nu cere
}

