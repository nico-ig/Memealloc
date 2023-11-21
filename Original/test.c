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
  printf("/*\n");
  printf(" * Verify if the brk is corectly setup\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Setup brk\n");
  printf(" * 2. Verify if brk saved is the same as current brk\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> SETUP BRK\n");

  void *expected_brk = current_brk();
  initial_brk = expected_brk;
  setup_brk();

  printf("------->> %sInitial brk address%s\n", UNDERLINE, DEFAULT);
  assert(expected_brk, original_brk);
}

void test_alloc_and_free_0_bytes() {
  printf("/*\n");
  printf(" * Verify if the first block is allocated correctly\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc a block of size 0\n");
  printf(" * 2. Verify if block starts at initial brk\n");
  printf(" *\n");
  printf(" */\n");

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

  compare_memory(expected_memory, p-16, current_brk() - initial_brk);

  printf("------>> %sMemory free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;

  memory_free(p);
  compare_memory(expected_memory, p-16, current_brk() - initial_brk);

  printf("------>> %sDismiss brk%s\n", UNDERLINE, DEFAULT);
  expected_brk = initial_brk;
  dismiss_brk();
  assert(expected_brk, original_brk);
}

void test_alloc_and_free_100_bytes() {
  printf("/*\n");
  printf(" * Verify if alloc and free works for first block\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc a block of size 100\n");
  printf(" * 2. Free block\n");
  printf(" *\n");
  printf(" */\n");
  
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

  compare_memory(expected_memory, p-16, current_brk() - initial_brk);

  printf("------>> %sMemory free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;

  memory_free(p);
  compare_memory(expected_memory, p-16, current_brk() - initial_brk);

  printf("------>> %sDismiss brk%s\n", UNDERLINE, DEFAULT);
  expected_brk = initial_brk;
  dismiss_brk();
  assert(expected_brk, original_brk);
}

void test_free_address_not_allowed() {
  printf("/*\n");
  printf(" * Verify if free doens't attempt to access an address\n");
  printf(" * out of heap\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc block of size 1\n");
  printf(" * 2. Try to free an address before initial brk\n");
  printf(" * 3. Try to free an address after current brk\n");
  printf(" * 4. Free block\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> ADDRESS NOT ALLOWED FREE\n");

  void *p = initial_brk - 1;
  
  char expected_memory[17];
  memset(expected_memory, 0, 17);
  expected_memory[0] = 1;
  expected_memory[8] = 1;

  printf("------->> %sInitial memory%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  compare_memory(expected_memory, p + 1, current_brk() - initial_brk);

  printf("------->> %sAddress before initial brk%s\n", UNDERLINE, DEFAULT);
  printf("--------->> %sReturn value%s\n", UNDERLINE, DEFAULT);
  unsigned long int ret = memory_free(p);
  assert((void *)0, (void *)ret);

  printf("--------->> %sMemory didn't change%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p + 1, current_brk() - initial_brk);
  dismiss_brk();

  printf("------->> %sAddress before end of block header%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  p = initial_brk + 1;

  printf("--------->> %sReturn value%s\n", UNDERLINE, DEFAULT);
  ret = memory_free(p);
  assert((void *)0, (void *)ret);

  printf("--------->> %sMemory didn't change%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p - 1, current_brk() - initial_brk);
  dismiss_brk();

  printf("------->> %sAddress after current brk%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  p = current_brk() + 1;

  printf("--------->> %sReturn value%s\n", UNDERLINE, DEFAULT);
  ret = memory_free(p);
  assert((void *)0, (void *)ret);

  printf("--------->> %sMemory didn't change%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p - 18, current_brk() - initial_brk);
  dismiss_brk();

  printf("------->> %sAddress is current brk%s\n", UNDERLINE, DEFAULT);
  memory_alloc(0);
  p = current_brk();

  printf("--------->> %sReturn value%s\n", UNDERLINE, DEFAULT);
  ret = memory_free(p);
  assert((void *)1, (void *)ret);

  printf("--------->> %sMemory changed%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;
  expected_memory[8] = 0;

  compare_memory(expected_memory, p - 16, current_brk() - initial_brk);
  dismiss_brk();
}

void test_double_free() {
  printf("/*\n");
  printf(" * Verify if free doens't crash wheen freeing a free address\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc a block of size 1\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Try to free block again\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> DOUBLE FREE\n");

  void *p = memory_alloc(1);
  char expected_memory[17];
  memset(expected_memory, 0, 17);
  expected_memory[0] = 0;
  expected_memory[8] = 1;

  printf("------->> %sFirst free%s\n", UNDERLINE, DEFAULT);
  memory_free(p);
  compare_memory(expected_memory, p - 16, current_brk() - initial_brk);

  printf("------->> %sSecond free%s\n", UNDERLINE, DEFAULT);
  memory_free(p);
  compare_memory(expected_memory, p - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_alloc_same_block_after_free() {
  printf("/*\n");
  printf(" * Verify if block is reused after free\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc block of size 5\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc 5 bytes\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> ALLOC SAME BLOCK AFTER FREE\n");

  char expected_memory[21];
  memset(expected_memory, 0, 21);
  expected_memory[0] = 1;
  expected_memory[8] = 5;

  printf("------->> %sFirst alloc%s\n", UNDERLINE, DEFAULT);
  void *p1 = memory_alloc(5);
  void *brk1 = current_brk();

  printf("--------->> %sMemory after first alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("--------->> %sMemory after free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;
  memory_free(p1);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("------->> %sSecond alloc (after free)%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 1;
  void *p2 = memory_alloc(5);
  void *brk2 = current_brk();

  printf("--------->> %sSame pointer%s\n", UNDERLINE, DEFAULT);
  assert(p1, p2);

  printf("--------->> %sBrk didn't change%s\n", UNDERLINE, DEFAULT);
  assert(brk1, brk2);

  printf("--------->> %sMemory after second alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p2 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_alloc_after_free_dont_split() {
  printf("/*\n");
  printf(" * Verify if block isn't split if remaining size is too small\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc block of size 17\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc block of size 1\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> ALLOC AFTER FREE, DONT SPLIT\n");

  char expected_memory[33];
  memset(expected_memory, 0, 33);
  expected_memory[0] = 1;
  expected_memory[8] = 17;

  printf("------->> %sFirst alloc%s\n", UNDERLINE, DEFAULT);
  void *p1 = memory_alloc(17);
  void *brk1 = current_brk();

  printf("--------->> %sMemory after first alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("--------->> %sMemory after free%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 0;
  memory_free(p1);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("------->> %sSecond alloc (remaining space would be of 16 bytes)%s\n", UNDERLINE, DEFAULT);
  expected_memory[0] = 1;
  void *p2 = memory_alloc(1);
  void *brk2 = current_brk();

  printf("--------->> %sSame pointer%s\n", UNDERLINE, DEFAULT);
  assert(p1, p2);

  printf("--------->> %sBrk didn't change%s\n", UNDERLINE, DEFAULT);
  assert(brk1, brk2);

  printf("--------->> %sMemory after second alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_alloc_two_consecutives_blocks() {
  printf("/*\n");
  printf(" * Verify if new block is created when requesting more than one\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc a block of size 5\n");
  printf(" * 2. Alloc block of size 3\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> ALLOC TWO CONSECUTIVES BLOCKS\n");

  char expected_memory[40];
  memset(expected_memory, 0, 40);
  expected_memory[0] = 1;
  expected_memory[8] = 5;
  expected_memory[21] = 1;
  expected_memory[29] = 3;

  printf("------->> %sFirst alloc%s\n", UNDERLINE, DEFAULT);
  void *p1 = memory_alloc(5);

  printf("--------->> %sBrk after first alloc%s\n", UNDERLINE, DEFAULT);
  void *expected_brk = initial_brk + 21;
  assert(expected_brk, current_brk());

  printf("--------->> %sMemory after first alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("------->> %sSecond alloc%s\n", UNDERLINE, DEFAULT);
  void *p2 = memory_alloc(3);
  void *brk2 = current_brk();

  printf("--------->> %sBrk after first alloc%s\n", UNDERLINE, DEFAULT);
  expected_brk += 19;
  assert(expected_brk, brk2);

  printf("--------->> %sEnd of first block is the start of second%s\n", UNDERLINE, DEFAULT);
  assert(p1 + 5, p2 - 16);

  printf("--------->> %sMemory after second alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_three_consecutives_blocks_free_second() {
  printf("/*\n");
  printf(" * Verify if free only changes requested address\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc three blocks of size 1\n");
  printf(" * 2. Free the second block\n");
  printf(" *\n");
  printf(" */\n");

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
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("------->> %sMemory after free in second block%s\n", UNDERLINE, DEFAULT);
  memory_free(p2);
  expected_memory[17] = 0;
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_alloc_end_heap_after_free() {
  printf("/*\n");
  printf(" * Verify if new block isn't created wheen size is too small,\n");
  printf(" * a new block should be created at end of heap\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc a block of size 1\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc block of size 2\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> ALLOC END HEAP AFTER FREE\n");

  char expected_memory[35];
  memset(expected_memory, 0, 35);
  expected_memory[0] = 0;
  expected_memory[8] = 1;
  expected_memory[17] = 1;
  expected_memory[25] = 2;

  void *p1 = memory_alloc(1);
  printf("------->> %sBrk after first alloc%s\n", UNDERLINE, DEFAULT);
  void *brk1 = initial_brk + 17;
  assert(brk1, current_brk());
  
  printf("------->> %sBrk after free and second allocs%s\n", UNDERLINE, DEFAULT);
  memory_free(p1);
  void *p2 = memory_alloc(2);

  void *brk2 = brk1 + 18;
  assert(brk2, current_brk());

  printf("------->> %sSecond block start is at previous brk%s\n", UNDERLINE, DEFAULT);
  assert(brk1, p2 - 16);

  printf("------->> %sMemory after free and allocs%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_split_in_two_exact_size() {
  printf("/*\n");
  printf(" * Verify if two blocks are allocated inside a previous one\n");
  printf(" * that has been freed and take full size\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc block of size 18\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc two blocks of size 1\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> SPLIT IN TWO EXACT SIZE AFTER FREE\n");

  char expected_memory[34];
  memset(expected_memory, 0, 34);
  expected_memory[0] = 1;
  expected_memory[8] = 1;

  void *p1 = memory_alloc(18);
  void *brk = initial_brk + 34;
  memory_free(p1);

  printf("----->> %sFirst split%s\n", UNDERLINE, DEFAULT);
  printf("------->> %sFirst split is at start of block%s\n", UNDERLINE, DEFAULT);
  void *p2 = memory_alloc(1);
  assert(p1 - 16, p2 - 16);

  printf("------->> %sBrk didnt change after first split%s\n", UNDERLINE, DEFAULT);
  assert(brk, current_brk());

  printf("----->> %sSecond split%s\n", UNDERLINE, DEFAULT);
  printf("------->> %sSecond split is after first split%s\n", UNDERLINE, DEFAULT);
  void *p3 = memory_alloc(1);
  assert(p2 + 1, p3 - 16);

  printf("------->> %sBrk didnt change after second split%s\n", UNDERLINE, DEFAULT);
  assert(brk, current_brk());

  printf("------->> %sMemory after second split is allocated%s\n", UNDERLINE, DEFAULT);
  expected_memory[17] = 1;
  expected_memory[25] = 1;
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_split_in_two_size_is_not_enough() {
  printf("/*\n");
  printf(" * Verify if a block that doesn't fit in the remaining\n");
  printf(" * space of a free bigger block after another block is\n");
  printf(" * allocated is allocated at end of heap\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc block of size 18\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc block of size 1\n");
  printf(" * 4. Alloc block of size 2\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> SPLIT IN TWO AFTER FREE, SIZE IS NOT ENOUGH\n");

  char expected_memory[51];
  memset(expected_memory, 0, 51);
  expected_memory[0] = 1;
  expected_memory[8] = 1;
  expected_memory[17] = 0;
  expected_memory[25] = 1;
  expected_memory[34] = 1;
  expected_memory[42] = 2;

  void *p1 = memory_alloc(18);
  void *brk = initial_brk + 34;
  memory_free(p1);

  printf("----->> %sFirst split%s\n", UNDERLINE, DEFAULT);
  printf("------->> %sFirst split is at start of block%s\n", UNDERLINE, DEFAULT);
  void *p2 = memory_alloc(1);
  assert(p1 - 16, p2 - 16);

  printf("------->> %sBrk didnt change after first split%s\n", UNDERLINE, DEFAULT);
  assert(brk, current_brk());

  printf("----->> %sSecond block%s\n", UNDERLINE, DEFAULT);
  printf("------->> %sSecond block is after previous brk%s\n", UNDERLINE, DEFAULT);
  void *p3 = memory_alloc(2);
  assert(brk, p3 - 16);

  printf("------->> %sBrk after second block%s\n", UNDERLINE, DEFAULT);
  brk += 18;
  assert(brk, current_brk());

  printf("------->> %sMemory after second block is allocated%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_split_in_two_size_fill() {
  printf("/*\n");
  printf(" * Verify if a block is extended to take the free space in a free\n");
  printf(" * block and the next block is allocated at end of heap\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc block of size 17\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc two blocks of size 1\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> ALLOC AFTER FREE, EXTEND FIRST BLOCK\n");

  char expected_memory[49];
  memset(expected_memory, 0, 49);
  expected_memory[0] = 1;
  expected_memory[8] = 17;
  expected_memory[33] = 1;
  expected_memory[41] = 1;

  void *p1 = memory_alloc(17);
  memory_free(p1);

  printf("----->> %sMemory after first alloc after free%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("----->> %sMemory after second alloc after free%s\n", UNDERLINE, DEFAULT);
  memory_alloc(1);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_search_until_free() {
  printf("/*\n");
  printf(" * Search between allocated blocks until one is free\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc 5 blocks of size 0\n");
  printf(" * 2. Free last block\n");
  printf(" * 3. Alloc block of size 0\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> SEARCH FOR FREE BLOCK\n");

  char expected_memory[80];
  memset(expected_memory, 0, 80);
  expected_memory[0] = 1;         // With size 0 each block is 16 byts appart
  expected_memory[16] = 1;
  expected_memory[32] = 1;
  expected_memory[48] = 1;

  void *p1 = memory_alloc(0);
  memory_alloc(0);
  memory_alloc(0);
  memory_alloc(0);
  void *p2 = memory_alloc(0);
  memory_free(p2);

  printf("----->> %sMemory after initial allocs and free%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("----->> %sMemory after searching for free block%s\n", UNDERLINE, DEFAULT);
  expected_memory[64] = 1;
  memory_alloc(0);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_search_until_fit() {
  printf("/*\n");
  printf(" * Search between free blocks until one that fits\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc blocks of size [0..3] \n");
  printf(" * 2. Free blocks\n");
  printf(" * 3. Alloc block of size 3\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> SEARCH FOR BLOCK THAT FITS\n");

  char expected_memory[70];
  memset(expected_memory, 0, 70);
  expected_memory[24] = 1;
  expected_memory[41] = 2;
  expected_memory[59] = 3;

  void *p1 = memory_alloc(0);
  memory_free(p1);

  void *p2 = memory_alloc(1);
  memory_free(p2);

  p2 = memory_alloc(2);
  memory_free(p2);

  p2 = memory_alloc(3);
  memory_free(p2);

  printf("----->> %sMemory after initial allocs and free%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  printf("----->> %sMemory after searching for block that fits%s\n", UNDERLINE, DEFAULT);
  expected_memory[51] = 1;
  memory_alloc(3);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_alloc_first_fit() {
  printf("/*\n");
  printf(" * Alloc new block in the first one that fits\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc two blocks of size 3\n");
  printf(" * 2. Free blocks\n");
  printf(" * 3. Alloc block of size 3\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> ALLOC IN FIRST FIT\n");

  char expected_memory[38];
  memset(expected_memory, 0, 38);
  expected_memory[0] = 1;
  expected_memory[8] = 3;
  expected_memory[27] = 3;

  void *p1 = memory_alloc(3);
  void *p2 = memory_alloc(3);

  memory_free(p1);
  memory_free(p2);

  memory_alloc(3);

  printf("----->> %sMemory after final alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_alloc_first_fit_extend() {
  printf("/*\n");
  printf(" * First fit is bigger than request\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc one block of size 4\n");
  printf(" * 1. Alloc one block of size 1\n");
  printf(" * 2. Free blocks\n");
  printf(" * 3. Alloc block of size 1\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> EXTEND IN FIRST FIT\n");

  char expected_memory[37];
  memset(expected_memory, 0, 37);
  expected_memory[0] = 1;
  expected_memory[8] = 4;
  expected_memory[28] = 1;

  void *p1 = memory_alloc(4);
  void *p2 = memory_alloc(1);

  memory_free(p1);
  memory_free(p2);

  memory_alloc(1);

  printf("----->> %sMemory after final alloc%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_multiple_splits() {
  printf("/*\n");
  printf(" * Alloc a block and split it in multiple parts\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc a block of size 100\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc blocks of size 1\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> SPLIT BLOCK IN MULTIPLE PARTS\n");

  char expected_memory[116];
  memset(expected_memory, 0, 116);

  void *p1 = memory_alloc(100);
  memory_free(p1);

  int i;
  for ( i = 0; i < 6; i++ ) {
    expected_memory[i * 17] = 1;
    expected_memory[(i * 17) + 8] = 1;
    memory_alloc(1);
  }

  expected_memory[((i - 1) * 17) + 8] = 15;   // Change last block size

  printf("----->> %sMemory after splits%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

void test_split_multiple_blocks() {
  printf("/*\n");
  printf(" * Alloc at end of heap and split mixed\n");
  printf(" *\n");
  printf(" * Steps:\n");
  printf(" * 1. Alloc a block of size 18\n");
  printf(" * 2. Free block\n");
  printf(" * 3. Alloc a block of size 1\n");
  printf(" * 4. Alloc a block of size 24\n");
  printf(" * 5. Free block\n");
  printf(" * 6. Alloc a block of size 2\n");
  printf(" * 7. Alloc a block of size 1\n");
  printf(" * 8. Alloc a block of size 26\n");
  printf(" * 9. Free block\n");
  printf(" * 10. Alloc a block of size 5\n");
  printf(" * 11. Alloc a block of size 4\n");
  printf(" * 12. Alloc a block of size 3\n");
  printf(" *\n");
  printf(" */\n");

  printf("=====>> SPLIT BLOCK IN MULTIPLE PARTS\n");

  char expected_memory[112];
  memset(expected_memory, 0, 112);

  void *p1 = memory_alloc(18);
  memory_free(p1);
  memory_alloc(1);
  void *p2 = memory_alloc(21);
  memory_free(p2);
  memory_alloc(2);
  memory_alloc(1);
  p2 = memory_alloc(25);
  memory_free(p2);
  memory_alloc(5);
  memory_alloc(4);
  memory_alloc(3);

  expected_memory[0] = 1;
  expected_memory[8] = 1;
  expected_memory[17] = 1;
  expected_memory[25] = 1;
  expected_memory[34] = 1;
  expected_memory[42] = 2;
  expected_memory[52] = 1;
  expected_memory[60] = 3;
  expected_memory[71] = 1;
  expected_memory[79] = 5;
  expected_memory[92] = 1;
  expected_memory[100] = 4;

  printf("----->> %sMemory after splits and allocs%s\n", UNDERLINE, DEFAULT);
  compare_memory(expected_memory, p1 - 16, current_brk() - initial_brk);

  dismiss_brk();
}

int main() {
  printf("===================== TESTS =====================\n");
  test_setup_brk();
  test_alloc_and_free_0_bytes();
  test_alloc_and_free_100_bytes();
  test_alloc_two_consecutives_blocks();

  test_double_free();
  test_free_address_not_allowed();
  test_alloc_same_block_after_free();
  test_alloc_end_heap_after_free();
  test_search_until_free();
  test_search_until_fit();

  test_alloc_after_free_dont_split();
  test_three_consecutives_blocks_free_second();
  test_split_in_two_exact_size();
  test_split_in_two_size_is_not_enough();
  test_split_in_two_size_fill();
  test_multiple_splits();
  test_split_multiple_blocks();

  test_alloc_first_fit();
  test_alloc_first_fit_extend();

  total();
  return 0;
}
