#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	if (argc == 1) {
		printf("Please provide command to benchmark.\n");
		return 1;
	}

	int pid = fork();
	if (pid == -1) {
		fprintf(2, "Error forking.\n");
		return 1;
	} else if (pid == 0) {
		exec(*(argv + 1), argv + 1);
	} else {
		uint64 start = time();
		int status, num_syscalls;
		wait2(&status, &num_syscalls);
		uint64 end = time();
		uint64 delta = (end - start) / 1000000;

		printf("------------------\n");
		printf("Benchmark Complete\n");
		printf("Time Elapsed:\t%d ms\n", delta);
		printf("System Calls:\t%d\n", num_syscalls);
	}

	return 0;
}
