#include "user/umalloc.h"
#include "kernel/stat.h"
#include "kernel/param.h"

LinkedList Flist, Blist;

void
removeFreeList(struct BlockHeader* mem_block)
{
	struct BlockHeader **next_free = (struct BlockHeader **) (mem_block + 1); // Pointer to data portion of block to check for free block
	struct BlockHeader **prev_free = (struct BlockHeader **) (mem_block + 2);

	if (*prev_free != NULL) {
		struct BlockHeader **next_free_prev = (struct BlockHeader **) (*prev_free + 1);
		*next_free_prev = *next_free;
	} else {
		Flist.head = *next_free;
	}

	if (*next_free != NULL) {
		struct BlockHeader **prev_free_next = (struct BlockHeader **) (*next_free + 2);
		*prev_free_next = *prev_free;
	} else {
		Flist.tail = *prev_free;
	}

	Flist.len--;
}

void
addFreeList(struct BlockHeader* mem_block)
{
	if (mem_block == NULL) return;

	struct BlockHeader **next_free = (struct BlockHeader **) (mem_block + 1);
	struct BlockHeader **prev_free = (struct BlockHeader **) (mem_block + 2);

	*next_free = NULL;
	*prev_free = Flist.tail;

	if (Flist.tail != NULL) {
		struct BlockHeader **next_free_tail = (struct BlockHeader **) (Flist.tail + 1);
		*next_free_tail = mem_block;
	} else {
		Flist.head = mem_block;
	}

	Flist.tail = mem_block;
	Flist.len++;
}

void
addBlockList(struct BlockHeader* mem_block)
{
	if (mem_block == NULL) return;

	if (Blist.head == NULL) {
		Blist.head = Blist.tail = mem_block;
		mem_block->prev_block = mem_block->next_block = NULL;
		Blist.len++;
		return;
	}

	// Check if block is already in list
	struct BlockHeader *curr_block = Blist.head;
	while (curr_block != NULL) {
		if (curr_block == mem_block) return; // Stop if block is already in list to avoid repetitive adds
		curr_block = curr_block->next_block;
	}

	Blist.tail->next_block = mem_block;
	mem_block->prev_block = Blist.tail;
	mem_block->next_block = NULL;
	Blist.tail = mem_block;
	Blist.len++;
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
{
	if (ap == NULL) return;

	// Get to header portion of block
	struct BlockHeader *mem_block = (struct BlockHeader *) ap - 1;
	if (isFree(mem_block)) return;

	// Set 0th bit of size field to 1 to denote free block
	mem_block->size |= 1;
	addFreeList(mem_block);
}

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
			(char *) curr_block + (curr_block->size & ~1),
			curr_block->size & ~1,
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
		curr_block = *((struct BlockHeader **) (curr_block + 1));
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
setName(void* block, const char* name)
{
	if (block == NULL || name == NULL) return;

	/**
	 * `mem_block` points to beginning of the data portion of the block,
	 * so we need to subtract by 1 to get back to the header
	*/

	struct BlockHeader *mem_block = (struct BlockHeader *) block - 1;
	myStrncpy(mem_block->name, name, strlen(name));
}

void*
calloc(uint nmemb, uint nbytes)
{
	uint sz = nmemb * nbytes;
	void *block = malloc(sz);
	memset(block, 0, sz);

	return block;
}

void*
firstFit(uint nbytes)
{
	struct BlockHeader *mem_block = Flist.head;
	while (mem_block != NULL) {
		uint block_size = mem_block->size & ~1;
		if (block_size >= nbytes + HEADER_SZ) {
			removeFreeList(mem_block);
			mem_block->size &= ~1;
			addBlockList(mem_block);

			return mem_block + 1;
		}
		mem_block = *((struct BlockHeader **) (mem_block + 1)); // Advance to next potential free block
	}

	return mem_block; // Will always be NULL here. Will return non-NULL block inside while loop
}

void*
malloc(uint nbytes)
{
	struct BlockHeader *mem_block = firstFit(nbytes);
	if (mem_block != NULL)  return mem_block;

	uint total_size = findNearestMultiple(nbytes + HEADER_SZ);
	mem_block = (struct BlockHeader *) sbrk(total_size);

	/**
	 * Set 0th bit of size field to 0 to denote it's in use.
	 * Not required since any multiple of 16 is guarnateed to leave the 0th bit at 0, but it's good to be explicit.
	*/
	mem_block->size = total_size & ~1;
	mem_block->name[0] = '\0';
	mem_block->next_block = mem_block->prev_block = NULL;

	addBlockList(mem_block);

	// Return data portion of block
	return mem_block + 1;
}
