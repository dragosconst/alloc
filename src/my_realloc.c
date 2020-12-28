/*
	Functia de realocare seamana cu cea de alocare, in mod similar cauta heap-uri libere destul de
	incapatoare. Daca nu gaseste, isi face un heap nou.Este adaugata functionalitate ca se uita, in
	plus, si la blocul\spatiul liber de dupa blocul actual, sa vada daca poate sa-si ia si de acolo
	spatiu. Asta in cazul in care vrem un size mai mare, daca size-ul cerut e mai mic, atunci pur si
	simplu fac un split. Daca split-ul nu e posibil, nu	se face nicio modificare concreta.
*/

#include "my_alloc.h"

void*
my_realloc(void* ptr, size_t newsize)
{
	if(!ptr) // realloc pe NULL e malloc
		return my_alloc(newsize);

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
		// mai intai ma uit daca blocul dupa el e liber si destul de mare
		size_t remainder_size = newsize - block->size;
		d_block* next_block = block->next;
		d_heap* heap = get_heap_of_block(block);
		if(next_block && next_block->free && next_block->size + sizeof(d_block) >= remainder_size)
		{	// adica, daca putem lua blocul urmator si sa-l lipim de asta
			block = merge_blocks(block, next_block);
			block = split_block(newsize, block);
			return (block + 1);
		}
		else if(next_block == NULL && heap->free_end_size >= remainder_size)
		{	// daca avem in coada heap-ului spatiu nealocat destul
			block->size = newsize; // il luam pur si simplu
			heap->free_end_size -= remainder_size;
			return (block + 1);
		}
		else
		{	// trebuie cautat altundeva spatiu
			size_t copy_for = block->size;
			void* data = block + 1; // atentie la multi-threading, nu vrem sa ne corupa cineva datele de aici
			my_free(data);

			void* new_add = my_alloc(newsize);
			d_block* to_move = new_add - sizeof(d_block);
			memcpy(new_add, data, copy_for);

			return (to_move + 1);
		}
	}
}
