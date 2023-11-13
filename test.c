#include <stdio.h>
#include <unistd.h>

#include "memalloc.h"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define DEFAULT "\033[0m"

extern void *original_brk;
static unsigned int num_tests = 0;
static unsigned int failed_tests = 0;

void assert(void *a, void *b) {
  num_tests++;

  printf("\tRESULT: ");

  if ( a == b) {
    printf("%sSUCCESS%s\n", GREEN, DEFAULT);
  } 
  else { 
    printf("%sFAILED%s\n", RED, DEFAULT);
    failed_tests++;
  }
  printf("\n");
}

void total() {
  printf("============================ RESULTS ============================\n");
  printf("==>> TOTAL: %u\n", num_tests);
  printf("==>> %sSUCCESS%s: %u\n", GREEN, DEFAULT, num_tests - failed_tests);
  printf("==>> %sFAILED%s: %u\n", RED, DEFAULT, failed_tests);
}

int main() {
  printf("============================= TESTS =============================\n");
  printf("==>> SETUP BRK\n");

  void *expected_brk = sbrk(0);
  setup_brk();

  printf("-->> Initial brk address\n");
  printf("\tExpected: %p\n", expected_brk);
  printf("\tReceived: %p\n", original_brk);
  assert(expected_brk, original_brk);

  printf("==>> FIRST ALLOC, 0 BYTES\n");

  void *expected_initial_addr = expected_brk; // The address should be the last brk
  void *expected_pointer_addr = expected_initial_addr + 16; // The pointer address should be after the block headder
  expected_brk += 16; // Should alloc just the block header
  
  void *p = memory_alloc(0);

  printf("-->> Brk address\n");
  printf("\tExpected: %p\n", sbrk(0));
  printf("\tReceived: %p\n", expected_brk);
  assert(sbrk(0), expected_brk);

  printf("-->> Block initial address\n");
  printf("\tExpected: %p\n", expected_initial_addr);
  printf("\tReceived: %p\n", p - 16);
  assert(expected_initial_addr, p - 16);

  printf("-->> Pointer address\n");
  printf("\tExpected: %p\n", expected_pointer_addr);
  printf("\tReceived: %p\n", p);
  assert(expected_pointer_addr, p);

  printf("-->> Flag\n");
  unsigned long int flag = *(unsigned long int*)(p - 16);
  printf("\tExpected: 1\n");
  printf("\tReceived: %lu\n", flag);
  assert((void *)1, (void *)flag);

  printf("-->> Size\n");
  unsigned long int size = *(unsigned long int*)(p - 8);
  printf("\tExpected: 0\n");
  printf("\tReceived: %lu\n", size);
  assert((void *)0, (void *)size);

  total();
  return 0;
}
