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

	d_heap* heap;
	d_block* block;
	//printf("asking for %ld space\n", size);
	block = search_for_free_block(size); // functia asta cauta exclusiv in bins
	if(!block)
	{
		heap = create_heap(size);
		block = search_for_free_block(size);
		if(!block)
		{	// asta inseamna ca e prea mare sa poata fi alocat pe bins, deci heap-ul tocmai creat e alocat special doar pt el
			block = (d_block*)(heap + 1);
			printf("large request of %ld\n", size);
		}
	}
	return (block + 1);

}
