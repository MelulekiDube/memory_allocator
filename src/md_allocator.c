/*
	My own personal implementation of malloc realloc and calloc	
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

// lock
pthread_mutex_t global_malloc_lock;

#define INVALID_PTR (void*) -1
#define lock() pthread_mutex_lock(&global_malloc_lock)
#define unlock() pthread_mutex_unlock(&global_malloc_lock)
#define inbetween_blocks(block, current) (block > current && block < current->next)
#define MIN_SIZE 1024
#define DEBUG

typedef long Align;

typedef union block{
	struct {
		union block *next_block, *prev_block;
		size_t size;
	} s;
	Align x; // force alignment of blocks
} Block;

#define BLOCKSIZE sizeof(Block)
Block *addmoreBlocks(size_t);
Block *get_first_fit_block(size_t);
Block *fragment_block(Block*, size_t);
void coalesce(Block*, Block*); //combines these two blocks given into a single block to avoid fragmentation

Block *head, *tail;

void *md_malloc(size_t numbytes){
	if(!numbytes) return NULL;
	
	//declare variables to be used in this function
	Block *header; //this is the selected block to be returned to the user;
	
	lock(); // we are entering a critical section
	header = get_first_fit_block(numbytes); // we outsource the use a seperate method for finding the block
	
	// if we get a valid block we will return that block
	if(header){
		unlock();
		return (void*) header+1; // we return +1 because when we ask the os for block we include the size of the block. see below
	}	
	
	// add this block to the list of blocks we have
	header = addmoreBlocks(numbytes);
	if(!header){
		unlock();
		return NULL;
	}
	
	unlock();
	return (void*) (header+1);
}

/* requests more blocks from the os. The requested blocks may be more than what is requested*/
Block *addmoreBlocks(size_t size){
	
	size_t total_size; // this is the total_size of the block that we want. its gonna be numbytes+blocksize;
	void *block; // this is going to be used for additional space if neededd
	Block *header;
	

	// if we are out of blocks or we dont have blocks of this size then ask for more
	total_size = size + BLOCKSIZE; // the block size for storing the header
	if(!head && total_size < MIN_SIZE)
		total_size = MIN_SIZE;
	block = sbrk(total_size); //sbrk will return the old ending address of the data segment
	if(block == INVALID_PTR)
		return NULL;
	#ifdef DEBUG
	printf("Block address returned is %p\n", block);
	#endif
	
	header = block;
	header->s.size = size;
	header->s.next_block = NULL;
	header->s.prev_block = NULL;
	
	
	
	if(!head)
		head = header;
	if(tail){
		tail->s.next_block = header;
		header->s.prev_block = tail;
	}
	tail = header;
	
	
	return header;
}

/* this will find the first block that can fit the requirements and is not being used currently from the list of blocksize*/
Block *get_first_fit_block(size_t size){
	Block *current_block = head;
	while(current_block){
		// if the current bloc is not being used and the size is big enough to be used.
		if (current_block->s.size >= size){
			
			if(current_block->s.size > size)
				current_block = fragment_block(current_block, size);
			
			Block *prev = current_block->s.prev_block;
			Block *next = current_block->s.next_block;
			
			// make next of prev point to the next of current and the prev of next of currrent point to prev of current
			prev->s.next_block = next;
			next->s.prev_block = prev;
			
			//make next of current and prev of currrent point to NULL
			current_block->s.next_block = NULL;
			current_block->s.prev_block = NULL;
			
			if(current_block==head) head = next;
			if(current_block==tail) tail = prev;
			return current_block;
		
		}
		current_block = current_block->s.next_block;
	}
	return NULL;
}

/*
	1) we check if block.size - size > BLOCKSIZE we will divide this memory location
	 if not we will not and just return the block that was returned to us

*/
Block *fragment_block(Block *block, size_t size){
	size_t currentblock_size = block->s.size;
	size_t residual_size = currentblock_size - size;
	if(residual_size > BLOCKSIZE){
		//break the block up
		Block *nextBlock = block+size;
		
		nextBlock->s.size = residual_size;
		//
		// set the prev and next pointer as appropriate
		nextBlock->s.next_block = block->s.next_block;
		nextBlock->s.prev_block = block;
		block->s.next_block->s.prev_block = nextBlock;
		
		block->s.next_block = nextBlock;
	}
	return block;
}

void md_free(void *ptr){
	Block *block;
	Block *pblock;
	Block *prev;
	Block *current;

	block = ptr;
	pblock = block-1;
	prev = NULL;
	for(current = head; current && inbetween_blocks(pblock, current); current = current->s.next_block)
		
}

// still need to do error checks for this method
void coalesce(Block* block1, Block* block2){
	size_t total_size = block1->s.size + block2->s.size+BLOCKSIZE;
	block1->s.size = total_size;
	block1->s.next_block = block2->s.next_block;
	block2->s.next_block = NULL;
}


int main(){
	int *a = md_malloc(4);
	void *endptr = sbrk(0);
	*a = 5;
	printf("A is a pointer to address: %p and the value in that location is %d\n", a, *a);
	printf("End is pointing to the ad: %p\n",  endptr);
	return 0;
}