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


/*
	Functie apelata doar pe large bins.
*/
d_block*
find_best_fit(size_t size, d_block* bin_start)
{
	d_block* old = bin_start;
	//printf("closest fit is %zd differance is %zd\n", closest_fit,  (ssize_t)bin_start->size - (ssize_t)size);
	printf("stuck in find_best_fit\n");
	//printf("old is %p old->next %p old->prev %p\n", old, old->next, old->prev);
	do
	{	// fiind garantata ordinea crescatoarea, primul block cu size suficient e si cel mai bun
		if(bin_start->size >= size)
			return bin_start;
		bin_start = bin_start->next;
		printf("bin start is %p, but old is %p\n", bin_start, old);
	}while(old != bin_start);
	// daca am ajuns aici, inseamna ca nu exista niciun block in bin suficient de mare
	return NULL;
}

d_block*
search_for_free_block(size_t size)
{
	// ne uitam daca e prea mare sa aiba ce cauta in bins first of all
	if(size > VBIG_BLOCK_SIZE * 2) // adica nu e in niciun tip de bin
		return NULL;

	size = aligned_size(size); // alinierea ma ajuta mai ales la implementarea binsurilor
	int bin_index = get_bin_type(size);
	d_block* bin_block = pseudo_bins[bin_index];
	printf("block.c: asking for bin %d...\n", bin_index);
	printf("ce naiba %zd %d\n", size, size >= BIG_BLOCK_SIZE);
	if(bin_block && size >= BIG_BLOCK_SIZE)
	{	// la large bins trebuie facut si un split si un search mai comprehensiv
		bin_block = find_best_fit(size, bin_block);
		printf("im splittin stuff\n");
		if(bin_block)//;	// e posibil ca find best fit sa dea fail
		{
			remove_block_from_bin(bin_block);
			bin_block->free = 0;
			/*SPLIT !!!*/ bin_block = split_block(size, bin_block);
			return bin_block;
		}
		else
			printf("actually nah im not\n");
	}
	if(!bin_block)
	{
		// poate totusi putem lua un bin mai mare si sa-l splituim
		bin_index = get_closest_bin_type(size);
		if(bin_index < 0) // nu exista literalmente niciun bin pe care-l putem lua
			return NULL;
		if(bin_index <= 63)
			bin_block = pseudo_bins[bin_index];
		else
			bin_block = find_best_fit(size, pseudo_bins[bin_index]);
		//printf("block.c: asking for %zd size, bin size is %zd, bin index is %d\n", size, pseudo_bins[bin_index]->size, bin_index);
		remove_block_from_bin(bin_block);
		/*SPLIT !!!*/ bin_block = split_block(size, bin_block);
		bin_block->free = 0;
		//printf("block.c: request granted\n");
		//bin_block->next = bin_block->prev = NULL;
		return bin_block;
	}

	bin_block->free = 0;
	remove_block_from_bin(bin_block);
	//bin_block->next = bin_block->prev = NULL;
	return bin_block;
}

d_block*
split_block(size_t size, d_block* block) // nu modifica campul free din block-ul initial
{
	if(!block)
	{
		printf("cant split null block\n");
		return NULL;
	}
	if(!is_valid_addr(block))
	{
		printf("how did this happen block is %p zise is %zd\n", block, size);
		//return block;
	}
	// size has to be aligned
	//printf("im splitting\n");
	if((ssize_t)block->size - (ssize_t)size < (ssize_t)(sizeof(d_block) + 8)) // daca spatiul in plus e prea mic sa mai bagam metadate
	{
	printf("no split req %zd %zd\n", (ssize_t)block->size - (ssize_t)size, (ssize_t)block->size - (ssize_t)size <  (ssize_t)(sizeof(d_block) + 8));
//		while(1);
		return block; // nu are rost sa fac split, ca as corupe segmentul de date
	}
	//printf("%zd %zd\n", (ssize_t)block->size - (ssize_t)size, (ssize_t)block->size - (ssize_t)size <  (ssize_t)(sizeof(d_block) + 8));
	d_block* newblock = (d_block*)((char*)block + sizeof(d_block) + size);
	newblock->size = block->size - size - sizeof(d_block);
	printf("newblock %p size %zd\n", newblock, newblock->size);
	memset((char*)newblock + sizeof(d_block), 0, newblock->size);
	//printf("newblock acces incoming, old block has %zd size versus req of %zd\n", block->size, size);
	//printf("newblock acces success\n");
	newblock->free = 1;
	if(block->last)
	{	// split pe ultimu bloc creeaza un nou ultim bloc
		block->last = 0;
		newblock->last = 1;
	}
	else
	{
		newblock->last = 0;
	}
	insert_block_in_bin(newblock);
	block->size = size;
	if(!is_valid_addr(block)) printf("how did this happen\n");
	// size vine gata aliniat
	return block;
}

