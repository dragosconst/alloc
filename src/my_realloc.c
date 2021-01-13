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
	//printf("realloc started\n");
	newsize = aligned_size(newsize);
	if(!ptr) // realloc pe NULL e malloc
	{
		return my_alloc(newsize);
	}

	d_block* block = (d_block*)((char*)ptr - sizeof(d_block));
	pthread_mutex_lock(&global_mutex);
	if(!is_valid_addr(block))
	{
		pthread_mutex_unlock(&global_mutex);
		return NULL;
	}
	if(newsize <= block->size) // pe cazul in care vrem sa shrinkuim blocul
	{
		if(block->free && get_bin_type(block->size) >= 0)
			remove_block_from_bin(block);
		block = split_block(newsize, block);
		show_all_bins();
		show_all_heaps();
		pthread_mutex_unlock(&global_mutex);
		return (block + 1);
	}
	else // trebuie sa cautam undeva
	{
		// mai intai ma uit daca blocul dupa el e liber si destul de mare
		size_t extra_size = newsize - block->size;
		if(!block->last)
		{
			d_block* next_block = (d_block*)((char*)block + sizeof(d_block) + block->size);
			if(is_valid_addr(next_block) && next_block->free)
			{
				/*
				Pentru ca inghit si sizeof(d_block) cand iau urm block, ma intereseaza daca are size in plus
				tinand cont si de dimensiunea metadatelor.
				*/
				if(next_block->size + sizeof(d_block) >= extra_size)
				{
					if(get_bin_type(next_block->size) >= 0) // stim deja ca next_block e free
						remove_block_from_bin(next_block);
					if(sizeof(d_block) + 8 >= extra_size) // daca metadatele blocului sunt destule pt realloc, nu-mi ramane decat sa fac split cu cea mai mica valoarea posibila
						next_block = split_block(8, next_block);
					else
						next_block = split_block(extra_size - sizeof(d_block), next_block);
					block->size += next_block->size + sizeof(d_block);
					if(next_block->last)// daca in urma splitului nu s a intamplat nimic cu next block
						block->last = 1;

					pthread_mutex_unlock(&global_mutex);
					return (block + 1);
				}
			}
		}
		// trebuie cautat altundeva spatiu
		size_t copy_for = block->size;
		char* data = (char*)(block + 1);
		char data_cpy[block->size];
		memcpy(data_cpy, data, copy_for);
		pthread_mutex_unlock(&global_mutex);
		my_free(data);

		char* new_add = my_alloc(newsize);
		d_block* to_move = (d_block*)((char*)new_add - sizeof(d_block));
		pthread_mutex_lock(&global_mutex);
		memcpy(new_add, data_cpy, copy_for);
		pthread_mutex_unlock(&global_mutex);
		return (to_move + 1);
	}
}

