#include "my_alloc.h"

void*
my_alloc(size_t size)
{
	if(!bins_initialized)
	{	// n am gasit o metoda mai desteapta sa initializez bins
		for(int i = 0; i < 66; ++i)
			pseudo_bins[i] = NULL;
		bins_initialized = 1;
	}
	if(size <= 0)
		return NULL;


	for(int i = 0; i < 66;++i)
	{	if(pseudo_bins[i])
		printf("bin %d has size %zd\n", i, pseudo_bins[i]->size);
	}
	d_heap* heap;
	d_block* block;
	printf("my_alloc.c:asking for %ld space\n", size);
	block = search_for_free_block(size); // functia asta cauta exclusiv in bins
	if(!block)
	{
		printf("my_alloc.c: failed to find free block\n");
		heap = create_heap(size);
		block = search_for_free_block(size);
		if(!block)
		{	// asta inseamna ca e prea mare sa poata fi alocat pe bins, deci heap-ul tocmai creat e alocat special doar pt el
			block = (d_block*)(heap + 1);
			printf("large request of %ld\n", size);
		}
	}
	printf("my_alloc.c: bloc adresa: %zd, size: %zd, struct size: %zd, last: %d\n\n",block, block->size, sizeof(d_block), block->last);
	return (block + 1);

}
