#include "kernel/types.h"
#include "user/user.h"

#define MAX_NAME 8
#define PAGE_SZ 4096
#define MIN_DATA 16
#define HEADER_SZ 32
#define FREE true
#define BLOCK false

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
void addList(struct BlockHeader*, bool);
uint findNearestMultiple(uint);
bool isFree(struct BlockHeader*);
void mallocPrint(void);
void myStrncpy(char*, const char*, uint);
void setName(struct BlockHeader*, const char*);
