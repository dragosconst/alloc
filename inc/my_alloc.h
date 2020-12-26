#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef struct my_heap{
	struct my_heap* prev;
	struct my_heap* next;
	size_t          all_size;
	size_t          free_size;
}d_heap;

typedef struct my_block {
	struct my_block* prev;
	struct my_block* next;
	size_t           size;
	int				 free;
}d_block;


#define META_SIZE sizeof(struct block_meta)

struct block_meta* find_free_block(struct block_meta** last, size_t size);
struct block_meta* request_space(struct block_meta* last, size_t size);

void* my_alloc(size_t size);
void* my_free(void* ptr);


