/*
	Functii utile folosite prin proiect care nu sunt legate ca scope neaparat de vreo
	componenta logica (heap, block etc) a proiectului).
*/

#include "my_alloc.h"

size_t
closest_page_size(size_t size)
{
	return (size % getpagesize() == 0 ? size : getpagesize() * (size / getpagesize() + 1));
}

ssize_t
abs_big(ssize_t arg)
{
	return arg > 0 ? arg : -arg;
}

int
get_bin_type(size_t size)
{
	//printf("utils.c: checking size is %zd max size is %zd\n", size);
	// se presupune ca e dat un size care incape intr-un bin
	size_t index = (size / 8) - 1;
	printf("utils.c: index is %zd\n", index);
	if(index <= 63) // small bins
		return (int)index;
	else if(size <= VBIG_BLOCK_SIZE)
	{
		size_t old_min = 512;
		for(int i = 64; i < NBINS; ++i)
		{
			size_t min_size = old_min;
			size_t max_size = old_min + LARGE_RANGE;
			if(min_size <= size && size <= max_size)
				return i;
			old_min = max_size;
		}
	}
	else
	{
		// daca ajung aici, nu e neaparat trimis prost, de-aia am pus ?
		// sunt situatii in care verific daca functia returneaza -1 exclusiv
		printf("parametru size trimis in get bin type prost: %ld?\n", size);
		return -1;
	}
}

void
remove_block_from_bin(d_block* victim)
{
	if(victim->size > VBIG_BLOCK_SIZE) return; // am apelat pt un block care nu e in niciun bin
	if(pseudo_bins[get_bin_type(victim->size)] == victim) // e primul in bin
	{
		printf("ramura if\n");
		pseudo_bins[get_bin_type(victim->size)] = (victim->next == victim ? NULL : victim->next); // daca e singurul din bin, il inlocuiesc cu NULL
	}
	else
	{
		d_block* bin = pseudo_bins[get_bin_type(victim->size)];
		if(!bin) printf("thinking about %p\n", bin);
		printf("bin is %p and br is %p and bin->next is %p and br->next is %p and bin->prev is %p and br->prev is %p\n", bin, victim, bin->next, victim->next, bin->prev, victim->prev);
	}
	//printf("br %p are size %zd\n", br, sizeof(br));
	//printf("br->prev = %p, br->next = %p\n",br->prev,br->next);
	if(victim->prev)
		victim->prev->next = victim->next;
	if(victim->next)
		victim->next->prev = victim->prev;
}

void
insert_block_in_bin(d_block* block)
{
	if(block->size <= 512)
	{	// smallbin insertion, easy
		int bin_index = block->size / 8 - 1;
		if(!pseudo_bins[bin_index])
		{
			pseudo_bins[bin_index] = block;
			block->next = block->prev = block;
		}
		else
		{
			d_block* bin = pseudo_bins[bin_index];
			bin->prev->next = block;
			block->prev = bin->prev;
			block->next = bin;
			bin->prev = block;
		}
	}
	else if(block->size <= VBIG_BLOCK_SIZE)
	{	// large bin insertion, do insertion sort
		int bin_index = get_bin_type(block->size);
		if(!pseudo_bins[bin_index])
		{
			pseudo_bins[bin_index] = block;
			block->next = block->prev = block;
		}
		else
		{
			d_block* bin = pseudo_bins[bin_index];
			size_t cr_size;
			d_block* old = bin;
			do
			{	// mergem pe presupunerea ca bin-ul e deja sortat
				if(bin->size > block->size)
					break;
				bin = bin->next;
			}while(bin != old);

			bin->prev->next = block;
			block->prev = bin->prev;
			block->next = bin;
			bin->prev = block;
			if(bin == pseudo_bins[bin_index] && bin->size > block->size)
			{
				// cazul in care toate block urile din bin sunt mai mari, si deci trebuie pus pe prima pozitie din bin
				pseudo_bins[bin_index] = block;
			}
		}
	} // daca nu se incadreaza in niciun bin, nu facem nimic
}

int
get_closest_bin_type(size_t size)
{
	size_t index = (size / 8) - 1;
	if(index > 63)
	{	// pe large bins trebuie sa caut efectiv daca exista o valoare suitable, deoarece operez pe range-uri;
		for(int i = 64; i < NBINS; ++i)
		{
			if(pseudo_bins[i])
			{
				d_block* bin = pseudo_bins[i];
				d_block* old_bin = bin;
				do
				{
					if(bin->size >= size)
						return i;
					bin = bin->next;
				}while(bin != old_bin);
			}
		}
		return -1;
	}
	for(int i = index; i < 66; ++i)
		if(pseudo_bins[i] != NULL)
			return i;
	return -1;
}

size_t
aligned_size(size_t size)
{
// momentant fac alinierea la 8 bytes
	size_t align = 8;
	return (size + align - 1) & ~(align-1);
}

void
show_all_heaps()
{
	d_heap* heap = heap_top;
	if(!heap)
	{
		printf("no heaps to show\n\n\n");
		return;
	}

	for(int i = 0; i < 60; ++i)
		printf("-");
	printf("\n");

	d_heap* old = heap_top;
	do
	{
		printf("heap %p has following blocks: ", heap);
		d_block* bk = (d_block*)(heap + 1);
		int old_last = 0;
		do
		{
			printf("block %p, size %ld, is last %d, is free %d -> ", bk, bk->size,bk->last, bk->free);
			old_last = bk->last;
			if(!bk->last)
				bk = (d_block*)((char*)bk + sizeof(d_block) + bk->size);
		}while(!old_last);
		printf("nil\n");
		heap = heap->prev;
	}while(heap != old);

	for(int i = 0; i < 60; ++i)
		printf("-");
	printf("\n");
}

void
show_all_bins()
{
	for(int i = 0; i < 160; ++i)
		printf("-");
	printf("\n");
	for(int i = 0; i < 66; ++i)
	{
		if(pseudo_bins[i])
		{
			d_block* init = pseudo_bins[i];
			d_block* old = init;
			printf("bin %d has: ", i);
			int max_nr = 0;
			do
			{
				printf("%p with size %zd, ",init, init->size); 
				max_nr++;
				init = init->next;
			}while(old != init && max_nr <= 10);
			printf("\n");
		}
		else
		{
			printf("bin %d is empty\n", i);
		}
	}

	for(int i = 0; i < 160; ++i)
		printf("-");
	printf("\n");
}
