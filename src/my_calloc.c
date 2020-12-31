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

	/*
	Nu mi-e clar cat de necesar e mutex lock-ul asta. Din moment ce nu am cum sa accesez
	zona de memorie returnata pana cand e terminata functia, pare degeaba. Singuru
	pericol apare la double free, dar asta e o eroare a user-ului.
	*/
	pthread_mutex_lock(&global_mutex);
	memset(ptr, 0, count * size);
	pthread_mutex_unlock(&global_mutex);
	return ptr;
}
