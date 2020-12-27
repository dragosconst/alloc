/*
	Functia de realocare seamana cu cea de alocare, in mod similar cauta heap-uri libere destul de
	incapatoare. Daca nu gaseste, isi face un heap nou. Asta in cazul in care vrem un size mai mare,
	daca size-ul cerut e mai mic, atunci pur si simplu adaugam
*/

#include "my_alloc.h"

void*
my_realloc(void* ptr, size_t newsize)
{
	if(!ptr) // realloc pe NULL e malloc
		return my_alloc(size);

	d_block* block = ptr - sizeof(d_block);
	if(!is_valid_addr(block))
	{
		printf("realloc pe pointer invalid!\n");
		return NULL;
	}
	if(newsize <= block->size) // pe cazul in care vrem sa shrinkuim blocul
	{
		block = split_block(newsize, block);
		return (block + 1);
	}
	else // trebuie sa cautam undeva
	{
		// mai intai ma uit daca blocul liber dupa el e destul de mare
		size_t remainder_size = newsize - block->size;
		d_block* next_block = block->next;
		if(next_block->size + sizeof(d_block) >= remainder_size)
		{	// adica, daca putem lua blocul urmatorul (care e liber) si sa-l lipim de asta
			block = merge_blocks(block, next_block);
			block = split_block(newsize, block);
			return (block + 1);
		}
		else
		{
			void* new_add = my_alloc(newsize);
			d_block* to_move = new_add - sizeof(d_block);
			memcpy(new_add, block + 1, block->size);

			block->free = 1;
			if(block->prev && block->prev->free)
				block = merge_blocks(block->prev, block);
			if(block->next && block->next->free)
				block = merge_blocks(block, block->next);
			return (to_move + 1);
		}
	}
}
