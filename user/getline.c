#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(void)
{
  // Case 1, have getline auto-allocate the buffer:
  uint sz = 0;
  char *line = 0;
  //
  // Case 2, provide a small buffer to test resizing:
  // uint sz = 10;
  // char *line = malloc(sz);


  int fd = open("README.md", O_RDONLY);
  while (true) {
    printf("Buffer address: %p\n", line);
    if (getline(&line, &sz, fd) <= 0) {
      break;
    }
    printf("Line:           %s\n", line);
  }
  return 0;
}
