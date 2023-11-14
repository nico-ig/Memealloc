#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "memalloc.h"

#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define PURPLE "\033[1;35m"
#define DEFAULT "\033[0m"
#define UNDERLINE "\033[4m"

extern void *original_brk;
static void *initial_brk;
static unsigned int num_tests = 0;
static unsigned int failed_tests = 0;

void assert(void *expected, void *recv) {
  num_tests++;

  printf("\tExpected: %s%p%s\n", BLUE, expected, DEFAULT);
  printf("\tReceived: %s%p%s\n", PURPLE, recv, DEFAULT);

  printf("\tRESULT: ");

  if ( expected == recv ) {
    printf("%sSUCCESS%s\n", GREEN, DEFAULT);
  } 
  else { 
    printf("%sFAILED%s\n", RED, DEFAULT);
    failed_tests++;
  }
  printf("\n");
}

void print_memory(char *expected, char *recv, unsigned long int size) {
  printf("\t%sExpected%s/%sReceived%s:\n", BLUE, DEFAULT, PURPLE, DEFAULT);

  printf("\t%s0x%02X%s/%s0x%02X%s ", BLUE, expected[0], DEFAULT, PURPLE, recv[0], DEFAULT);

  for ( unsigned long int i = 1; i < size; i++ ) {
    if ( i % 4 == 0 ) 
        printf("\n\t");

    printf("%s0x%02X%s/%s0x%02X%s ", BLUE, expected[i], DEFAULT, PURPLE, recv[i], DEFAULT);
  }

  printf("\n");
}

void compare_memory(char *expected, char *recv, unsigned long int size) {
  num_tests++;

  print_memory(expected, recv, size);

  printf("\tRESULT: ");

  if ( memcmp(expected, recv, size) == 0 ) {
    printf("%sSUCCESS%s\n", GREEN, DEFAULT);
  } 
  else { 
    printf("%sFAILED%s\n", RED, DEFAULT);
    failed_tests++;
  }
  printf("\n");
}

void total() {
  printf("==================== RESULTS ====================\n");
  printf("=====>> TOTAL: %u\n", num_tests);
  printf("=====>> %sSUCCESS%s: %u\n", GREEN, DEFAULT, num_tests - failed_tests);
  printf("=====>> %sFAILED%s: %u\n", RED, DEFAULT, failed_tests);
}

void test_setup_brk() {
  printf("=====>> SETUP BRK\n");

  void *expected_brk = current_brk();
  initial_brk = expected_brk;
  setup_brk();

  printf("------->> %sInitial brk address%s\n", UNDERLINE, DEFAULT);
  assert(expected_brk, original_brk);
}

void test_alloc_and_free_0_bytes() {
  printf("=====>> ALLOC 0 BYTES\n");
  
  void *expected_initial_addr = initial_brk; // The address should be the first brk
  void *expected_pointer_addr = expected_initial_addr + 16; // The pointer address should be after the block headder
  void *expected_brk = initial_brk + 16; // Should alloc just the block header
  
  void *p = memory_alloc(0);

  printf("------->> %sBrk address%s\n", UNDERLINE, DEFAULT);
  assert(current_brk(), expected_brk);

  printf("------->> %sBlock initial address%s\n", UNDERLINE, DEFAULT);
  assert(expected_initial_addr, p - 16);

  printf("------->> %sPointer address%s\n", UNDERLINE, DEFAULT);
  assert(expected_pointer_addr, p);

  printf("------->> %sFlag%s\n", UNDERLINE, DEFAULT);
  unsigned long int flag = *(unsigned long int*)(p - 16);
  assert((void *)1, (void *)flag);

  printf("------->> %sSize%s\n", UNDERLINE, DEFAULT);
  unsigned long int size = *(unsigned long int*)(p - 8);
  assert((void *)0, (void *)size);

  printf("------>> %sMemory layout%s\n", UNDERLINE, DEFAULT);
  char expected_memory[16];
  memset(expected_memory, 0, 16);
  expected_memory[0] = 1;
  expected_memory[8] = 0;

  compare_memory(expected_memory, p-16, (unsigned long int)(current_brk()-initial_brk));

  printf("------>> %sMemory free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;

  memory_free(p);
  compare_memory(expected_memory, p-16, (unsigned long int)(current_brk()-initial_brk));

  printf("------>> %sDismiss brk%s\n", UNDERLINE, DEFAULT);
  expected_brk = initial_brk;
  dismiss_brk();
  assert(expected_brk, original_brk);
}

void test_alloc_and_free_100_bytes() {
  printf("=====>> ALLOC 100 BYTES\n");
  
  void *expected_initial_addr = initial_brk; // The address should be the first brk
  void *expected_pointer_addr = expected_initial_addr + 16; // The pointer address should be after the block headder
  void *expected_brk = initial_brk + 116; // Should alloc the block header + 100 bytes
  
  void *p = memory_alloc(100);

  printf("------->> %sBrk address%s\n", UNDERLINE, DEFAULT);
  assert(expected_brk, current_brk());

  printf("------->> %sBlock initial address%s\n", UNDERLINE, DEFAULT);
  assert(expected_initial_addr, p - 16);

  printf("------->> %sPointer address%s\n", UNDERLINE, DEFAULT);
  assert(expected_pointer_addr, p);

  printf("------->> %sFlag%s\n", UNDERLINE, DEFAULT);
  unsigned long int flag = *(unsigned long int*)(p - 16);
  assert((void *)1, (void *)flag);

  printf("------->> %sSize%s\n", UNDERLINE, DEFAULT);
  unsigned long int size = *(unsigned long int*)(p - 8);
  assert((void *)100, (void *)size);

  printf("------->> %sMemory layout%s\n", UNDERLINE, DEFAULT);
  char expected_memory[116];
  memset(expected_memory, 0, 116);
  expected_memory[0] = 1;
  expected_memory[8] = 100;

  compare_memory(expected_memory, p-16, (unsigned long int)(current_brk()-initial_brk));

  printf("------>> %sMemory free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;

  memory_free(p);
  compare_memory(expected_memory, p-16, (unsigned long int)(current_brk()-initial_brk));

  printf("------>> %sDismiss brk%s\n", UNDERLINE, DEFAULT);
  expected_brk = initial_brk;
  dismiss_brk();
  assert(expected_brk, original_brk);
}

void test_free_address_not_allowed() {
  printf("=====>> ADDRESS NOT ALLOWED FREE\n");
  printf("------->> %sAddress before initial brk%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  
  void *p = initial_brk - 1;
  
  char expected_memory[17];
  memset(expected_memory, 0, 17);
  expected_memory[0] = 1;
  expected_memory[8] = 1;

  memory_free(p);
  compare_memory(expected_memory, p + 1, (unsigned long int)(current_brk()-initial_brk));
  dismiss_brk();

  printf("------->> %sAddress before end of block header%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  p = initial_brk + 1;
  memory_free(p);
  compare_memory(expected_memory, p - 1, (unsigned long int)(current_brk()-initial_brk));
  dismiss_brk();

  printf("------->> %sAddress after current brk%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  p = current_brk() + 1;

  memory_free(p);
  compare_memory(expected_memory, p - 18, (unsigned long int)(current_brk()-initial_brk));
  dismiss_brk();

  printf("------->> %sAddress is current brk%s\n", UNDERLINE, DEFAULT);
  memory_alloc(0);
  p = current_brk();

  expected_memory[0] = 0;
  expected_memory[8] = 0;

  memory_free(p);
  compare_memory(expected_memory, p - 16, (unsigned long int)(current_brk()-initial_brk));
  dismiss_brk();
}

void test_double_free() {
  printf("=====>> DOUBLE FREE\n");

  void *p = memory_alloc(1);
  char expected_memory[17];
  memset(expected_memory, 0, 17);
  expected_memory[0] = 0;
  expected_memory[8] = 1;

  printf("------->> %sFirst free%s\n", UNDERLINE, DEFAULT);
  memory_free(p);
  compare_memory(expected_memory, p - 16, (unsigned long int)(current_brk()-initial_brk));

  printf("------->> %sSecond free%s\n", UNDERLINE, DEFAULT);
  memory_free(p);
  compare_memory(expected_memory, p - 16, (unsigned long int)(current_brk()-initial_brk));

  dismiss_brk();
}

void test_alloc_same_block_after_free() {
  printf("=====>> ALLOC SAME BLOCK AFTER FREE\n");

  char expected_memory[21];
  memset(expected_memory, 0, 21);
  expected_memory[0] = 1;
  expected_memory[8] = 5;

  printf("------->> %sFirst alloc%s\n", UNDERLINE, DEFAULT);
  void *p1 = memory_alloc(5);
  void *brk1 = current_brk();
  printf("--------->> %sMemory after first alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(brk1-initial_brk));

  printf("--------->> %sMemory after free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;
  memory_free(p1);
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(brk1-initial_brk));

  printf("------->> %sSecond alloc (after free)%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 1;
  void *p2 = memory_alloc(5);
  void *brk2 = current_brk();

  printf("--------->> %sSame pointer%s\n", UNDERLINE, DEFAULT);
  assert(p1, p2);

  printf("--------->> %sBrk didn't change%s\n", UNDERLINE, DEFAULT);
  assert(brk1, brk2);

  printf("--------->> %sMemory after second alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p2 - 16, (unsigned long int)(brk2-initial_brk));

  dismiss_brk();
}

void test_alloc_after_free_dont_split() {
  printf("=====>> ALLOC AFTER FREE, DONT SPLIT\n");

  char expected_memory[33];
  memset(expected_memory, 0, 33);
  expected_memory[0] = 1;
  expected_memory[8] = 17;

  printf("------->> %sFirst alloc%s\n", UNDERLINE, DEFAULT);
  void *p1 = memory_alloc(17);
  void *brk1 = current_brk();
  printf("--------->> %sMemory after first alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(brk1-initial_brk));

  printf("--------->> %sMemory after free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;
  memory_free(p1);
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(brk1-initial_brk));

  printf("------->> %sSecond alloc (remaining space is of 16 bytes)%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 1;
  void *p2 = memory_alloc(1);
  void *brk2 = current_brk();

  printf("--------->> %sSame pointer%s\n", UNDERLINE, DEFAULT);
  assert(p1, p2);

  printf("--------->> %sBrk didn't change%s\n", UNDERLINE, DEFAULT);
  assert(brk1, brk2);

  printf("--------->> %sMemory after second alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p2 - 16, (unsigned long int)(brk2-initial_brk));

  dismiss_brk();
}

void test_alloc_two_consecutives_blocks() {
  printf("=====>> ALLOC TWO CONSECUTIVES BLOCKS\n");

  char expected_memory[40];
  memset(expected_memory, 0, 40);
  expected_memory[0] = 1;
  expected_memory[8] = 5;
  expected_memory[21] = 1;
  expected_memory[29] = 3;

  printf("------->> %sFirst alloc%s\n", UNDERLINE, DEFAULT);
  void *p1 = memory_alloc(5);
  void *brk1 = current_brk();

  printf("--------->> %sBrk after first alloc%s\n", UNDERLINE, DEFAULT);
  void *expected_brk = initial_brk + 21;
  assert(expected_brk, current_brk());

  printf("--------->> %sMemory after first alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(brk1-initial_brk));

  printf("------->> %sSecond alloc%s\n", UNDERLINE, DEFAULT);
  void *p2 = memory_alloc(3);
  void *brk2 = current_brk();

  printf("--------->> %sBrk after first alloc%s\n", UNDERLINE, DEFAULT);
  expected_brk += 19;
  assert(expected_brk, brk2);

  printf("--------->> %sEnd of first block is the start of second%s\n", UNDERLINE, DEFAULT);
  assert(p1 + 5, p2 - 16);

  printf("--------->> %sMemory after second alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(brk2-initial_brk));

  dismiss_brk();
}

void test_three_consecutives_blocks_free_second() {
  printf("=====>> ALLOC THREE CONSECUTIVES BLOCKS, FREE THE SECOND ONE\n");

  char expected_memory[51];
  memset(expected_memory, 0, 51);
  expected_memory[0] = 1;
  expected_memory[8] = 1;
  expected_memory[17] = 1;
  expected_memory[25] = 1;
  expected_memory[34] = 1;
  expected_memory[42] = 1;

  void *p1 = memory_alloc(1);
  void *p2 = memory_alloc(1);
  memory_alloc(1);

  printf("------->> %sBrk after allocs%s\n", UNDERLINE, DEFAULT);
  void *expected_brk = initial_brk + 51;
  assert(expected_brk, current_brk());

  printf("------->> %sMemory after allocs%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(current_brk()-initial_brk));

  printf("------->> %sMemory after free in second block%s\n", UNDERLINE, DEFAULT);
  memory_free(p2);
  expected_memory[17] = 0;
  compare_memory(expected_memory, p1 - 16, (unsigned long int)(current_brk()-initial_brk));

  dismiss_brk();
}

int main() {
  printf("===================== TESTS =====================\n");
  test_setup_brk();
  test_alloc_and_free_0_bytes();
  test_alloc_and_free_100_bytes();
  test_free_address_not_allowed();
  test_double_free();
  test_alloc_same_block_after_free();
  test_alloc_after_free_dont_split();
  test_alloc_two_consecutives_blocks();
  test_three_consecutives_blocks_free_second();

  total();
  return 0;
}
