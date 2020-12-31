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

int
get_bin_type(size_t size)
{
	printf("utils.c: checking size is %zd max size is %zd\n", size);
	// se presupune ca e dat un size care incape intr-un bin
	int index = (size / 8) - 1;
	if(index <= 63) // small bins
		return index;
	if(size >= BIG_BLOCK_SIZE / 2 && size  <= BIG_BLOCK_SIZE * 2)
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
	{
		if(size <= BIG_BLOCK_SIZE * 2 && pseudo_bins[64]) return 64;
		if(size <= VBIG_BLOCK_SIZE * 2 && pseudo_bins[65]) return 65;
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
