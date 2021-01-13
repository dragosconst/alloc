CC = gcc
CFLAGS = -I../inc -Wpedantic -g -ggdb -pthread

STATIC = liballoc.a
SRC = ../src/compile_first.c ../src/utils/utils.c ../src/heap/heap.c ../src/block/block.c ../src/block/_unlock_block.c ../src/my_alloc.c ../src/heap/heap_free.c ../src/block/block_free.c ../src/my_free.c ../src/my_calloc.c ../src/unlocks/_unlock_free.c ../src/unlocks/_unlock_alloc.c ../src/my_realloc.c
OBJ = $(SRC:.c=.o)

$(STATIC): $(OBJ)
	@echo "[Link Static]"
	@ar rs $@ $^

.c .o:
	@echo [Compile] $<
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) *~ core tags *.bak Makefile.bal

all:	$(STATIC)

