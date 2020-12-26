#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef enum{
	VSMALL,
	SMALL,
	NORMAL
}heap_type;

typedef struct my_heap{
	struct my_heap* prev;
	struct my_heap* next;
	heap_type		type;
	size_t          all_size;
	size_t          free_size;
}d_heap; // tin metadate pt heap doar pt a eficientiza small heaps

typedef struct my_block {
	struct my_block* prev;
	struct my_block* next;
	size_t           size;
	int				 free;
}d_block;

extern d_heap heap_top; // vreau aceiasi versiune a variabilei in tot proiectul

#define META_BLOCK_SIZE sizeof(struct my_block)
#define META_HEAP_SIZE  sizeof(struct my_heap)
#define SMALL_BLOCK_SIZE  getpagesize() / 8 // in small heaps pun doar small blocks
#define VSMALL_BLOCK_SIZE getpagesize() / 16 // la fel la very small

// functiile importante
void* my_alloc(size_t size);
void* my_free(void* ptr);
void* my_calloc(size_t count, size_t size);
void* my_realloc(void* ptr, size_t newsize);

// functii interne ale bibliotecii
d_heap* search_for_free_heap(size_t size);
d_block* search_for_free_block(size_t size, d_heap* heap);
//void defragment_heap(d_heap* heap);
void* create_heap(size_t size);
void* append_block(size_t size, d_heap* heap);
