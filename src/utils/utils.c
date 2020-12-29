/*
	Functii utile folosite prin proiect care nu sunt legate ca scope neaparat de vreo
	componenta logica (heap, block etc) a proiectului).
*/

#include "my_alloc.h"

size_t
closest_page_size(size_t size)
{
	return (size % getpagesize() == 0 ? size : getpagesize() * (size / getpagesize() + 1));
}
