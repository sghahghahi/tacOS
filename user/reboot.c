#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
	char *output_msg = "Rebooting";
	const int LEN = strlen(output_msg);
	printf("\n");
	for (int i = 0; i < LEN; i++) {
		printf("%c", *(output_msg + i));
		sleep(2);
	}
	for (int i = 0; i < 3; i++) {
		printf(".");
		sleep(3);
	}
	printf("\n");
	reboot();

	return 0;
}
