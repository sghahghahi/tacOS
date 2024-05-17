#include "kernel/types.h"
#include "user/umalloc.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

int main(void)
{
//  malloc_setfsm(FSM_FIRST_FIT);

  char *a = malloc(132);
  char *b = malloc(42);  /* Will be deleted */
  char *c = malloc(132);
  char *d = malloc(132); /* Will be deleted */
  char *e = malloc(132);
  char *f = malloc(3132);

  /* These allocations total 4096 bytes after alignment is included. */

  setName(a, "A");
  setName(b, "B");
  setName(c, "C");
  setName(d, "D");
  setName(e, "E");
  setName(f, "F");

  free(b);
  free(d);

  /**
   * The following allocation will choose:
   * - First fit: free space at the end of the page (unnamed, no variable)
   * - Best fit:  B
   * - Worst fit: D
   */
  char *g = malloc(42);

  setName(g, "G");

  mallocPrint();

  /**
   * Things to check:
   * - correct alignment (evenly divisible by 16)
   * - free block sizes are correct (make sure not off by one)
   * - end of last block's memory address - first address = 4096
   */

  return 0;
}
