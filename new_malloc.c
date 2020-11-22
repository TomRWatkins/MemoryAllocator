#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Block Constants */

#define ALIGNMENT       (sizeof(double))
#define ALIGN(size)     (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/* Block Macros */

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


/* Free List Global Variable */

Block FreeList = {-1, -1, &FreeList, &FreeList}; //USE AN ARRAY OF THESE
												 //Block FreeLists[10] // FreeList[i] = {-1, -1, &FreeList, &FreeList};


void printFreeList() {
    for (Block *curr = FreeList.next; curr != &FreeList; curr = curr->next) {
    	printf("%p - %d |  ",curr, curr->size);
 	}
}

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
    myFree(block->data);
}

Block * free_list_search(size_t size) {
    /* Perform best fit algorithm */    
    int found = 0;
    Block *smallest = FreeList.next;

    for (Block *curr = FreeList.next; curr != &FreeList; curr = curr->next) {                
        if (curr->capacity - sizeof(Block) >= size) {   
        	found = 1;
        	if(curr->capacity < smallest->capacity) {
        		smallest = curr;
        	}            
        }
    }
    
    if(found) {
    	if((smallest->size - size <= (32 + sizeof(Block)))) {
    		//Send whole block
    		printf("Found block of size (<32 Left) %zu at address %pu\n",smallest->size, smallest);    		
    		return smallest;
    	}
    	else {
    		//Split the block ASSIGN BLOCK TO ALLOCATE ON THE LEFT  
    		int oldCapac = smallest->capacity;
    		int oldSize = smallest->size;

    		Block *split = ((char *) smallest + sizeof(Block)) + size; 
    		
    		smallest->capacity = sizeof(Block) + size;
    		smallest->size = size;

    		
    		split->capacity = oldCapac - smallest->capacity;
    		split->size = split->capacity - sizeof(Block);
    		addFree(split->data);

    		printf("Found block of size %zu at address %pu\n",smallest->size, smallest);
    		return smallest;
    	}
    }
    return NULL;
}

void    free_list_insert(Block *block) {
    /* Append to head */
    Block *head = FreeList.next;
    head->prev = block;
    FreeList.next = block;
    block->next = head;
    block->prev = &FreeList;
}

void *myMalloc(size_t size) {
	if (!size) {
        return NULL;
    }
    if(!(size % 8 == 0)) {
       size = ((size + 7) & (-8)); 
    }
    // Search free list for any available block with matching size
    Block *block = free_list_search(size);


    if (!block) {
        block_allocate();    
   		block = free_list_search(size);
    }

    block = block_detach(block);
    

    if (!block) {
        return NULL;
    }
    printf("Address being returned is %p\n",block->data);
    return block->data;
}

void addFree(void *ptr) {
	 if (!ptr) {
        return;
    }

    Block *block = BLOCK_FROM_POINTER(ptr);
    free_list_insert(block);

}

void myFree(void *ptr) {
	 if (!ptr) {
        return;
    }

    Block *block = BLOCK_FROM_POINTER(ptr);
    free_list_insert(block);
    
    coalesce();
    

}

void coalesce() {
	Block *curr = FreeList.next;
   	Block *next = curr->next;
   	if(next != &FreeList) {   		
   		curr->next = next->next;
   		curr->size = curr->size + next->size + sizeof(Block);
    	curr->capacity = curr->capacity + next->capacity;
    	
    	block_detach(next);

    	
    }
    
}



int main() {
	char word[10];
	char newWord[10] = "Filler";

	while(1) {
		printf("Enter A/F: ");
		scanf("%s", word);
		if(word[0] == 'A'){     
			printf("Enter amount of memory to allocate: ");
			scanf("%s", word);			   
      		int x = atoi(word);
      		int *q = (int*)myMalloc(x);
      		*q = 100;
      		printf("Allocated - %p\n", q);
		}
		else{
			printf("Enter address to free: ");
			int *ptr;
			scanf("%p", &ptr);
      		
      		myFree(ptr);
    	}
		printFreeList();
	}

	//int *p = (int*) myMalloc(100);
	//*p = 11;	
	
	//int *a = (int*) myMalloc(105);
	//*a = 11;	
	
	//int *z = (int*) myMalloc(105);
	//*z = 11;	
	
	//int *x = (int*) myMalloc(105);
	//*x = 11;	
	
	//myFree(p);myFree(a);myFree(z);myFree(x);

	//printFreeList();
}


