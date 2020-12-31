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

#ifndef MYALLOC_INCLU
#define MYALLOC_INCLU

/*typedef enum{
	MICRO,
	VSMALL,
	SMALL,
	NORMAL,
	BIG,
	VBIG,
	LARGE
}heap_type;*/

typedef struct my_heap{
	struct my_heap* prev;
	struct my_heap* next;
	//heap_type		type;
	size_t          all_size;
}d_heap; // tin metadate pt heap doar pt a eficientiza small heaps

typedef struct my_block {
	struct my_block* prev;
	struct my_block* next;
	size_t           size;
	int		  		 free;
	int 		 	 last; // daca e ultimul de pe heap-ul sau
}d_block;

extern d_heap* heap_top; // vreau aceiasi versiune a variabilei in tot proiectul
extern pthread_mutex_t global_mutex;
extern int bins_initialized;
extern d_block* pseudo_bins[66]; // o sa am doar 2 large bins
extern int MALLOC_ATOMIC; // flag care indica daca vreau sau nu ca malloc si free sa foloseasca locks; useful pt realloc

#define BLOCK_OFFSET sizeof(struct my_block) % 8
#define HEAP_OFFSET  sizeof(struct my_heap) % 8
#define VBIG_BLOCK_SIZE   getpagesize() * 4 // fac un fel de bins
#define BIG_BLOCK_SIZE	  getpagesize() / 2
/*#define NORMAL_BLOCK_SIZE getpagesize() / 8
#define SMALL_BLOCK_SIZE  getpagesize() / 16
#define VSMALL_BLOCK_SIZE getpagesize() / 32
#define MICRO_BLOCK_SIZE  getpagesize() / 64*/
// functiile importante
void* my_alloc(size_t size);
void my_free(void* ptr);
void* my_calloc(size_t count, size_t size);
void* my_realloc(void* ptr, size_t newsize);

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
int get_closest_bin_type(size_t size);
size_t aligned_size(size_t size);
int is_valid_addr(void* addr);
d_heap* get_heap_of_block(d_block* block);
d_heap* free_some_to_os(d_block* block);
int free_heap_to_os(d_block* block);
d_block* merge_blocks(d_block* bl, d_block* br);
ssize_t abs_big(ssize_t arg);

#endif //MYALLOC_INCLU
