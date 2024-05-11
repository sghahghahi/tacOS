#include "user/umalloc.h"
#include "kernel/stat.h"
#include "kernel/param.h"

LinkedList Flist, Blist;

void
addList(struct BlockHeader* mem_block, bool free)
{
	if (mem_block == NULL) return;

	LinkedList *list = NULL;
	if (free)	list = &Flist;
	else		list = &Blist;

	if (list->head == NULL) {
		list->head = list->tail = mem_block;
		list->head->prev_block = NULL;
		list->tail->next_block = NULL;
		list->len++;
		return;
	}

	list->tail->next_block = mem_block;
	mem_block->prev_block = list->tail;
	list->tail = mem_block;
	list->tail->next_block = NULL;
	list->len++;
}

uint
findNearestMultiple(uint size)
{
	if (size <= MIN_DATA) return MIN_DATA;

	uint new_size = MIN_DATA;
	while (new_size < size) new_size += MIN_DATA;

	return new_size;
}

void
free(void *ap)
{}

bool
isFree(struct BlockHeader* mem_block)
{
	return (mem_block->size & 1) == 1;
}

void
mallocPrint(void)
{
	struct BlockHeader *curr_block = Blist.head;
	printf("-- Current Memory State --\n");
	while (curr_block != NULL) {
		printf("[BLOCK %p-%p]\t%d\t[%s]\t'%s'\n",
			curr_block,
			(char *) curr_block + curr_block->size,
			curr_block->size,
			isFree(curr_block) ? "FREE" : "USED",
			curr_block->name);
		curr_block = curr_block->next_block;
	}

	printf("\n-- Free List --\n");
	curr_block = Flist.head;
	if (curr_block == NULL) {
		printf("NULL\n");
		return;
	}

	while (curr_block != NULL) {
		printf("[%p] -> ", curr_block);
		curr_block = curr_block->next_block;
	}
	printf("NULL\n");
}

/**
 * A bit different than regular `strncpy()`.
 * If `nbytes > MAX_NAME`, then only copy `MAX_NAME` bytes.
 * Otherwise, copy `nbytes` bytes from `src` to `dst`.
*/
void
myStrncpy(char* dst, const char* src, uint nbytes)
{
	if (dst == NULL || src == NULL) return;

	int upper_bound = nbytes > MAX_NAME ? MAX_NAME : nbytes;
	for (int i = 0; i < upper_bound; i++) {
		dst[i] = src[i];
	}
	dst[upper_bound] = '\0';
}

void
setName(struct BlockHeader* mem_block, const char* name)
{
	/**
	 * mem_block points to beginning of the data portion of the block,
	 * so we need to subtract by 1 to get back to the header
	*/
	myStrncpy((mem_block - 1)->name, name, strlen(name));
}

void*
malloc(uint nbytes)
{
	struct BlockHeader *mem_block = NULL;

	int total_size = findNearestMultiple(nbytes + sizeof(struct BlockHeader));
	mem_block = (struct BlockHeader *) sbrk(total_size);
	mem_block->size = total_size & ~1; // Set last bit to 1 to denote that it's not free
	mem_block->name[0] = '\0';
	mem_block->next_block = mem_block->prev_block = NULL;

	addList(mem_block, BLOCK);

	return mem_block + 1; // Pointer to start of memory chunk (not the header with medatada)
}
