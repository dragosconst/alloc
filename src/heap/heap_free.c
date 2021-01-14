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
		//perror("eroare la dezalocarea unui heap");
		return 0;
	}
	return 1;
}

void
scan_and_kill_heaps(d_heap* ignore)
{
	d_heap* heap = heap_top;
	while(free_heaps > MAX_FREE_HEAPS)
	{
		d_block* first_block = (d_block*)(heap + 1);
		if(heap != ignore && first_block->free && first_block->size == heap->all_size)
		{	// avem un heap liber
			d_heap* to_del = heap;
			heap = heap->prev;

			if(get_bin_type(first_block->size) >= 0)
			{
				remove_block_from_bin(first_block);
			}

			if(to_del == heap_top)
				heap_top = heap;
			to_del->prev->next = to_del->next;
			to_del->next->prev = to_del->prev;
			if(munmap(to_del, to_del->all_size) < 0)
			{
				//perror("eroare la dezalocarea unui heap in scan");
				return;
			}
			free_heaps--;
		}
		else
			heap = heap->prev;
	}
}

d_heap*
free_some_to_os(d_block* block)
{
	// functia e apelata cand vreau sa dau pagini la OS pana raman cu ceva ce poate fi pastrat in bin
	d_heap* heap = get_heap_of_block(block);
	// stim sigur ca putem da fara probleme tot peste VBIG_BLOCK_SIZE *2
	size_t total_size = block->size;
	size_t to_free = total_size - (size_t)VBIG_BLOCK_SIZE;
	char* max_len = (char*)block + sizeof(d_block) + (size_t)VBIG_BLOCK_SIZE; // adresa pana unde ar putea sa se intinda block-ul dat, fara sa depaseasca cel mai mare bin
	size_t offset = (uintptr_t)(max_len) % getpagesize();
	to_free += ((uintptr_t)(max_len) % getpagesize());
	if((uintptr_t)(max_len) % getpagesize()) max_len -= ((uintptr_t)(max_len) % getpagesize()); // adaug orice mai ramane din pagina pe care se afla max_len
	if(munmap(max_len, to_free) < 0)
	{
		//perror("eroare la eliberarea de memorie");
		return NULL;
	}
	heap->all_size -= to_free; // could be buggy?
	block->size -= to_free; // block trebuie bagat la loc in bin de funtia care apeleaza aici
	return heap;
}
