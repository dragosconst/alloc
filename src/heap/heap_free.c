/*
	Aici o sa fie functiile de freeing care afecteaza intreaga structura a heap-ului.
*/

#include "my_alloc.h"

int
free_heap_to_os(d_heap* heap)
{
	if(heap == heap_top)
		heap_top == heap->prev;

	// scot heap ul din lista de heap uri
	if(heap->prev && heap->next)
	{
		heap->prev->next = heap->next;
		heap->next->prev = heap->prev;
	}
	else if(heap->prev)
		heap->prev->next = NULL;
	else
		heap->next->prev = NULL;

	// deoarece heap e alocat cu mmap, nu trebuie sa fac niciun fel de verificare la adresa sa
	if(munmap(heap, heap->all_size) < 0)
	{
		perror("eroare la dezalocarea unui heap");
		return 0;
	}
	return 1;
}

d_heap*
free_block_to_os(d_heap* heap, d_block* block)
{
	d_block* prev_block = block->prev;

	size_t offset = ((uintptr_t)block) % getpagesize(); // determin daca block-ul sta pe un inceput de pagina sau mai are si alt bloc in fata pe pagina lui
	size_t total_freed_size = block->size + sizeof(d_block);
	void* starting_add = block;
	size_t remaining_size = offset ? getpagesize() - offset : 0;// cat o sa ramana din block(incluzand metadatele) pe heap dupa ce dau inapoi OS-ului paginile libere
	if(offset) starting_add += remaining_size;
	if(munmap(starting_add, total_freed_size - remaining_size) < 0)
	{
		perror("eroare la dezalocarea unor pagini in free_block");
		return NULL;
	}
	//heap->free_end_size -= total_freed_size - remaining_size;
	heap->all_size -= total_freed_size - remaining_size;
	if(prev_block)
	{
		prev_block->next = NULL;
		heap->free_end_size += remaining_size;
		return heap;
	}
	else
	{
		// daca era singurul block din heap ramas, putem sterge tot heap-ul
		if(free_heap_to_os(heap))
			return NULL; // nu avem ce returna, pt ca tot heap-ul e scos cu mmap
		else
		{
			printf("freeing to OS failed\n");
			return NULL;
		}
	}
}
