#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
	uint64 timestamp = time();
	printf("Current UNIX timestamp: %d\n", timestamp);

	return 0;
}
