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
		total_unusable_space += traverse->size;
	}

	// acum in traverse e ultimul bloc alocat din heap
	if(traverse == (d_block*)(heap + 1)) // daca avem un heap gol
	{
		d_block* newblock = (d_block*)(heap + 1);
		newblock->prev = NULL;
		newblock->next = NULL;
		newblock->size = size;
		newblock->free = 0;
		// modify heap
		heap->free_end_size -= sizeof(d_block) + size;
		heap->free_size -= sizeof(d_block) + size;
		return (newblock + 1);
	}
	else
	{
		// trebuie verificat manual daca avem spatiu in coada la final
		if(heap->all_size - total_unusable_space - sizeof(d_block) < size)
			return NULL;
		d_block* newblock = (d_block*)((void*)traverse + sizeof(d_block) + traverse->size));
		newblock->prev = traverse;
		newblock->next = NULL;
		traverse->next = newblock;
		newblock->size = size;
		newblock->free = 0;
		// modify heap
		heap->free_end_size -= sizeof(d_block) + size;
		heap->free_size -= sizeof(d_block) + size;
		return (newblock + 1);
	}
}

d_block*
search_for_free_block(size_t size, d_heap* heap)
{
	// se presupune ca heap-ul actual are destul spatiu ca sa salvam blocul pe el
	// desigur, asta nu inseamna neaparat ca exista un bloc free destul de mare, poate
	// avem la final spatiul (sau poate heap-ul trebuie defragmentat)
	d_block* traverse = (d_block*)(heap + 1);
	while(traverse->next)
	{
		if(traverse->free && traverse->size >= size)
		{
			if(traverse->size > size)
			{
				traverse = split_block(size, traverse);
			}
			traverse->free = 0;
			// modify heap
			heap->free_size -= size;
			return traverse;
		}
		traverse = traverse->next;
	}
	return NULL;// nu avem niciun free block destul de mare
}

d_block*
split_block(size_t size, d_block* block)
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
	heap->free_size -= sizeof(d_block);
	block->size = size;
	block->next = newblock;
	return block;
}
