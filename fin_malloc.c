#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

/* Block Constants for 8 Byte allignment */
#define ALIGNMENT       (sizeof(double))
#define ALIGN(size)     (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/* Convert Pointer to address of Block */
#define BLOCK_FROM_POINTER(ptr) \
    (Block *)((intptr_t)(ptr) - sizeof(Block))

/* Block Structure */
typedef struct block Block;
struct block {
    size_t   capacity;  /* Number of bytes allocated to block */
    size_t   size;      /* Number of bytes used by block */
    Block *  prev;      /* Pointer to previous block structure */
    Block *  next;      /* Pointer to next block structure */
    char     data[];    /* Label for user accessible block data */
};

void myFree(void *);
void free_list_insert(Block *);
void addFree(void *);
void coalesce();
void init();


/* Free List Global Variables and binIndex */
static int binIndex;
int initInt = 0;
Block FreeLists[9]; //Array of doubly linked freelists
 

//Function to print each Bins Free List and display the total memory. 
void printFreeList() {
    for(int i = 0; i < 9; i++) {
    	int total = 0;
        printf("Bin[%d] - ", i);
        for (Block *curr = FreeLists[i].next; curr != &FreeLists[i]; curr = curr->next) {
            printf("%p - %d |  ",curr, curr->size);
            total += curr->size;
        }  

        printf(" - Total memory %d\n", total);
    }
}

//Function to detatch a given block from its FreeList
Block * block_detach(Block *block) {
    /* Update prev and next blocks to reference each other */
    if (block) {
        Block *prev = block->prev;
        Block *next = block->next;
        prev->next  = block->next;
        next->prev  = block->prev;
        block->next = block;
        block->prev = block;
    }
    return block;
}

//Function called when there isn't sufficient memory in a given Bins FreeList, the function
//calls sbrk() to allocate another 8K block to the front of the List.
Block block_allocate() {  
	intptr_t allocated = sizeof(Block) + (8192);  
    Block *  block     = sbrk(allocated);
    if (block == -1) {
        printf("No Memory\n");
    }

    block->capacity = allocated;
    block->size     = 8192;
    block->prev     = block;
    block->next     = block;
    addFree(block->data);
    coalesce();
}

//A Function to search a given Free List for the smallest block able to satisfy a request,
//the block is then split and returned to the application, the remainder of the block is reallocated 
//on the FreeList.
Block * free_list_search(size_t size) {
    /* Perform best fit algorithm */    
    int found = 0;

    if (size <= 32) binIndex = 0; 
    else if (size <= 64) binIndex = 1; 
    else if (size <= 128) binIndex = 2; 
    else if (size <= 256) binIndex = 3; 
    else if (size <= 512) binIndex = 4; 
    else if (size <= 1024) binIndex = 5;
    else if (size <= 2048) binIndex = 6;
    else if (size <= 4096) binIndex = 7;
    else if (size <= 8192) binIndex = 8;        
    else binIndex = 8;

    size = pow(2, (binIndex+5));
    
    Block *smallest;
    smallest = FreeLists[binIndex].next; 
    
    for (Block *curr = FreeLists[binIndex].next; curr != &FreeLists[binIndex]; curr = curr->next) {                
        if (curr->capacity - sizeof(Block) >= size) {   
            found = 1;
            if(curr->capacity < smallest->capacity) {
                smallest = curr;
            }            
        }
    } 
    
    if(found) {    	   		
        if((smallest->size == size)) {
            //Send whole block
            printf("Found block of size %zu at address", smallest->size);          
            return smallest;
        }
        //Split the block ASSIGN BLOCK TO ALLOCATE ON THE LEFT  
        int oldCapac = smallest->capacity;
        int oldSize = smallest->size;

        Block *split = ((char *) smallest + sizeof(Block)) + size; 
            
        smallest->capacity = sizeof(Block) + size;
        smallest->size = size;

            
        split->capacity = oldCapac - smallest->capacity;
        split->size = split->capacity - sizeof(Block);
        addFree(split->data);

        printf("Found block of size %zu at address", smallest->size);
            
    	return smallest;    
    }
    return NULL;
}

//A Function to insert a given block at the head of a FreeList
void free_list_insert(Block *block) {	
    /* Append to head */
    Block *head;
    head = FreeLists[binIndex].next;
    head->prev = block;
    FreeLists[binIndex].next = block;
    block->next = head;
    block->prev = &FreeLists[binIndex];
}

//A Function to initialise the Bins of FreeListss
void init() {
	for(int i = 0; i < 9; i++) {
		FreeLists[i].capacity = -1;
		FreeLists[i].size = -1;
		FreeLists[i].prev = &FreeLists[i];
		FreeLists[i].next = &FreeLists[i];		
	}
}

//A Function that allocates the block of memory, returning a pointer, of a given size request
void *myMalloc(size_t size) {
	if(!initInt)  {
		initInt = 1;
		init();
	}      

	if (!size) {
        return NULL;
    }

    if(!(size % 8 == 0)) {
       size = ((size + 7) & (-8)); 
    }

    // Search free list for any smallest block with matching size
    Block *block = free_list_search(size);


    if (!block) {
        block_allocate();    
   		block = free_list_search(size);
    }

    block = block_detach(block);
    

    if (!block) {
        return NULL;
    }

    //printf("Address being returned is %p\n",block->data);
    return block->data;
}

//A helper function to add a Block to the FreeList given a pointer
void addFree(void *ptr) {
	 if (!ptr) {
        return;
    }

    Block *block = BLOCK_FROM_POINTER(ptr);
    free_list_insert(block);

}

//A  function to add a Block to the FreeList given a pointer and coalesce the adjacent Free Blocks
void myFree(void *ptr) {
	 if (!ptr) {
        return;
    }

    Block *block = BLOCK_FROM_POINTER(ptr);
    int x = block->size;
    
    if (x <= 32) binIndex = 0; 
    else if (x <= 64) binIndex = 1; 
    else if (x <= 128) binIndex = 2; 
    else if (x <= 256) binIndex = 3; 
    else if (x <= 512) binIndex = 4; 
    else if (x <= 1024) binIndex = 5;
    else if (x <= 2048) binIndex = 6;
    else if (x <= 4096) binIndex = 7;
    else if (x <= 8192) binIndex = 8;        
    else binIndex = 8;

    free_list_insert(block);
    
    coalesce();
}

//A function that coalsces adjacent Free Blocks in memory
void coalesce() {    
	//Fix coalescing
    for (Block *curr = FreeLists[binIndex].next; curr != &FreeLists[binIndex]; curr = curr->next) {
    	Block *next; 
    	Block *prev; 	
    	next = curr->next;
    	prev = curr->prev;

    	//printf("\n%d\n", ((next-curr)*32));
    	if(((next-curr)*32) == curr->capacity) {     		
        	curr->next = next->next;
        	curr->size = curr->size + next->size + sizeof(Block);
        	curr->capacity = curr->capacity + next->capacity;       
        	block_detach(next);     
    	} 	 
    }
}


int main() {
	char word[10];
	char newWord[10] = "Filler";

	while(1) {
        printf("\n");
		printf("Enter A/F: ");
		scanf("%s", word);
		if(word[0] == 'A'){     
			printf("Enter amount of memory to allocate: ");
			scanf("%s", word);			   
      		int x = atoi(word);
      		int *q = (int*)myMalloc(x);
      		*q = 100;
      		printf(" %p\n", q);
		}
		else{
			printf("Enter address to free: ");
			int *ptr;
			scanf("%p", &ptr);
      		
      		myFree(ptr);
    	}
        printf("\n");
		printFreeList();
	}
}


