struct block_meta {
	size_t size;
	struct block_meta *next;
	int free;
	int magic;
};

#define META_SIZE sizeof(struct block_meta)

struct block_meta* find_free_block(struct block_meta** last, size_t size);
struct block_meta* request_space(struct block_meta* last, size_t size);

void* my_alloc(size_t size);
void* my_free(void* ptr);


