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
	d_block* best = NULL;
	ssize_t closest_fit = VBIG_BLOCK_SIZE * 2;
	printf("closest fit is %zd differance is %zd\n", closest_fit,  (ssize_t)bin_start->size - (ssize_t)size);
	printf("stuck in find_best_fit\n");
	printf("old is %p old->next %p old->prev %p\n", old, old->next, old->prev);
	do
	{
		if((ssize_t)bin_start->size - (ssize_t)size >= 0 && (ssize_t)bin_start->size - (ssize_t)size < closest_fit)
		{
			printf("val is %zd and abs is %zd\n", (ssize_t)bin_start->size - (ssize_t)size, abs_big((ssize_t)bin_start->size - (ssize_t)size));
			closest_fit = bin_start->size - size;
			best = bin_start;
		}
		bin_start = bin_start->next;
		printf("bin start is %p, but old is %p\n", bin_start, old);
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
	int bin_index = get_bin_type(size);
	d_block* bin_block = pseudo_bins[bin_index];
	printf("block.c: asking for bin %d...\n", bin_index);

	if(bin_block && size >= BIG_BLOCK_SIZE / 4)
	{	// la large bins trebuie facut si un split si un search mai comprehensiv
		bin_block = find_best_fit(size, bin_block);
		printf("im splittin stuff\n");
		if(bin_block)	// e posibil ca find best fit sa dea fail
			bin_block = split_block(size, bin_block);
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
		printf("block.c: asking for %zd size, bin size is %zd, bin index is %d\n", size, pseudo_bins[bin_index]->size, bin_index);
		bin_block = split_block(size, bin_block);
		bin_block->free = 0;

		if(bin_block->prev != bin_block)
		{
			printf("bin block %p, next %p, prev %p\n", bin_block, bin_block->next, bin_block->prev);
			bin_block->prev->next = bin_block->next; // scot din lista dublu inlantuita
			bin_block->next->prev = bin_block->prev;
			if(bin_block == pseudo_bins[bin_index])
				pseudo_bins[bin_index] = bin_block->next;
		}
		else pseudo_bins[bin_index] = NULL;
		printf("block.c: request granted\n");
		//bin_block->next = bin_block->prev = NULL;
		return bin_block;
	}

	bin_block->free = 0;

	if(bin_block->prev != bin_block)
	{
	printf("bin block %p, next %p, prev %p\n", bin_block, bin_block->next, bin_block->prev);
		bin_block->prev->next = bin_block->next; // scot din lista dublu inlantuita
		bin_block->next->prev = bin_block->prev;
		if(bin_block == pseudo_bins[bin_index]) // la large bins e posibil sa fie altundeva block ul gasit
			 pseudo_bins[bin_index] = bin_block->next;
	}
	else pseudo_bins[bin_index] = NULL;
	//bin_block->next = bin_block->prev = NULL;
	return bin_block;
}

d_block*
split_block(size_t size, d_block* block) // nu modifica campul free din block-ul initial
{
	if(!is_valid_addr(block)) printf("how did this happen\n");
	// size has to be aligned
	printf("im splitting\n");
	if((ssize_t)block->size - (ssize_t)size < (ssize_t)(sizeof(d_block) + 8)) // daca spatiul in plus e prea mic sa mai bagam metadate
	{
	printf("no split req %zd %zd\n", (ssize_t)block->size - (ssize_t)size, (ssize_t)block->size - (ssize_t)size <  (ssize_t)(sizeof(d_block) + 8));
		return block; // nu are rost sa fac split, ca as corupe segmentul de date
	}
	printf("%zd %zd\n", (ssize_t)block->size - (ssize_t)size, (ssize_t)block->size - (ssize_t)size <  (ssize_t)(sizeof(d_block) + 8));
	d_block* newblock = (d_block*)((char*)block + sizeof(d_block) + size);
	printf("newblock acces incoming, old block has %zd size versus req of %zd\n", block->size, size);
	newblock->size = block->size - size - sizeof(d_block);
	printf("newblock acces success\n");
	if(newblock->size < BIG_BLOCK_SIZE / 4 && newblock->size > block->size - size - sizeof(d_block))
	{	/*
			Singurul fel in care pot ajunge in situatia asta logic ar fi daca sizeof(d_block) nu ar fi multiplu de 8.
			Caz in care e posibil ca alinierea sa creasca size-ul noului bloc, dar eu de fapt o vreau mai mica, ca sa
			incape in spatiul disponibil. Asta inseamna o pierdere de maxim 7 bytes pe spli, deoarece in cel mai rau
			caz, cand nb->size mod 8 e 7, ultimii 7 bytes nu intra in size. Nu ar trebui sa afecteze functionalitatea
			prea mult, deoarece request-urile userului vor fi garantate, iar pierderea asta e oricum dependenta de
			arhitectura sistemului. Un sistem pe care sizeof(d_block) mod 8 e 0 nu pierde niciun byte.
			Oricum, pentru valori din large bins, nu ne intereseaza sa pastram un multiplu fix de 8, deci pierderea se
			va face doar pe valori <512 bytes.
		*/
		newblock->size -= 8; // TODO: eventual un macro in loc de 8 magic number
		printf("halt\n");
		while(1);
	}
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

	int bin_index = get_bin_type(newblock->size);
	//printf("block.c: bin index is %d\n", bin_index);
	// inserare in bin
	if(pseudo_bins[bin_index])
	{	// pastrez structura de lista dublu inlantuita circulara si adaug bloc-ul la final
		//printf("block.c: adding to queue\n");
		pseudo_bins[bin_index]->prev->next = newblock;
		newblock->next = pseudo_bins[bin_index];
		newblock->prev = pseudo_bins[bin_index]->prev;
		pseudo_bins[bin_index]->prev = newblock;
	}
	else
	{
		//printf("block.c: new bin\n");
		pseudo_bins[bin_index] = newblock;
		newblock->prev = newblock->next = newblock;
	}

	block->size = size;
	if(!is_valid_addr(block)) printf("how did this happen\n");
	// size vine gata aliniat
	return block;
}

