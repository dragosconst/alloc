/*
	Fisierul asta trebuie compilat primul din biblioteca, deoarece contine cele mai
	importante initializari.
*/
#include "my_alloc.h"

d_heap* heap_top = NULL;
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
int bins_initialized = 0;
d_block* pseudo_bins[66] = {0};
int free_heaps = 0;
