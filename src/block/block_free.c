/*
	Aici sunt functii pentru eliberarea blocurilor de memorie (sau functii care ajuta la eliberarea blocurilor).
*/

#include "my_alloc.h"

int
is_valid_addr(void* addr) // verific daca exista vreun bloc cu adresa data de user
{
	d_heap* heap = heap_top;
	d_heap* prev = NULL;
	while(heap != prev)
	{
		prev = heap; // e o lista circulara
		d_block* block = (d_block*)(heap + 1);
		while(1)
		{
			if(block == addr)
				return 1;
			if(block->last)
				break;
			printf("block_free.c: val in last e %d\n", block->last);
			block = (d_block*)((void*)block + sizeof(d_block) + block->size);
		}
		heap = heap->prev;
	}
	return 0;
}

d_heap*
get_heap_of_block(d_block* block)
{
	d_heap* heap = heap_top;
	d_heap* prev = NULL;
	while(heap != prev)
	{
		prev = heap; // e o lista circulara

		d_block* _block = (d_block*)(heap + 1);
		while(1)
		{
			if(_block == block)
				return heap;
			if(_block->last)
				break;
			printf("_block is %zd size is %zd\n", _block, _block->size);
			_block = (d_block*)((void*)_block + sizeof(d_block) + _block->size);
		}
		heap = heap->prev;
	}
	return NULL;
}

d_block*
get_prev_block(d_block* block)
{
	d_heap* heap = get_heap_of_block(block);
	d_block* traverse = (d_block*)(heap + 1);
	if(traverse == block) return NULL; // nu ne intereseaza daca e primu block
	while((d_block*)((void*)traverse + sizeof(d_block) + traverse->size) != block)
	{
		printf("block_free.c: traverse is %zd block is %zd\n", traverse, block);
		traverse = (d_block*)((void*)traverse + sizeof(d_block) + traverse->size);
	}
	// acum, traverse e blocul fix dinainte
	if(traverse->free)
		return traverse;
	return NULL;
}
d_block*
get_next_block(d_block* block)
{
	if(block->last)
		return NULL;

	d_heap* heap = get_heap_of_block(block);
	d_block* traverse = (d_block*)(heap + 1);
	d_block* old_val = NULL;
	while(old_val != block)
	{
		old_val = traverse;
		traverse = (d_block*)((void*)traverse + sizeof(d_block) + traverse->size);
	}
	// acum, traverse e blocul fix de dupa
	if(traverse->free)
		return traverse;
	return NULL;
}

d_block*
merge_blocks(d_block* bl, d_block* br)
{
	// ordinea argumentelor e importanta, ca sa fie mai usoara implementarea
	// de observat ca e mostenita valoarea de free din bl, evident se presupune ca e free si bl, si br
	printf("block_free.c: merging block of size %zd with block of size %zd\n", bl->size, br->size);
	bl->size += sizeof(d_block) + br->size - BLOCK_OFFSET; // scadem block offset ca sa ramanem garantat cu un size multiplu de 8
	if(bl->size % 8) // logic nu ar trebui sa ajunga pe cazul asta niciodata
		printf("block_free.c: failed size calculation %ld\n", bl->size);

	if(br->last)
		bl->last = 1;

	// br trebuie scos din bin_ul sau, de bl se ocupa ala care a apelat functia
	if(br->next == br) // e singur in bin
	{
		pseudo_bins[get_bin_type(br->size)] = NULL;
		// nu e obligatoriu sa le fac NULL
		br->next = br->prev = NULL;
	}
	else
	{
		br->prev->next = br->next;
		br->next->prev = br->prev;
	}
	return bl;
}
