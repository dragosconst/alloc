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
	int index = (size / 8) - 1;
	if(index <= 63) // small bins
		return index;
	if(size >= BIG_BLOCK_SIZE / 4 && size  <= BIG_BLOCK_SIZE * 2)
		return 64; // stiu ca e magic number, dar din moment ce am doar 2 large bins, nu cred ca e o problema asa mare
	else if(size >= VBIG_BLOCK_SIZE / 4 && size <= VBIG_BLOCK_SIZE * 2)
		return 65;
	else
	{
		printf("parametru size trimis in get bin type prost?\n");
		return -1;
	}
}

int
get_closest_bin_type(size_t size)
{
	int index = (size / 8) - 1;
	if(index > 63)
	{	// pe large bins trebuie sa caut efectiv daca exista o valoare suitable
		if(size <= BIG_BLOCK_SIZE * 2 && pseudo_bins[64])
		{
			d_block* bin = pseudo_bins[64];
			d_block* old = bin;
			do
			{
				printf("utils.c: bin size is %zd block's size is %zd\n", bin->size, size);
				if(bin->size >= size)
					return 64;
				bin = bin->next;
			}while(old != bin);
		}
		if(size <= VBIG_BLOCK_SIZE * 2 && pseudo_bins[65])
		{
			d_block* bin = pseudo_bins[65];
			d_block* old = bin;
			do
			{
				printf("utils.c: bin size is %zd block's size is %zd\n", bin->size, size);
				if(bin->size >= size)
					return 65;
				bin = bin->next;
			}while(old != bin);
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
