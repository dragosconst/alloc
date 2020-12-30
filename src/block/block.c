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
find_best_fit(size_t size, d_block* bin_start)
{
	d_block* old = bin_start;
	d_block* best;
	size_t closest_fit = size;
	do
	{
		if(abs(bin_start->size - size) < closest_fit)
		{
			closest_fit = abs(bin_start->size - size);
			best = bin_start;
		}
		bin_start = bin_start->next;
	}while(old != bin_start);

	return best;
}

d_block*
search_for_free_block(size_t size)
{
	// ne uitam daca e prea mare sa aiba ce cauta in bins first of all
	if(size > VBIG_BLOCK_SIZE * 2) // adica nu e in niciun tip de bin
		return NULL;

	size = aligned_size(size); // alinierea ma ajuta mai ales la implementarea binsurilor
	printf("block.c: asking for bin...\n");
	int bin_index = get_bin_type(size);
	d_block* bin_block = pseudo_bins[bin_index];
	if(!bin_block)
	{
		// poate totusi putem lua un bin mai mare si sa-l splituim
		bin_index = get_closest_bin_type(size);
		printf("block.c: asking for %zd size, index is %d\n", size, bin_index);	
		if(bin_index < 0) // nu exista literalmente niciun bin pe care-l putem lua
			return NULL;
		bin_block = pseudo_bins[bin_index];
		printf("block.c: if exec halts here, possible bug in split_block\n");
		bin_block = split_block(size, bin_block);
		bin_block->free = 0;

		if(bin_block->prev != bin_block)
		{
			bin_block->prev->next = bin_block->next; // scot din lista dublu inlantuita
			bin_block->next->prev = bin_block->prev;
			pseudo_bins[bin_index] = bin_block->next;
		}
		else pseudo_bins[bin_index] = NULL;
		printf("block.c: request granted\n");
		return bin_block;
	}
	bin_block->free = 0;
	if(size >= BIG_BLOCK_SIZE / 2)
	{	// la large bins trebuie facut si un split
		bin_block = find_best_fit(size, bin_block);
		bin_block = split_block(size, bin_block);
	}

	if(bin_block->prev != bin_block)
	{
		bin_block->prev->next = bin_block->next; // scot din lista dublu inlantuita
		bin_block->next->prev = bin_block->prev;
		if(bin_block == pseudo_bins[bin_index]) // la large bins e posibil sa fie altundeva block ul gasit
			 pseudo_bins[bin_index] = bin_block->next;
	}
	else pseudo_bins[bin_index] = NULL;
	return bin_block;
}

d_block*
split_block(size_t size, d_block* block) // nu modifica campul free din block-ul initial
{
	// size has to be aligned
	if(block->size - size < aligned_size(sizeof(d_block) + 8)) // daca spatiul in plus e prea mic sa mai bagam metadate
	{
		return block; // nu are rost sa fac split, ca as corupe segmentul de date
	}
	d_block* newblock = (d_block*)((void*)block + aligned_size(sizeof(d_block) + size));
	printf("block.c: if exec halts here, newblock is bugged\n");
	newblock->size = aligned_size(block->size - size - sizeof(d_block));
	printf("bruh\n");
	newblock->free = 1;
	if(block->last)
	{	// split pe ultimu bloc creeaza un nou ultim bloc
		block->last = 0;
		newblock->last = 1;
	}
	int bin_index = get_bin_type(newblock->size);
	if(pseudo_bins[bin_index])
	{	// pastrez structura de lista dublu inlantuita circulara si adaug bloc-ul la final
		pseudo_bins[bin_index]->prev->next = newblock;
		newblock->next = pseudo_bins[bin_index];
		newblock->prev = pseudo_bins[bin_index]->prev;
		pseudo_bins[bin_index]->prev = newblock;
	}
	else
	{
		pseudo_bins[bin_index] = newblock;
		newblock->prev = newblock->next = newblock;
	}
	block->size = size;
	// size vine gata aliniat
	return block;
}
