#include "my_alloc.h"

/*
	Legat de MT: la malloc se poate face o mica imbunatatire, anume e destul sa garantam ca
	putem executa functia search_for_free_block in mod atomic. Daca nu este gasit niciun
	block, atunci putem avea o problema, deoarece trebuie sa fim siguri ca heap-ul nou creat
	special pentru block-ul respectiv nu e populat de alte malloc-uri. Deoarece dupa creearea
	heap-ului trebuie apelata din nou functia search_for_free_block, am folosit o variabila
	globala SEARCH_ATOMIC, care sa fie folosita de functia search_for_free_block pt a determina
	daca are sau nu nevoie de locks. M-am asigurat ca se scrie pe functia SEARCH_ATOMIC doar
	in mod thread-safe;
*/

void*
my_alloc(size_t size)
{
	printf("bruh\n");
	if(!bins_initialized)
	{
		if(pthread_mutex_lock(&global_mutex)) while(1);
		// n am gasit o metoda mai desteapta sa initializez bins
		for(int i = 0; !bins_initialized && i < 66; ++i)
			pseudo_bins[i] = NULL;
		bins_initialized = 1;
		if(pthread_mutex_unlock(&global_mutex)) while(1);
	}
	if(size <= 0)
	{
		return NULL;
	}
	d_heap* heap;
	d_block* block;
	printf("my_alloc.c:asking for %ld space\n", size);
	block = search_for_free_block(size); // functia asta cauta exclusiv in bins
	if(!block)
	{
		printf("my_alloc.c: failed to find free block\n");
		pthread_mutex_lock(&global_mutex); // vreau lock, ca sa nu fie ocupat heap-ul pana apuc sa-l folosesc
		heap = create_heap(size);
		printf("heap created\n");
		block = _unlock_search_for_free_block(size);
		pthread_mutex_unlock(&global_mutex);
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
	return (block + 1);
}
