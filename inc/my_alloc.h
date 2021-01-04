#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <stdalign.h>

#ifndef MYALLOC_INCLU
#define MYALLOC_INCLU

typedef struct _my_heap{
	struct _my_heap* prev;
	struct _my_heap* next;
	size_t          all_size;
}_unaligned_d_heap;


typedef struct my_heap{
	struct my_heap* prev;
	struct my_heap* next;
	size_t          all_size;
	char			padding[8 - _Alignof(_unaligned_d_heap) % 8];
}d_heap; // tin metadate pt heap doar pt a eficientiza small heaps

typedef struct _my_block{
	struct my_block* prev;
	struct my_block* next;
	size_t           size;
	int		  		 free;
	int 		 	 last; // daca e ultimul de pe heap-ul sau

}_unaligned_d_block;

typedef struct my_block {
	struct my_block* prev;
	struct my_block* next;
	size_t           size;
	int		  		 free;
	int 		 	 last; // daca e ultimul de pe heap-ul sau
	char			 padding[8 - _Alignof(_unaligned_d_block) % 8];
}d_block;


#define NBINS 		 		66
#define BLOCK_OFFSET 	  	sizeof(struct my_block) % 8
#define HEAP_OFFSET  	  	sizeof(struct my_heap) % 8
#define LARGE_RANGE			4 * getpagesize()
#define VBIG_BLOCK_SIZE   	(((NBINS - 64) * LARGE_RANGE) + 512) // cel mai mare block size care incape intr-un large bin
#define BIG_BLOCK_SIZE	  	513 // cel mai mic block size care incape intr-un large bin, 512 e cel mai mare smallbin
#define PREALLOC_THRESHOLD	getpagesize() // pentru valori sub macro-ul asta, prealoc niste memorie pe heap, pentru o posibila optimizare
/*
Un pic despre bins. Nu am implementat intocmai cum este in c standard, in primul rand am
64 de smallbins, in loc de 62, si numarul de large bins e definit de macroul NBINS - 64.
In al doilea rand, n-am mai implementat fast bins si unsorted bins, deoarece mi s-au parut
f dificil de scris si la unsorted nici nu am inteles foarte bine ideea din spate. In al
treilea rand, deoarece am facut large bins sa fie definibili de user prin macro, nu am
mai facut si optimizarea din c, in care large bins-urile pe size-uri mai mici au si un
range mai mic, toate large bins-urile mele au range 4 * getpagesize(). Tinand cont ca
implementarea mea de bins e asa de diferita de cea din C, si ca seamana mai mult doar
ca concept general, am denumit array-ul de bins "pseudo_bins", pentru ca totusi nu sunt
totuna cu bins-urile din glibc.
*/

extern d_heap* heap_top; // vreau aceiasi versiune a variabilei in tot proiectul
extern pthread_mutex_t global_mutex;
extern int bins_initialized;
extern d_block* pseudo_bins[NBINS]; // o sa am doar 2 large bins

// functiile importante
void* my_alloc(size_t size);
void my_free(void* ptr);
void* my_calloc(size_t count, size_t size);
void* my_realloc(void* ptr, size_t newsize);
void* _unlock_alloc(size_t size);
void _unlock_free(void* ptr);

// functii interne ale bibliotecii
d_block* search_for_free_block(size_t size); // un heap free ori are un bloc liber, ori mai are spatiu in coada
d_block* find_best_fit(size_t size, d_block* bin_start);
size_t find_biggest_free_block(d_heap* heap);
d_heap* create_heap(size_t size);
d_block* split_block(size_t size, d_block* block);
d_block* get_prev_block(d_block* block);
d_block* get_next_block(d_block* block);
size_t closest_page_size(size_t size);
int get_bin_type(size_t size);
void insert_block_in_bin(d_block* block);
int get_closest_bin_type(size_t size);
size_t aligned_size(size_t size);
int is_valid_addr(void* addr);
d_heap* get_heap_of_block(d_block* block);
d_heap* free_some_to_os(d_block* block);
int free_heap_to_os(d_block* block);
d_block* merge_blocks(d_block* bl, d_block* br);
void remove_block_from_bin(d_block* victim);
ssize_t abs_big(ssize_t arg);
void show_all_heaps(); // pt debugging
void show_all_bins();

#endif //MYALLOC_INCLU
