//* shim -> sh improved */

#include "user/shim.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

void
mallocErr(void)
{
	fprintf(STDERR_FILENO, "Memory allocation error.\n");
	exit(1);
}

void
add(LinkedList* list, struct Command* command)
{
	struct Command *new_node = (struct Command *) malloc(sizeof(*command));
	if (new_node == NULL) ERROR;

	memcpy(new_node, command, sizeof(*command));

	if (list->head == NULL) {
		list->head = list->tail = new_node;
		list->head->prev = NULL;
		list->tail->next = NULL;
		list->len++;
		return;
	}

	list->tail->next = new_node;
	new_node->prev = list->tail;
	new_node->next = NULL;
	list->tail = new_node;
	list->len++;
}

/**
 * Will try to remove `command` with corresponding `command_no`.
 * If `command_no` not specified, will remove oldest command in list (head)
 */
void
remove(LinkedList* list, int* command_no)
{
	if (list->head == NULL) return;

	if (command_no == NULL) {
		list->head = list->head->next;
		free(list->head->prev);
		list->head->prev = NULL;
		list->len--;
		return;
	}

	struct Command *curr = list->head;
	while (curr != NULL) {
		if (curr->command_no != *command_no) {
			curr = curr->next;
			continue;
		}

		if (list->len == 1) {
			free(curr);
			list->head = list->tail = NULL;
			list->len--;
			return;

		} else if (curr == list->head) {
			list->head = curr->next;
			free(curr);
			list->head->prev = NULL;
			list->len--;
			return;

		} else if (curr == list->tail) {
			list->tail = curr->prev;
			free(curr);
			list->tail->next = NULL;
			list->len--;
			return;
		}

		curr->prev->next = curr->next;
		curr->next->prev = curr->prev;
		free(curr);
		curr = NULL;
		list->len--;
		return;
	}
}

void
updateHistory(LinkedList* list, struct Command* command)
{
	if (list->len < 100) {
		add(list, command);
		return;
	}

	remove(list, NULL);
	add(list, command);
}

/* Prints last 100 `command`s */
void
printHistory(LinkedList* list)
{
	struct Command *curr = list->head;
	while (curr != NULL) {
		printf("%d\t%s\n", curr->command_no, curr->full_command);
		curr = curr->next;
	}
}

/* Prints run time for each `command` */
void
printTimes(LinkedList* list)
{
	struct Command *curr = list->head;
	while (curr != NULL) {
		printf("[%d|%dms]\t%s\n", curr->command_no, curr->time_ran, curr->full_command);
		curr = curr->next;
	}
}

/* Tokenizes `command` based on whitespace and sets corresponding flags for the `command` */
void
parseCommand(char* line, struct Command* command)
{
	line = trimWhitespace(line);
	command->has_pipe = command->has_output_redirect = command->has_input_redirect = command->has_append = false;
	command->stdout_file = command->stdin_file = NULL;
	command->full_command = (char *) malloc(strlen(line) + 1);
	if (command->full_command == NULL) ERROR;
	strcpy(command->full_command, line);

	char *delimiter = " \t\r";
	command->tokens = (char **) malloc(MAX_BUF * sizeof(char));
	if (command->tokens == NULL) ERROR;

	char *next_tok = line;
	char *curr_tok;
	int i = 0;
	while ((curr_tok = next_token(&next_tok, delimiter)) != NULL) {
		if (strcmp(curr_tok, "|") == 0) {
			command->has_pipe = true | command->has_pipe;

		} else if (strcmp(curr_tok, ">") == 0) {
			command->has_output_redirect = true;
			command->stdout_file = (char *) malloc(strlen(next_tok) + 1);
			if (command->stdout_file == NULL) ERROR;

			strcpy(command->stdout_file, next_tok);

		} else if (strcmp(curr_tok, "<") == 0) {
			command->has_input_redirect = true;
			command->stdin_file = (char *) malloc(strlen(next_tok) + 1);
			if (command->stdin_file == NULL) ERROR;

			strcpy(command->stdin_file, next_tok);

		} else if (strcmp(curr_tok, ">>") == 0) {
			command->has_append = true;
			command->has_output_redirect = true;
			command->stdout_file = (char *) malloc(strlen(next_tok) + 1);
			if (command->stdout_file == NULL) ERROR;

			strcpy(command->stdout_file, next_tok);
		}

		command->tokens[i] = (char *) malloc(strlen(curr_tok) + 1);
		if (command->tokens[i] == NULL) ERROR;

		strcpy(command->tokens[i++], curr_tok);
	}
	command->tokens[i] = NULL;
	command->num_tokens = i;
}

void
execute(struct Command* command, int* status_code)
{
	int pid = fork();
	if (pid == -1) {
		fprintf(STDERR_FILENO, "Error forking.\n");
		return;
	} else if (pid == 0) {
		exec(command->tokens[0], command->tokens);
		if (command->tokens[0] == NULL) exit(0); // Simply continue if user enters empty command
		fprintf(STDERR_FILENO, "Command %s not found.\n", command->tokens[0]);
		exit(1);
	}

	uint64 start = time();
	wait(status_code);
	uint64 end = time();
	command->time_ran = (end - start) / 1000000;
}

int
isDigit(char ch)
{
	return ch >= '0' && ch <= '9';
}

struct Command*
findCommandNo(LinkedList* list, int num)
{
	struct Command *curr = list->head;
	while (curr != NULL) {
		if (curr->command_no == num) return curr;
		curr = curr->next;
	}

	return NULL;
}

struct Command*
findCommandStr(LinkedList* list, char* command)
{
	struct Command *curr = list->tail;
	while (curr != NULL) {
		if (isSubstring(command, curr->full_command)) return curr;
		curr = curr->prev;
	}

	return NULL;
}

/* Finds most recent runnable `command` in `list` */
struct Command*
lastValidCommand(struct Command* command, int* status_code, LinkedList* list)
{
	while (command != NULL) {
		char *curr_cmd = command->full_command;
		if (curr_cmd[0] == '!') {
			if (curr_cmd[1] == '!') {
				return lastValidCommand(command->prev, status_code, list);
			} else if (isDigit(curr_cmd[1])) {
				executeCommand(command, status_code, list);
			}
			command = command->prev;
		}
		return command;
	}

	return NULL;
}

char*
trimLeading(char* str)
{
	while (*str == ' ' || *str == '\t' || *str == '\r') str++;
	return str;
}

char*
trimTrailing(char* str)
{
	int end = strlen(str);
	while (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r')	end--;
	str[end] = '\0';
	return str;
}

/* Trims leading and trailing whitespace in `str` */
char*
trimWhitespace(char* str)
{
	return trimLeading(trimTrailing(str));
}

/* Tokenizes `command` based on whitespace without updating any flags */
void
tokenizeCommand(char* line, struct Command* command)
{
	command->full_command = (char *) malloc(strlen(line) + 1);
	if (command->full_command == NULL) ERROR;

	strcpy(command->full_command, line);

	command->tokens = (char **) malloc(MAX_BUF * sizeof(char *));
	if (command->tokens == NULL) ERROR;

	char *delimiter = " \t\r";
	char *next_tok = line;
	char *curr_tok;
	int num_tokens = 0;
	while ((curr_tok = next_token(&next_tok, delimiter)) != NULL) {
		command->tokens[num_tokens] = (char *) malloc(strlen(curr_tok) + 1);
		if (command->tokens[num_tokens] == NULL) ERROR;

		strcpy(command->tokens[num_tokens++], curr_tok);
	}

	command->tokens[num_tokens] = NULL;
	command->num_tokens = num_tokens;
}

bool
hasLeadAndTrailWhitespace(char* line)
{
	int end = strlen(line);
	return (line[0] == ' ' && line[end - 1] == ' ');
}

bool
hasLeadingWhitespace(char* line)
{
	return line[0] == ' ';
}

/* Tokenizes `command` based on `"|<>"` and creates a new `command` with updated flags for each token */
void
tokenizePipeline(struct Command* command, LinkedList* list)
{
	char *delimiter = "|<>"; // Currently does not support append ">>"

	char *next_tok = (char *) malloc(strlen(command->full_command) + 1);
	if (next_tok == NULL) ERROR;

	strcpy(next_tok, command->full_command);  // strcpy() instead of simple assignment (x = y) so that the data stored in command->full_command is not altered
	char *curr_tok;

	int delim_index = 0;
	while ((curr_tok = next_token(&next_tok, delimiter)) != NULL) {
		struct Command cmd; // This command does not have any bool fields populated, it is essentially a blank command
		cmd.has_pipe = cmd.has_output_redirect = cmd.has_input_redirect = cmd.has_append = false;
		cmd.stdin_file = cmd.stdout_file = NULL;

		char delimiter_used[3] = { '\0' }; // Max space would be for ">>"
		delim_index += strlen(curr_tok);
		if (hasLeadAndTrailWhitespace(curr_tok)) {
			delim_index++;
			delimiter_used[0] = command->full_command[delim_index];
			if (command->full_command[delim_index + 1] == '>') { // Find '>>' in pipeline (next char after delimiter)
				strcpy(delimiter_used, ">>");
			}

		} else if (!hasLeadingWhitespace(curr_tok)) {
			delimiter_used[0] = command->full_command[delim_index]; // First command in pipeline
			if (command->full_command[delim_index + 1] == '>') { // Find '>>' in pipeline (next char after delimiter)
				strcpy(delimiter_used, ">>");
			}
		}

		if (strcmp(delimiter_used, "|") == 0) {
			cmd.has_pipe = true;
		} else if (strcmp(delimiter_used, ">>") == 0) {
			cmd.has_append = true;
			cmd.has_output_redirect = true;
			cmd.stdout_file = (char *) malloc(strlen(trimWhitespace(next_tok + 1)) + 1);
			if (cmd.stdout_file == NULL) ERROR;

			strcpy(cmd.stdout_file, trimWhitespace(next_tok + 1)); // + 1 to move past second '>' in '>>'

		} else if (strcmp(delimiter_used, "<") == 0) {
			cmd.has_input_redirect = true;
			// This assumes nothing comes after token after '<' token. Ex: "tolower < input.txt"
			cmd.stdin_file = (char *) malloc(strlen(trimWhitespace(next_tok)) + 1);
			if (cmd.stdin_file == NULL) ERROR;

			strcpy(cmd.stdin_file, trimWhitespace(next_tok));

		} else if (strcmp(delimiter_used, ">") == 0) {
			cmd.has_output_redirect = true;
			cmd.stdout_file = (char *) malloc(strlen(trimWhitespace(next_tok)) + 1);
			if (cmd.stdout_file == NULL) ERROR;

			strcpy(cmd.stdout_file, trimWhitespace(next_tok));
		}

		tokenizeCommand(curr_tok, &cmd); // Tokenize based on whitespace
		add(list, &cmd);
	}
}

/**
 * @brief Executes commands from `command` and updates the `status_code`
 * for that `command`.
*/

/**
 * Executes `command`s and updates their `status_code`s
 * If pipe or redirection identified, creates a new `LinkedList` of `command`s
 */
void
executeCommand(struct Command *command, int *status_code, LinkedList* list)
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
		if (command->tokens[1] == NULL) printHistory(list); // If history without '-t'
		else if (strcmp(command->tokens[1], "-t") == 0 || strcmp(command->tokens[1], "-T") == 0) printTimes(list);
		return;

	} else if (strcmp(command->tokens[0], "exit") == 0) {
		freeMemory(list);
		exit(0);

	} else if (command->tokens[0][0] == '#') {
		return;

	} else if (command->tokens[0][0] == '!') {
		if (command->tokens[0][1] == '!') { // To handle multiple `!!` commands in a row
			struct Command *last_valid_cmd = lastValidCommand(list->tail, status_code, list);
			if(last_valid_cmd != NULL) executeCommand(last_valid_cmd, status_code, list);
			return;

		} else if (isDigit(command->tokens[0][1])) {
			int num = atoi((next_token(command->tokens, " ") + 1)); // `+ 1` to move past `!`
			struct Command *found_command = findCommandNo(list, num);
			if (found_command != NULL) executeCommand(found_command, status_code, list);
			remove(list, &num);
			return;

		} else { // To handle `!<prefix>`
			// Going to assume that input will be valid (probably a bad assumption)
			struct Command *found_command = findCommandStr(list, command->full_command + 1); // `+ 1` to move past `!`
			if (found_command != NULL) executeCommand(found_command, status_code, list);
			return;
		}

	} else if (command->has_pipe || command->has_output_redirect || command->has_input_redirect || command->has_append) {
		LinkedList cmds;
		cmds.head = cmds.tail = NULL;
		cmds.len = 0;

		tokenizePipeline(command, &cmds);

		int pid = fork();
		if (pid == -1) {
			fprintf(STDERR_FILENO, "Error forking.\n");
			return;
		} else if (pid == 0) {
			executePipeline(&cmds, status_code);
			exit(0);
		} else {
			uint64 start = time();
			wait(status_code);
			uint64 end = time();
			command->time_ran = (end - start) / 1000000;
			return;
		}
	}

	/* Execute basic command */
	execute(command, status_code);
}

int
myStrncmp(const char* s1, const char* s2, int size)
{
	for (int i = 0; i < size; i++) {
		if (s1[i] != s2[i])	{
			return (int) s1[i] - (int) s2[i];
		}

		if (s1[i] == '\0' || s2[i] == '\0') break;;
	}

	return 0;
}

bool
isSubstring(const char* needle, const char* haystack)
{
	if (strcmp(needle, haystack) == 0) return 1;
	for (int i = 0; i < strlen(haystack); i++) {
		if (myStrncmp(needle, haystack + i, strlen(needle)) == 0) return true;
	}

	return false;
}

/* Executes all `command`s in `list`, where `list` is a complete pipeline full of `command`s */
void
executePipeline(LinkedList* list, int* status_code)
{
	int fd[2];
	struct Command *curr = list->head;

	while (curr != NULL) {
		if (pipe(fd) == -1) {
			fprintf(STDERR_FILENO, "Error creating pipe.\n");
			return;
		}

		int pid = fork();
		if (pid == -1) {
			fprintf(STDERR_FILENO, "Error forking.\n");
			exit(1);
		} else if (pid == 0) {
			if (curr->has_pipe) {
				// Send stdout to pipe
				close(1);
				dup(fd[1]);
				close(fd[0]);

			} else if (curr->stdout_file != NULL) {
				uint mode;
				if (curr->has_append) 	mode = O_WRONLY | O_CREATE | O_APPEND;
				else 					mode = O_WRONLY | O_CREATE;

				int file_fd = open(curr->stdout_file, mode);
				if (file_fd == -1) {
					fprintf(STDERR_FILENO, "Error opening file %s.\n", curr->stdout_file);
					exit(1);
				}
				close(1);
				dup(file_fd);;
				close(file_fd);

			} else if (curr->stdin_file != NULL) {
				int file_fd = open(curr->stdin_file, O_RDONLY);
				if (file_fd == -1) {
					fprintf(STDERR_FILENO, "Error opening file %s.\n", curr->stdin_file);
					return;
				}
				close(0);
				dup(file_fd);
				close(file_fd);
			}

			// Exit if we are at the last token (stdout file name)
			if (
				curr->prev != NULL &&
				curr->prev->stdout_file != NULL &&
				isSubstring(curr->tokens[0], curr->prev->stdout_file)
			) { exit(0); }

			// Exit if we are at the last token (stdin file name)
			if (
				curr->prev != NULL &&
				curr->prev->stdin_file != NULL &&
				strcmp(curr->tokens[0], curr->prev->stdin_file) == 0
			) { exit(0); }

			close(fd[0]);
			exec(curr->tokens[0], curr->tokens);
			if (curr->tokens[0] == NULL) exit(0); // Simply continue if user enters blank command
			fprintf(STDERR_FILENO, "Command %s not found.\n", curr->tokens[0]);
			exit(1);
		} else {
			if (curr->has_pipe) {
				// Receive stdin from pipe
				close(0);
				dup(fd[0]);
				close(fd[1]);
			}

			wait(status_code);
			close(fd[0]);
			close(fd[1]);
		}

		curr = curr->next;
	}
}

void
freeCommand(struct Command* command)
{
	free(command->full_command);
	for (int i = 0; i < command->num_tokens; i++) {
		free(command->tokens[i]);
	}
	free(command->tokens);
	if (command->stdout_file != NULL) free(command->stdout_file);
	if (command->stdin_file != NULL) free(command->stdin_file);
	free(command);
	command = NULL;
}

void
freeMemory(LinkedList* list)
{
	struct Command *curr = list->head;
	struct Command *next = NULL;
	while (curr != NULL) {
		next = curr->next;
		freeCommand(curr);
		curr = next;
	}
}

int
main(int argc, char *argv[])
{
	bool isScripting = false;
	int fd = 0;

	int status_code = 0;
	char *line = NULL, cwd[MAXPATH];
	uint sz = MAX_BUF;

	LinkedList list;
	list.head = list.tail = NULL;
	list.len = 0;

	if (argc > 1 && argv[1] != NULL) {
		isScripting = true;
		fd = open(argv[1], O_RDONLY);
		if (fd == -1) {
			fprintf(STDERR_FILENO, "Error opening file %s\n", argv[1]);
			exit(1);
		}
	}

	int i = 1;
	while (1) {
		struct Command command;
		command.command_no = i;

		getcwd(cwd, MAXPATH);
		if (!isScripting) printf("[%d]-[%d]-[%s]-> ", status_code, command.command_no, cwd);

		if (getline(&line, &sz, fd) <= 0) break;
		parseCommand(line, &command);
		executeCommand(&command, &status_code, &list);

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

	freeMemory(&list);
	return 0;
}
