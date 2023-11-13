/*
 * Get initial brk address
 */
void setup_brk();

/*
 * Restore brk for it's initial address
 */
void dismiss_brk();

/*
* Search for a free block with at least 'bytes' size
* If block is found, flag it as ocupied and return the address
* else, open space in heap for a new block
*/
void *memory_alloc(unsigned long int bytes); 

/*
* Flag a ocupied block as free
*/
int memory_free(void *pointer);

/*
 * Each block has the following struct:
 *
 * 0               1               2  
 * 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Flag     |      Size     |          Payload(...)         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
