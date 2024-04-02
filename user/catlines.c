#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUF_SZ 1024

/* Global variables to keep track of buffer position & size */
char buffer[BUF_SZ];
int curr_idx = 0, last_char_idx = 0;

int
fastgets(int fd, char* buf, uint buf_sz)
{
	/**
	 * 1) Don't read one byte at a time
	 * 		Instead, read a block of bytes at a time
	 * 		and read again if there is more to read
	 *
	 * 2) Increase buffer size (buffer that we're reading into)
	 * 		Resizing is more costly than the memory required
	 * 		for a bigger buffer from the start
	*/

	int total_chars_read = 0;
	char curr_char;

	while (total_chars_read < buf_sz - 1) {
		if (curr_idx >= last_char_idx) {
			last_char_idx = read(fd, buffer, buf_sz);
			curr_idx = 0;
			if (last_char_idx < 0) {
				fprintf(2, "Error reading from file.\n");
				return -1;
			} else if (last_char_idx == 0) break;
		}

		curr_char = buffer[curr_idx++];
		buf[total_chars_read++] = curr_char;
		if (curr_char == '\n' || curr_char == '\r') break;
	}

	buf[total_chars_read] = '\0';

	return total_chars_read;
}

int
main(int argc, char *argv[])
{
	if (argc <= 1) {
		fprintf(2, "Usage: %s filename\n", argv[0]);
		return 1;
  	}

	int fd = open(argv[1], O_RDONLY);
	char buf[BUF_SZ];
	int line_count = 0;
	while (fastgets(fd, buf, BUF_SZ) > 0) {
		printf("Line %d: %s", line_count++, buf);
	}
	printf("\n");

	return 0;
}
