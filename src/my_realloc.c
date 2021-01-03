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
	/*
		Pentru MT: vreau ca block-ul trimis sa fie protejat de
		mutex pana cand ii gasesc spatiu sau ii dau free.
		De asemenea, am nevoie sa evit lock-urile pe malloc si
		free (pt ca oricum realloc in sine e atomic), deci fol
		flag-ul MALLOC_ATOMIC ca sa ma asigur ca free si malloc
		nu dau aiurea lock la un mutex deja lock-uit de thread.
	*/
	pthread_mutex_lock(&global_mutex);

	newsize = aligned_size(newsize);
	if(!ptr) // realloc pe NULL e malloc
	{
		pthread_mutex_unlock(&global_mutex);
		return my_alloc(newsize);
	}

	d_block* block = (d_block*)((char*)ptr - sizeof(d_block));
	if(!is_valid_addr(block))
	{
		printf("realloc pe pointer invalid!\n");
		pthread_mutex_unlock(&global_mutex);
		return NULL;
	}
	if(newsize <= block->size) // pe cazul in care vrem sa shrinkuim blocul
	{
		printf("realloc.c: block has size %ld\n", block->size);
		if(block->free && get_bin_type(block->size) > 0)
			remove_block_from_bin(block);
		block = split_block(newsize, block);
		printf("realloc.c: block after split has %zd\n", block->size);
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
					if(get_bin_type(next_block->size) > 0) // stim deja ca next_block e free
						remove_block_from_bin(next_block);
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
		printf("getting real close to finishing realloc\n");
		size_t copy_for = block->size;
		char* data = (char*)(block + 1);
		char data_cpy[block->size];
		memcpy(data_cpy, data, copy_for);
		printf("entering free\n");
		_unlock_free(data);

		char* new_add = _unlock_alloc(newsize);
		d_block* to_move = (d_block*)((char*)new_add - sizeof(d_block));
		printf("realloc almost done\n");
		memcpy(new_add, data_cpy, copy_for);
		pthread_mutex_unlock(&global_mutex);
		return (to_move + 1);
	}
}
