#include "my_alloc.h"

void*
my_alloc(size_t size)
{
	if(!bins_initialized)
	{
		pthread_mutex_lock(&global_mutex);
		// n am gasit o metoda mai desteapta sa initializez bins
		for(int i = 0; !bins_initialized && i < 66; ++i)
			pseudo_bins[i] = NULL;
		bins_initialized = 1;
		pthread_mutex_unlock(&global_mutex);
	}
	if(size <= 0)
	{
		return NULL;
	}

	d_heap* heap;
	d_block* block;

	//printf("my_alloc.c:asking for %ld space\n", size);
	block = search_for_free_block(size); // functia asta cauta exclusiv in bins
	if(!block)
	{
		//printf("my_alloc.c: failed to find free block\n");
		pthread_mutex_lock(&global_mutex);
		heap = create_heap(size);
		block = _unlock_search_for_free_block(size);
		if(!block)
		{	// asta inseamna ca e prea mare sa poata fi alocat pe bins, deci heap-ul tocmai creat e alocat special doar pt el
			block = (d_block*)(heap + 1);
			block->free = 0;
			free_heaps--;
			//printf("large request of %ld\n", size);
		}
		pthread_mutex_unlock(&global_mutex);
	}
	//printf("my_alloc.c: bloc adresa: %zd, size: %zd, struct size: %zd, last: %d\n\n",block, block->size, sizeof(d_block), block->last);
	pthread_mutex_lock(&global_mutex);
	//show_all_bins();
	//show_all_heaps();
	pthread_mutex_unlock(&global_mutex);
	return (block + 1);
}

