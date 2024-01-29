/**
 * @file broken.c
 * @author mmalensek
 *
 * This program contains a series of buggy, broken, or strange C functions for
 * you to ponder. Your job is to analyze each function, fix whatever bugs the
 * functions might have, and then explain what went wrong. Sometimes the
 * compiler will give you a hint.
 *
 *  ____________
 * < Good luck! >
 *  ------------
 *      \   ^__^
 *       \  (oo)\_______
 *          (__)\       )\/\
 *              ||----w |
 *              ||     ||
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int func_counter = 1;
#define FUNC_START() printf("\n\n%d.) %s\n", func_counter++, __func__);


/**
 * This awesome code example was taken from the book 'Mastering C Pointers,'
 * one of the famously bad resources on learning C. It was trying to demonstrate
 * how to print 'BNGULAR'... with pointers...? Maybe?
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *      Hint: where are string literals stored in memory?
 *      Hint: what is '66' in this example? Can we do better?
 * 
 * Pointers to strings are immutable, whereas string arrays are mutable.
 * We must change a from a char pointer to a char array and initalize at declaration
 * No need to use 66. We can replace it with 'B'
 */
void
angular(void)
{
  FUNC_START();

  char a[] = "ANGULAR";
  *a = 'B';
  printf("%s\n", a);
}

/**
 * This function is the next step after 'Hello world' -- it takes user input and
 * prints it back out! (Wow).
 *
 * But, unfortunately, it doesn't work.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 * name is initalized to 0, which makes it a NULL pointer
 * gets() tries to store the user-inputted string into name, but that means it is trying to dereference a null pointer (seg fault)
 * Either we can change name to a char array, or we can allocate memory on the heap for name. We can't forget to free it when we're done
 */
void
greeter(void)
{
  FUNC_START();

  char *name = (char *) malloc(sizeof(char) * 128);
  if (name == NULL) exit(1);

  printf("Please enter your name: ");
  gets(name, 128);

  // Remove newline character
  char *p = name;
  for ( ; *p != '\n' && *p != 0; p++) { }
  *p = '\0';

  printf("Hello, %s!\n", name);

  free(name);
}

/**
 * This 'useful' function prints out an array of integers with their indexes, or
 * at least tries to. It even has a special feature where it adds '12' to the
 * array.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 * The for loop goes out of bounds. We need to reduce the upper bound to 100 (length of stuff)
 */
void
displayer(void)
{
  FUNC_START();
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"

  int stuff[100] = { 0 };

  /* Can you guess what the following does without running the program? */
  /* Rewrite it so it's easier to read. */
  stuff[15] = 12;

  for (int i = 0; i < 100; ++i) {
    printf("%d: %d\n", i, stuff[i]);
  }

  #pragma GCC diagnostic pop
}

/**
 * Adds up the contents of an array and prints the total. Unfortunately the
 * total is wrong! See main() for the arguments that were passed in.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 * The upper bound of the for loop != to the size of the array
 * It is only the number of bytes the array pointer takes up in memory, which is either 1 or 2 bytes (32 or 64-bit OS)
 * We need to pass the size of the array to adder(), and then change the upper bound of the for loop to be the length of the array
 */
void
adder(int *arr, const int length)
{
  FUNC_START();

  int total = 0;

  for (int i = 0; i < length; ++i) {
    total += arr[i];
  }

  printf("Total is: %d\n", total);
}

/**
 * This function is supposed to be somewhat like strcat, but it doesn't work.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 * This function returns a wild pointer
 * We are returning a function that only has local-scope. It will be deallocated after the function terminates
 * We need to allocate memory on the heap for buf_start and then free it after we're done using it in main
 * There is no need for buf. We can strcpy a and b to buf_start
 */
char *
suffixer(char *a, char *b)
{
  FUNC_START();
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdangling-pointer"

  char *buf_start = (char *) malloc(sizeof(char) + 128);
  if (buf_start == NULL) exit(1);
  strcpy(buf_start, a);
  strcpy(buf_start + strlen(a), b);
  return buf_start;

  #pragma GCC diagnostic pop
}

/**
 * This is an excerpt of Elon Musk's Full Self Driving code. Unfortunately, it
 * keeps telling people to take the wrong turn. Figure out how to fix it, and
 * explain what's going on so Elon can get back to work on leaving Earth for
 * good.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 * The fixed size of 8 for all strings leads to string3 not having a null-terminator
 * We can instead not specify size when declaring street strings, except for street4
 * We need to allow street4 to be large enough to fit "saint agnew " in the event that string1 != string2
 * The length of new_name is 13 (including null-terminator), so we must declare street4 with size 13 to allow strcpy to stay within bounds
 */
void
driver(void)
{
  FUNC_START();

  char street1[] = { "fulton" };
  char street2[] = { "gomery" };
  char street3[] = { "piedmont" };
  char street4[13] = { "baker" };
  char street5[] = { "haight" };

  if (strcmp(street1, street2)) {
    char *new_name = "saint agnew ";
    strcpy(street4, new_name);
  }

  printf("Welcome to TeslaOS 0.1!\n");
  printf("Calculating route...\n");
  printf("Turn left at the intersection of %s and %s.\n", street5, street3);
}

/**
 * This function tokenizes a string by space, sort of like a basic strtok or
 * strsep. It has two subtle memory bugs for you to find.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 * Need to type-cast return value of malloc() to (char *)
 * Need to allocate 1 more byte than strlen(str) to account for null byte
 * Need to check if memory allocation is successful
 * Must create dummy pointer (and free it after we're done using it) that points to beginning of line so that when we free, we are freeing from the beginning of the memory block, not somewhere in the middle or near the end
 */
void
tokenizer(void)
{
  FUNC_START();

  char *str = "Hope was a letter I never could send";
  char *line = (char *) malloc(strlen(str) + 1);
  if (line == NULL) return;
  char *lineToFree = line;
  char *c = line;

  strcpy(line, str);

  while (*c != '\0') {

    for ( ; *c != ' ' && *c != '\0'; c++) {
      // find the next space (or end of string)
    }

    *c = '\0';
    printf("%s\n", line);

    line = c + 1;
    c = line;
  }

  free(lineToFree);
}

/**
* This function should print one thing you like about C, and one thing you
* dislike about it. Assuming you don't mess up, there will be no bugs to find
* here!
*/
void
finisher(void)
{
  FUNC_START();

  printf("One thing I like about C: It forces me to write secure and clean code\n");
  printf("One thing I dislike about C: The uninformative compiler\n");
}

int
main(void)
{
  printf("Starting up!");

  angular();

  greeter();

  displayer();

  int nums[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };
  adder(nums, sizeof(nums) / sizeof(*nums));

  char *result = suffixer("kayak", "ing");
  printf("Suffixed: %s\n", result);
  free(result); // Free allocated memory returned from suffixer()

  driver();

  tokenizer();

  finisher();

  return 0;
}
