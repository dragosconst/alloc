/*
	Aici sunt functii pentru eliberarea blocurilor de memorie (sau functii care ajuta la eliberarea blocurilor).
*/

int
is_valid_addr(void* addr) // verific daca exista vreun bloc cu adresa data de user
{
	d_heap* heap = heap_top;
	while(heap)
	{
		d_block* block = (d_block*)(heap + 1);
		while(block)
		{
			if(block == addr)
				return 1;
			block = block->next;
		}
		heap = heap->prev;
	}
	return 0;
}

d_heap*
get_heap_of_block(d_block* block)
{
	d_heap* heap = heap_top;
	while(heap)
	{
		d_block* _block = (d_block*)(heap + 1);
		while(block)
		{
			if(_block == block)
				return heap;
			block = block->next;
		}
		heap = heap->prev;
	}
	return NULL;
}

d_block*
merge_blocks(d_block* bl, d_block* br)
{
	// ordinea argumentelor e importanta, ca sa fie mai usoara implementarea
	bl->size += sizeof(d_block) + br->size;
	bl->next = br->next;
	br->next->prev = bl;
	return bl;
}
