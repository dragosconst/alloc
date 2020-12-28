/*
	Aici o sa implementez toate functiile legate strict de gestionat heap-urile.
	Motivatia pt heap-uri separate e felul in care mmap functioneaza,
	spre deosebire de apeluri la brk\sbrk, nu este un singur segment de date
	marit in continuu, ci fiecare mmap genereaza propriul segment separat (ce va fi un heap).

	Pentru alocari mici si foarte mici, o sa fie generat un heap cat un page, pe care se pot
	pune doar blocuri care la randul lor sunt mici sau foarte mici. Nu-s sigur cat de buna e
	definitia pentru mic (size <= pagesize / 8) si foarte mic (size <= pagesize / 16), dar
	sigur e mai eficient decat daca as aloca blocuri oricum pica. Evident, un astfel de heap
	nu o sa contina fix 8 blocuri mici, de exemplu, deoarece trebuie stocate si metadatele
	heap-ului si ale blocurilor. De asemenea, am introdus posibilitatea ca un bloc vsmall, de
	exemplu, sa poata fi pus si pe un heap small\normal cu destul spatiu liber. Cred ca long
	term strategia asta o sa minimizeze numarul de apeluri de mmap.

	Heap-urile sunt legate unul de altul printr-o lista dublu inlantuita, heap_top fiind
	heap-ul care nu are pe ramura next niciun alt heap. Blocurile sunt alocate pe heap-urile
	respective.
*/

#include "my_alloc.h"

d_heap*
create_heap(size_t size)
{
	d_heap* heap;
	if(size < VSMALL_BLOCK_SIZE)
	{
		heap = (d_heap*)mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(heap == MAP_FAILED)
			return NULL;
		heap->type = VSMALL;
		heap->all_size = heap->free_end_size =  getpagesize() - sizeof(d_heap);
		heap->biggest_fblock = 0;
		return heap;
	}
	else if(size < SMALL_BLOCK_SIZE)
	{
		heap = (d_heap*)mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(heap == MAP_FAILED)
			return NULL;
		heap->type = SMALL;
		heap->all_size = heap->free_end_size = getpagesize() - sizeof(d_heap);
		heap->biggest_fblock = 0;
		return heap;
	}
	else
	{
		heap = (d_heap*)mmap(NULL, size + sizeof(d_heap), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(heap == MAP_FAILED)
			return NULL;
		heap->type = NORMAL;
		heap->all_size = heap->free_end_size = closest_page_size(size + sizeof(d_heap)) - sizeof(d_heap);
		heap->biggest_fblock = 0;
		return heap;
	}
}

int
is_compatible_type(heap_type my_type, heap_type cmp_type)
{
	if(my_type == VSMALL) // orice tip e compatibil
		return 1;
	if(my_type == SMALL)
		if(cmp_type == VSMALL) // small nu e compatibil cu vsmall
			return 0;
		else
			return 1;
	return 0; // cu heap uri normale nu e compatibil niciun tip
}

d_heap*
search_for_free_heap(size_t size)
{
	d_heap* heap = heap_top;
	heap_type correct_type;
	if(size <= VSMALL_BLOCK_SIZE) correct_type = VSMALL;
	else if(size <= SMALL_BLOCK_SIZE) correct_type = SMALL;
	else correct_type = NORMAL;

	while(heap)
	{
		if(heap->type == correct_type &&
		  (heap->biggest_fblock >= size || (heap->free_end_size >= size + sizeof(d_block))))
			// verific mai exact daca ori am destul free size in vreun block freed, ori am destul free size in coada heap-ului
			return heap;
		else if(is_compatible_type(correct_type, heap->type) &&
		  (heap->biggest_fblock >= size || (heap->free_end_size >= size + sizeof(d_block))))
			return heap;
		heap = heap->prev; // heap_top e heap-ul fara next, deci mergem catre prev
	}
	return NULL; // n avem heap necesar
}

d_heap*
heap_of_same_type(size_t size)
{
	heap_type correct_type;
	if(size <= VSMALL_BLOCK_SIZE) correct_type = VSMALL;
	else if(size <= SMALL_BLOCK_SIZE) correct_type = SMALL;
	else correct_type = NORMAL;

	d_heap* heap = heap_top;
	while(heap)
	{
		if(heap->type == correct_type)
			return heap;
		heap = heap->prev;
	}
	return NULL;
}
