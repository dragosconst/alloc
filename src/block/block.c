/*
	Aici o sa fie implementate functiile principale legate de blocuri. Blocurile sunt
	unitatea minima din mallocul meu, ele sunt subdiviziuni pe heap-uri. Desi blocurile
	nu au un atribut explicit type, tipul se deduce imediat din dimensiunea lor. Evident,
	un bloc nu poate fi stocat decat pe un heap de tipul sau.

	Daca nu exista niciun bloc liber suficient de mare, dar avem la capatul unui heap
	destul spatiu liber, atunci blocul e adaugat in capat cu functia append_block.
	Denumirea functiei poate fi un pic misleading, eu nu adaug nimic concret la heap,
	doar marchez o sectiune de la finalul sau ca fiind rezervata acestui bloc.
*/
#include "my_alloc.h"

d_block*
append_block(size_t size, d_heap* heap)
{
	d_block* traverse = (d_block*)(heap + 1);
	size_t total_unusable_space = 0; // ii zic unusable space pt ca nu e neaparat spatiu folosit, dar daca e gol, e sigur prea mic pt size-ul pe care il cerem
	while(traverse->next)
	{
		traverse = traverse->next;
		total_unusable_space += traverse->size + sizeof(d_block);
	}

	// acum in traverse e ultimul bloc alocat din heap
	if(traverse->size == 0) // daca avem un heap gol, deoarece mmap pune zero pe tot heap-ul in afara de metadate, si pt ca un malloc de 0 returneaza NULL, asa stim garantat ca daca size e 0 inseamna ca toata structura e 0
	{
		d_block* newblock = (d_block*)(heap + 1);
		//printf("am lipit la pozitia %p\n", newblock);
		newblock->prev = NULL;
		newblock->next = NULL;
		newblock->size = size;
		newblock->free = 0;
		// modify heap
		heap->free_end_size -= sizeof(d_block) + size;
		return newblock;
	}
	else
	{
		total_unusable_space += traverse->size; // pt ca while-ul se opreste la ultimu block, nu-i adauga niciodata size-ul in while
		// trebuie verificat manual daca avem spatiu in coada la final
		if(heap->all_size - total_unusable_space - sizeof(d_block) < size)
			return NULL;
		d_block* newblock = (d_block*)((void*)traverse + sizeof(d_block) + traverse->size);
		newblock->prev = traverse;
		newblock->next = NULL;
		traverse->next = newblock;
		newblock->size = size;
		newblock->free = 0;
		// modify heap
		heap->free_end_size -= sizeof(d_block) + size;
		return newblock;
	}
}

d_block*
search_for_free_block(size_t size, d_heap* heap)
{
	// se presupune ca heap-ul actual are destul spatiu ca sa salvam blocul pe el
	// desigur, asta nu inseamna neaparat ca exista un bloc free destul de mare, poate
	// avem la final spatiul (sau poate heap-ul trebuie defragmentat)
	d_block* traverse = (d_block*)(heap + 1);
	while(traverse)
	{
		if(traverse->free && traverse->size >= size)
		{
			if(traverse->size > size)
			{
				size_t old_size = traverse->size;
				traverse = split_block(size, traverse);
				if(old_size == heap->biggest_fblock)
					heap->biggest_fblock = find_biggest_free_block(heap);
			}
			traverse->free = 0;
			return traverse;
		}
		traverse = traverse->next;
	}
	return NULL;// nu avem niciun free block destul de mare
}

d_block*
split_block(size_t size, d_block* block) // nu modifica campul free din block-ul initial
{
	if(block->size - size <= sizeof(d_block)) // daca spatiul in plus e prea mic sa mai bagam metadate
	{
		return block; // nu are rost sa fac split, ca as corupe segmentul de date
	}
	d_block* newblock = (d_block*)((void*)block + sizeof(d_block) + size);
	newblock->size = block->size - size - sizeof(d_block);
	newblock->free = 1;
	newblock->prev = block;
	newblock->next = block->next;
	// trebuie scos din free size ul heap-ului spatiul ocupat de metadatele blocului nou
	block->size = size;
	block->next = newblock;
	return block;
}
