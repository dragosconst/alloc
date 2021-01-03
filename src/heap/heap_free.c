/*
	Aici o sa fie functiile de freeing care afecteaza intreaga structura a heap-ului.
*/

#include "my_alloc.h"

int
free_heap_to_os(d_block* block)
{
	d_heap* heap = get_heap_of_block(block);
	if(heap == heap_top)
		if(heap != heap->prev)
			heap_top = heap->prev;
		else
			heap_top = NULL;

	// scot heap ul din lista de heap uri
	heap->prev->next = heap->next;
	heap->next->prev = heap->prev;

	// deoarece heap e alocat cu mmap, nu trebuie sa fac niciun fel de verificare la adresa sa
	//printf("heap add = %ld hepa mod page = %d\n", heap, (uintptr_t)heap % getpagesize());
	//printf("heap has %ld left\n", heap->all_size);
	if(munmap(heap, heap->all_size) < 0)
	{
		perror("eroare la dezalocarea unui heap");
		return 0;
	}
	return 1;
}

d_heap*
free_some_to_os(d_block* block)
{
	// functia e apelata cand vreau sa dau pagini la OS pana raman cu ceva ce poate fi pastrat in bin
	d_heap* heap = get_heap_of_block(block);
	// stim sigur ca putem da fara probleme tot peste VBIG_BLOCK_SIZE *2
	size_t total_size = heap->all_size + sizeof(d_heap);
	size_t to_free = total_size - VBIG_BLOCK_SIZE;
	char* max_len = (char*)block + sizeof(d_block) + VBIG_BLOCK_SIZE; // adresa pana unde ar putea sa se intinda block-ul dat, fara sa depaseasca cel mai mare bin
	size_t offset = (uintptr_t)(max_len) % getpagesize();
	if((uintptr_t)(max_len) % getpagesize()) max_len -= ((uintptr_t)(max_len) % getpagesize()); // adaug orice mai ramane din pagina pe care se afla max_len
	to_free += ((uintptr_t)(max_len) % getpagesize());
	if(munmap(max_len, to_free) < 0)
	{
		perror("eroare la eliberarea de memorie");
		return NULL;
	}
	heap->all_size -= to_free;
	block->size -= (block->size - VBIG_BLOCK_SIZE - offset); // block trebuie bagat la loc in bin de funtia care apeleaza aici
	return heap;
}
