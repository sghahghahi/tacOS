#include "kernel/types.h"
#include "user/user.h"

#define MAX_NAME 8
#define MIN_DATA 16
#define HEADER_SZ 32

/* Structs */
typedef struct DoublyLinkedList {
	uint len;
	struct BlockHeader *head;
	struct BlockHeader *tail;
} LinkedList;

struct BlockHeader {
	char name[MAX_NAME];
	uint size;
	struct BlockHeader *next_block;
	struct BlockHeader *prev_block;
};

/* Functions */
void removeFreeList(struct BlockHeader*);
void addFreeList(struct BlockHeader*);
void addBlockList(struct BlockHeader*);
uint findNearestMultiple(uint);
bool isFree(struct BlockHeader*);
void mallocPrint(void);
void myStrncpy(char*, const char*, uint);
void setName(void*, const char*);
void* calloc(uint, uint);
void* firstFit(uint);
