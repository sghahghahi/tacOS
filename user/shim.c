/* shim -> sh improved */

#include "user/shim.h"
// #include "user/linkedlist.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

void
add(LinkedList* list, struct Command* command)
{
	struct Command *new_node = (struct Command *) malloc(sizeof(*command));
	if (new_node == NULL) {
		fprintf(2, "Error allocating memory.\n");
		exit(1);
	}
	memcpy(new_node, command, sizeof(*command));

	if (list->head == NULL) {
		list->head = list->tail = new_node;
		list->len++;
		return;
	}

	list->tail->next = new_node;
	new_node->prev = list->tail;
	new_node->next = NULL;
	list->tail = new_node;
	list->len++;
}

void
remove(LinkedList* list) {
	list->head = list->head->next;
	free(list->head->prev);

	list->len--;
}

void
updateHistory(LinkedList* list, struct Command* command)
{
	if (list->len < 100) {
		add(list, command);
		return;
	}

	remove(list);
	add(list, command);
}

/**
 * @brief Prints last 100 commands
*/
void
printHistory(LinkedList* list)
{
	struct Command *curr = list->head;
	while (curr != NULL) {
		printf("%d\t%s\n", curr->command_no, curr->full_command);
		curr = curr->next;
	}
}

void
parseCommand(char* line, struct Command* command)
{
	command->full_command = (char *) malloc(strlen(line) + 1);
	if (command->full_command == NULL) {
		fprintf(2, "Error allocating memory.\n");
		exit(1);
	}
	strcpy(command->full_command, line);

	char *delimiter = " \t\r";
	command->tokens = (char **) malloc(MAXPATH * sizeof(char));
	if (command->tokens == NULL) {
		fprintf(2, "Error allocating memory.\n");
		exit(1);
	}

	char *next_tok = line;
	char *curr_tok;
	int i = 0;
	while ((curr_tok = next_token(&next_tok, delimiter)) != NULL) {
		command->tokens[i] = (char *) malloc(strlen(curr_tok) + 1);
		if (command->tokens[i] == NULL) {
			fprintf(2, "Error allocating memory.\n");
			exit(1);
		}
		strcpy(command->tokens[i++], curr_tok);
	}
	command->tokens[i] = NULL;
}

void
execute(char** command, int* status_code)
{
	int pid = fork();
	if (pid == -1) {
		fprintf(2, "Error forking.\n");
		return;
	} else if (pid == 0) {
		exec(command[0], command);
		fprintf(2, "Command %s not found.\n", command[0]);
		exit(1);
	}

	wait(status_code);
}

int
isDigit(char ch) { return ch >= '0' && ch <= '9'; }

struct Command
*findCommandNo(LinkedList* list, int no)
{
	if (list->head == NULL) return NULL;

	struct Command *curr = list->head;
	while (curr != NULL) {
		if (curr->command_no == no) return curr;
		curr = curr->next;
	}

	return NULL;
}

struct Command
*findCommandStr(LinkedList* list, char* command)
{
	if (list->head == NULL) return NULL;

	struct Command *curr = list->tail;
	while (curr != NULL) {
		if (isSubstring(command, curr->full_command)) return curr;
		curr = curr->prev;
	}

	return NULL;
}

struct Command
*lastValidCommand(struct Command* command, int* status_code, LinkedList* list)
{
	while (command != NULL) {
		char *curr_cmd = command->full_command;
		if (curr_cmd[0] == '!') {
			if (curr_cmd[1] == '!') {
				return lastValidCommand(command->prev, status_code, list);
			} else if (isDigit(curr_cmd[1])) {
				executeCommands(command, status_code, list);
			}
			command = command->prev;
		}
		return command;
	}

	return NULL;
}

/**
 * @brief Executes commands from `command` and updates the `status_code`
 * for that `command`.
*/
void
executeCommands(struct Command *command, int *status_code, LinkedList* list)
{
	/* Handle built-ins */

	if (strcmp(command->tokens[0], "cd") == 0) {
		if (command->tokens[1] == NULL) {
			chdir("/");
			return;
		} else if (chdir(command->tokens[1]) == -1) {
			printf("cd: cannot open %s\n", command->tokens[1]);
			*status_code = 1;
		}
		return; // Successful `cd`

	} else if (strcmp(command->tokens[0], "history") == 0) {
		printHistory(list);
		return;

	} else if (strcmp(command->tokens[0], "exit") == 0) {
		exit(0);

	} else if (command->tokens[0][0] == '#') {
		return;

	} else if (command->tokens[0][0] == '!') {
		if (command->tokens[0][1] == '!') {
			// To handle multiple `!!` commands in a row
			struct Command *last_valid_cmd = lastValidCommand(list->tail, status_code, list);
			if(last_valid_cmd != NULL) executeCommands(last_valid_cmd, status_code, list);
			return;

		} else if (isDigit(command->tokens[0][1])) {
			// `+ 1` to move past `!`
			int num = atoi((next_token(command->tokens, " ") + 1));
			struct Command *found_command = findCommandNo(list, num);
			if (found_command != NULL) executeCommands(found_command, status_code, list);
			return;

		} else { // This is `!<prefix>`
			// Going to assume that input will be valid (probably a bad assumption)
			// `+ 1` to move past `!`
			struct Command *found_command = findCommandStr(list, command->full_command + 1);
			if (found_command != NULL) executeCommands(found_command, status_code, list);
			return;
		}
	}

	/* Execute any other type of command */
	execute(command->tokens, status_code);
}

/**
 * @brief Reads & executes commands from input file.
 * Opens and reads commands from `file_path`
 * @param file_path Name of the input file
 * @param line The command to parse
 * @param size Size of the command in bytes
 * @param status Exit status of the process
 * @param list `LinkedList` that keeps track of history
*/
void
execFile(char *file_path, char* line, uint* size, int* status, LinkedList* list)
{
	int i = 0;
	struct Command command;
	int fd = open(file_path, O_RDONLY);
	if (fd < 0) {
		fprintf(2, "Error opening file.\n");
		exit(1);
	}

	while (getline(&line, size, fd) >= 0) {
		command.command_no = i;
		parseCommand(line, &command);
		executeCommands(&command, status, list);
		if (
			command.tokens[0] == NULL ||
			command.tokens[0][0] == '!' ||
			command.tokens[0][0] == '#' ||
			strcmp(command.tokens[0], "history") == 0
		) continue;

		updateHistory(list, &command);
		i++;
	}

	close(fd);
}

/**
 * @brief Checks if `arg` is a shell script. Checks for a `.sh` file extension
 * @param arg The argument to check
 * @return `1` if `arg` is a shell script, `0` if not
*/
int
isFile(char* arg)
{
	int len = strlen(arg);
	return strcmp(arg + len - 3, ".sh") == 0;
}

int
isSubstring(const char* needle, const char* haystack)
{
	if (strcmp(needle, haystack) == 0) return 1;

	int i = 0;
	while (haystack[i] == needle[i]) { i++; }
	return i == strlen(needle);
}

int
main(int argc, char *argv[])
{
	int status_code = 0;
	char *line = NULL, cwd[MAXPATH];
	uint sz = 1024;

	LinkedList list;
	list.head = list.tail = NULL;
	list.len = 0;

	// I know worse is better...but I can't NOT check if argv[1] is a .sh file
	if (argc == 2 && isFile(argv[1])) {
		execFile(argv[1], line, &sz, &status_code, &list);
		return 0;
	}

	int i = 1;
	while (1) {
		struct Command command;
		command.command_no = i;

		getcwd(cwd, MAXPATH);
		printf("[%d]-[%d]-[%s]-> ", status_code, command.command_no, cwd);

		if (getline(&line, &sz, 0) <= 0) break;
		parseCommand(line, &command);
		executeCommands(&command, &status_code, &list);

		/* Add command to history */
		if (
			command.tokens[0] == NULL ||
			command.tokens[0][0] == '!' ||
			command.tokens[0][0] == '#' ||
			strcmp(command.tokens[0], "history") == 0
		) continue;

		updateHistory(&list, &command);
		i++;
	}

	return 0;
}
