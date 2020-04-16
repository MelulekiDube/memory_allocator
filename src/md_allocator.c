/*
	My own personal implementation of malloc realloc and calloc	
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define INVALID_PTR (void*) -1
#define lock() 1
#define unlock() 0
#define MIN_SIZE 1024
#define DEBUG

typedef long Align;

typedef union block{
	struct {
		union block *next_block;
		size_t size;
		int status; // tell whether the block is empty or not 
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
		header->s.status = 1; // set the block to being used
		unlock();
		return (void*) header+1; // we return +1 because when we ask the os for block we include the size of the block. see below
	}	
	
	// add this block to the list of blocks we have
	header = addmoreBlocks(numbytes);
	header->s.size = numbytes+BLOCKSIZE;
	header->s.next_block = NULL;
	header->s.status = 1;
	if(!head)
		head = header;
	if(tail)
		tail->s.next_block = header;
	tail = header;
	
	unlock();
	return (void*) (header+1);
}

/* requests more blocks from the os. The requested blocks may be more than what is requested*/
Block *addmoreBlocks(size_t size){
	
	size_t total_size; // this is the total_size of the block that we want. its gonna be numbytes+blocksize;
	void *block; // this is going to be used for additional space if neededd
	
	// if we are out of blocks or we dont have blocks of this size then ask for more
	total_size = size + BLOCKSIZE; // the block size for storing the header
	if(total_size < MIN_SIZE)
		total_size = MIN_SIZE;
	block = sbrk(total_size); //sbrk will return the old ending address of the data segment
	if(block == INVALID_PTR) {
		unlock();
		return NULL;
	}
	#ifdef DEBUG
	printf("Block address returned is %p\n", block);
	#endif
	return block;
}

/* this will find the first block that can fit the requirements and is not being used currently from the list of blocksize*/
Block *get_first_fit_block(size_t size){
	Block *current_block = head;
	while(current_block){
		// if the current bloc is not being used and the size is big enough to be used.
		if(!current_block->s.status){
			if (current_block->s.size == size)
				return current_block;
			if(current_block->s.size > size)
				return fragment_block(size);
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
		
		//
		nextBlock->s.size = residual_size;
		nextBlock->s.next_block = block->s.next_block;
		nextBlock>s.status = 0;
		block->s.next = nextBlock;
	}
	return block;
}

void md_free(void *ptr){
	Block *block;
	Block *pblock;


	block = ptr;
	pblock = block-1;
	
	pblock->s.status = 0;
	if(!pblock->s.next_block->s.status){
		coalesce(pblock, pblock->s.next_block); 
	}
}

// still need to do error checks for this method
void coalesce(Block* block1, Block* block2){
	size_t total_size = block1->s.size + block2->s.size+BLOCKSIZE;
	block1->s.size = total_size;
	block1->s.next_block = block2->s.next_block;
	block2->s.next_block = null;
}


int main(){
	int *a = md_malloc(4);
	void *endptr = sbrk(0);
	*a = 5;
	printf("A is a pointer to address: %p and the value in that location is %d\n", a, *a);
	printf("End is pointing to the ad: %p\n",  endptr);
	return 0;
}