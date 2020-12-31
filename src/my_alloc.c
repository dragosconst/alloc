#include "my_alloc.h"

void*
my_alloc(size_t size)
{
	if(!bins_initialized)
	{
		if(MALLOC_ATOMIC)
			pthread_mutex_lock(&global_mutex);
		// n am gasit o metoda mai desteapta sa initializez bins
		for(int i = 0; !bins_initialized && i < 66; ++i)
			pseudo_bins[i] = NULL;
		bins_initialized = 1;
		if(MALLOC_ATOMIC)
			pthread_mutex_unlock(&global_mutex);
	}
	if(size <= 0)
	{
		return NULL;
	}

	d_heap* heap;
	d_block* block;
	//printf("my_alloc.c:asking for %ld space\n", size);
	if(MALLOC_ATOMIC)
	{
		//printf("bruh\n");
		pthread_mutex_lock(&global_mutex);
	}
	else
		printf("HOW?????????????????????????????????????????????\n");
	printf("im outta here cuh %d\n", MALLOC_ATOMIC);
	block = search_for_free_block(size); // functia asta cauta exclusiv in bins
	if(!block)
	{
		//printf("my_alloc.c: failed to find free block\n");
		heap = create_heap(size);
		printf("heap created\n");
		block = search_for_free_block(size);
		if(!block)
		{	// asta inseamna ca e prea mare sa poata fi alocat pe bins, deci heap-ul tocmai creat e alocat special doar pt el
			block = (d_block*)(heap + 1);
			block->free = 0;
			printf("large request of %ld\n", size);
		}
	}
	//printf("my_alloc.c: bloc adresa: %zd, size: %zd, struct size: %zd, last: %d\n\n",block, block->size, sizeof(d_block), block->last);
	if(MALLOC_ATOMIC)
	{
		printf("bruh commenced!!!!!!!!!!!!!!!!\n");
		pthread_mutex_unlock(&global_mutex);
	}
	return (block + 1);
}
