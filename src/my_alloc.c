#include "my_alloc.h"

void*
my_alloc(size_t size)
{
	printf("bruh\n");
	if(pthread_mutex_lock(&global_mutex)) while(1);
	if(!bins_initialized)
	{
		// n am gasit o metoda mai desteapta sa initializez bins
		for(int i = 0; !bins_initialized && i < 66; ++i)
			pseudo_bins[i] = NULL;
		bins_initialized = 1;
	}
	if(size <= 0)
	{
	pthread_mutex_unlock(&global_mutex);
		return NULL;
	}
	d_heap* heap;
	d_block* block;
	printf("my_alloc.c:asking for %ld space\n", size);
	block = search_for_free_block(size); // functia asta cauta exclusiv in bins
	if(!block)
	{
		printf("my_alloc.c: failed to find free block\n");
		heap = create_heap(size);
		printf("heap created\n");
		block = search_for_free_block(size);
		if(!block)
		{	// asta inseamna ca e prea mare sa poata fi alocat pe bins, deci heap-ul tocmai creat e alocat special doar pt el
			block = (d_block*)(heap + 1);
			block->free = 0;
			//printf("large request of %ld\n", size);
		}
	}
	printf("my_alloc.c: bloc adresa: %zd, size: %zd, struct size: %zd, last: %d\n\n",block, block->size, sizeof(d_block), block->last);
	printf("after malloc\n");
	show_all_bins();
	show_all_heaps();
	if(pthread_mutex_unlock(&global_mutex)) while(1);
	return (block + 1);
}

