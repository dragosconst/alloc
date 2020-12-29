/*
	Aici o sa fie functiile de freeing care afecteaza intreaga structura a heap-ului.
*/

#include "my_alloc.h"

int
free_heap_to_os(d_heap* heap)
{
	if(heap == heap_top)
		heap_top = heap->prev;

	// scot heap ul din lista de heap uri
	if(heap->prev && heap->next)
	{
		heap->prev->next = heap->next;
		heap->next->prev = heap->prev;
	}
	else if(heap->prev)
		heap->prev->next = NULL;
	else if(heap->next)
		heap->next->prev = NULL;

	// deoarece heap e alocat cu mmap, nu trebuie sa fac niciun fel de verificare la adresa sa
	//printf("heap add = %ld hepa mod page = %d\n", heap, (uintptr_t)heap % getpagesize());
	//printf("heap has %ld left\n", heap->all_size);
	if(munmap(heap, heap->all_size) < 0)
	{
		//printf("bruh %ld\n", heap->all_size);
		perror("eroare la dezalocarea unui heap");
		return 0;
	}
	return 1;
}

d_heap*
try_free_block_to_os(d_heap* heap, d_block* block)
{
	// functia asta incearca sa dea niste pagini libere inapoi la OS, doar daca are ce sa dea
	d_block* prev_block = block->prev;

	ssize_t offset = ((uintptr_t)block) % getpagesize(); // determin daca block-ul sta pe un inceput de pagina sau mai are si alt bloc in fata pe pagina lui
	ssize_t total_freed_size = block->size + sizeof(d_block);
	void* starting_add = block;
	ssize_t remaining_size = offset ? getpagesize() - offset : 0;// cat o sa ramana din block(incluzand metadatele) pe heap dupa ce dau inapoi OS-ului paginile libere
	if(offset) starting_add += remaining_size;
	if(total_freed_size > remaining_size && munmap(starting_add, total_freed_size - remaining_size) < 0)
	{
		perror("eroare la dezalocarea unor pagini in free_block");
		return NULL;
	}
	//heap->free_end_size -= total_freed_size - remaining_size;
	//printf("heap has %ld and i will free %ld\n", heap->all_size, (total_freed_size - remaining_size > 0 ? total_freed_size - remaining_size : 0));
	heap->all_size -= (total_freed_size - remaining_size > 0 ? (ssize_t)heap->free_end_size + total_freed_size - remaining_size : 0);
	if(prev_block)
	{
		prev_block->next = NULL;
		heap->free_end_size = remaining_size < total_freed_size ? remaining_size : total_freed_size + heap->free_end_size;
		return heap;
	}
	else
	{
		// daca era singurul block din heap ramas, putem sterge tot heap-ul
		if(free_heap_to_os(heap))
			return NULL; // nu avem ce returna, pt ca tot heap-ul e scos cu mmap
		else
		{
			printf("freeing heap to OS failed\n");
			return NULL;
		}
	}
}
