#include "kernel/types.h"

#define bool _Bool
#define true 1
#define false 0
#define MAX_BUF 1024
#define ERROR mallocErr()

/* Structs */
struct Command {
	int command_no;
	int num_tokens;
	char **tokens; 				// Tokenized command. Ex: `{ "echo", "Hi", "|", "tolower" }`
	char *full_command; 		// Full command as one string. Ex: `{ "echo Hi | tolower" }`
	bool has_pipe;
	bool has_output_redirect;
	bool has_input_redirect;
	bool has_append;
	char *stdout_file;
	char *stdin_file;
	uint64 time_ran;			// Amount of time this process took to run in ms
	struct Command *next;
	struct Command *prev;
};

typedef struct DoublyLinkedList {
	int len;
	struct Command *head;
	struct Command *tail;
} LinkedList;


/* Linked List API */
void add(LinkedList*, struct Command*);
void remove(LinkedList*, int*);

/* Functions */
void mallocErr(void);
void updateHistory(LinkedList*, struct Command*);
void printHistory(LinkedList*);
void printTimes(LinkedList*);
void parseCommand(char*, struct Command*);
void execute(struct Command*, int*);
int isDigit(char);
struct Command* findCommandNo(LinkedList*, int);
struct Command* findCommandStr(LinkedList*, char*);
struct Command* lastValidCommand(struct Command*, int*, LinkedList*);
char* trimLeading(char*);
char* trimTrailing(char*);
char* trimWhitespace(char*);
void tokenizeCommand(char*, struct Command*);
bool hasLeadAndTrailWhitespace(char*);
bool hasLeadingWhitespace(char*);
void tokenizePipeline(struct Command*, LinkedList*);
void executeCommand(struct Command*, int*, LinkedList*);
int myStrncmp(const char*, const char*, int);
bool isSubstring(const char*, const char*);
void executePipeline(LinkedList*, int*);
void freeCommand(struct Command*);
void freeMemory(LinkedList*);
