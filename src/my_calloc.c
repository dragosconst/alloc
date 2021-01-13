/*
	Functia calloc.
*/

#include "my_alloc.h"
#include <stdint.h>

void*
my_calloc(size_t count, size_t size)
{
	if(count <= 0 || size <= 0)
		return NULL;
	// check overflow !!
	if(SIZE_MAX / count < size)
		return NULL;
	void* ptr = my_alloc(count * size);

	pthread_mutex_lock(&global_mutex);
	memset(ptr, 0, count * size);
	pthread_mutex_unlock(&global_mutex);
	return ptr;
}
