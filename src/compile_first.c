/*
	Fisierul asta trebuie compilat primul din biblioteca, deoarece contine cele mai
	importante initializari.
*/
#include "my_alloc.h"

d_heap* heap_top = NULL;
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
