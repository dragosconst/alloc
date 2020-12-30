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
	newsize = aligned_size(newsize);
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
		size_t extra_size = newsize - block->size;
		if(!block->last)
		{
			d_block* next_block = (d_block*)((void*)block + block->size);
			if(next_block->free)
			{
				if(next_block->size >= extra_size)
				{
					next_block = split_block(extra_size, next_block);
					block->size += next_block->size + sizeof(d_block);
					if(next_block->last)// daca in urma splitului nu s a intamplat nimic cu next block
						block->last = 1;
					return (block + 1);
				}
			}
		}
		// trebuie cautat altundeva spatiu
		size_t copy_for = block->size;
		void* data = block + 1; // atentie la multi-threading, nu vrem sa ne corupa cineva datele de aici
		my_free(data);

		void* new_add = my_alloc(newsize);
		d_block* to_move = new_add - sizeof(d_block);
		memcpy(new_add, data, copy_for);

		return (to_move + 1);
	}
}
