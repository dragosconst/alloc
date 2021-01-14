/*
	Aici sunt functii pentru eliberarea blocurilor de memorie (sau functii care ajuta la eliberarea blocurilor).
*/

#include "my_alloc.h"

int
is_valid_addr(void* addr) // verific daca exista vreun bloc cu adresa data de user
{
	d_heap* heap = heap_top;
	d_heap* prev = heap_top;
	do
	{
		d_block* block = (d_block*)(heap + 1);
		while(1)
		{
			if(block == addr)
				return 1;
			if(block->last)
				break;
			block = (d_block*)((char*)block + sizeof(d_block) + block->size);
		}
		heap = heap->prev;
	}while(heap != prev);
	return 0;
}

d_heap*
get_heap_of_block(d_block* block)
{
	d_heap* heap = heap_top;
	d_heap* prev = heap_top;
	do
	{
		d_block* _block = (d_block*)(heap + 1);
		while(1)
		{
			if(_block == block)
				return heap;
			if(_block->last)
				break;
			_block = (d_block*)((char*)_block + sizeof(d_block) + _block->size);
		}
		heap = heap->prev;
	}while(heap != prev);
	return NULL;
}

d_block*
get_prev_block(d_block* block)
{
	d_heap* heap = get_heap_of_block(block);
	d_block* traverse = (d_block*)(heap + 1);
	if(traverse == block) return NULL; // nu ne intereseaza daca e primu block
	while((d_block*)((char*)traverse + sizeof(d_block) + traverse->size) != block)
	{
		traverse = (d_block*)((char*)traverse + sizeof(d_block) + traverse->size);
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
	while(old_val != block)// && (!old_val || (old_val && !old_val->last)))
	{
		old_val = traverse;
		if(!old_val->last)
			traverse = (d_block*)((char*)traverse + sizeof(d_block) + traverse->size);
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
	//printf("block_free.c: merging block of size %zd with block of size %zd\n", bl->size, br->size);
	//printf("bl %p are size %zd\n", bl, sizeof(d_block));
	//printf("br %p are size %zd\n", br, sizeof(br));
	// de observat ca e mostenita valoarea de free din bl
	if(bl->free)
		remove_block_from_bin(bl);
	bl->size += sizeof(d_block) + br->size;

	if(br->last)
		bl->last = 1;

	// br trebuie scos din bin_ul sau, de bl se ocupa ala care a apelat functia
	if(br->free)
		remove_block_from_bin(br);
	return bl;
}
