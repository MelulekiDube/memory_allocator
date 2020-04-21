#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifndef ALLOCATE_BYTES
#define ALLOCATE_BYTES 0x1000000
#endif /* #ifndef ALLOCATE_BYTES */

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
		
int main()
{
    void *init_ptr = malloc((size_t) 1);
    void *end_ptr = sbrk((intptr_t) 0);

    printf("\nmalloc returned: %p\n", init_ptr);
    printf("sbrk returned: %p\n", end_ptr);

	printf("0>1 : %i, end_ptr > init_ptr: %i\n", (0>1), (end_ptr > init_ptr));
	
	void* heap_used = end_ptr - init_ptr;
    printf(" Total Heap space = %p B = %p kB  \n", 
    heap_used,
    heap_used);
    return 0;
 }