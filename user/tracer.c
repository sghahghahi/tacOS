#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	int pid = fork();
	if (pid == 0) {
		strace();
		exec(*(argv + 1), argv + 1);
	} else {
		int time;
		wait(&time);
	}

	return 0;
}
