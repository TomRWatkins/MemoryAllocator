#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
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


/* Free List Global Variables and binIndex */
static int binIndex;

Block FreeList0 = {-1, -1, &FreeList0, &FreeList0}; //32
Block FreeList1 = {-1, -1, &FreeList1, &FreeList1}; //64
Block FreeList2 = {-1, -1, &FreeList2, &FreeList2}; //128
Block FreeList3 = {-1, -1, &FreeList3, &FreeList3}; //256
Block FreeList4 = {-1, -1, &FreeList4, &FreeList4}; //512
Block FreeList5 = {-1, -1, &FreeList5, &FreeList5}; //1024
Block FreeList6 = {-1, -1, &FreeList6, &FreeList6}; //2048
Block FreeList7 = {-1, -1, &FreeList7, &FreeList7}; //4096
Block FreeList8 = {-1, -1, &FreeList8, &FreeList8}; //8192
 

void printFreeList() {
    for(int i = 0; i < 9; i++) {
        printf("Bin[%d] - ", i);
        switch(i) {            
            case 0: for (Block *curr = FreeList0.next; curr != &FreeList0; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 1: for (Block *curr = FreeList1.next; curr != &FreeList1; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 2: for (Block *curr = FreeList2.next; curr != &FreeList2; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 3: for (Block *curr = FreeList3.next; curr != &FreeList3; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 4: for (Block *curr = FreeList4.next; curr != &FreeList4; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 5: for (Block *curr = FreeList5.next; curr != &FreeList5; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 6: for (Block *curr = FreeList6.next; curr != &FreeList6; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 7: for (Block *curr = FreeList7.next; curr != &FreeList7; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;
            case 8: for (Block *curr = FreeList8.next; curr != &FreeList8; curr = curr->next) {
                        printf("%p - %d |  ",curr, curr->size);
                    }
                    printf("\n"); break;               
        }        
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
    addFree(block->data);
    coalesce();
}

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
    switch(binIndex) {
        case 0: smallest = FreeList0.next; break;
        case 1: smallest = FreeList1.next; break;
        case 2: smallest = FreeList2.next; break;
        case 3: smallest = FreeList3.next; break;
        case 4: smallest = FreeList4.next; break;
        case 5: smallest = FreeList5.next; break;
        case 6: smallest = FreeList6.next; break;
        case 7: smallest = FreeList7.next; break;
        case 8: smallest = FreeList8.next; break;
    }
    
    switch(binIndex) {
        case 0: 
        for (Block *curr = FreeList0.next; curr != &FreeList0; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 1: 
        for (Block *curr = FreeList1.next; curr != &FreeList1; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 2: 
        for (Block *curr = FreeList2.next; curr != &FreeList2; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 3: 
        for (Block *curr = FreeList3.next; curr != &FreeList3; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 4: 
        for (Block *curr = FreeList4.next; curr != &FreeList4; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 5: 
        for (Block *curr = FreeList5.next; curr != &FreeList5; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 6: 
        for (Block *curr = FreeList6.next; curr != &FreeList6; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 7: 
        for (Block *curr = FreeList7.next; curr != &FreeList7; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;
        case 8: 
        for (Block *curr = FreeList8.next; curr != &FreeList8; curr = curr->next) {                
            if (curr->capacity - sizeof(Block) >= size) {   
                found = 1;
                if(curr->capacity < smallest->capacity) {
                    smallest = curr;
                }            
            }
        } break;

    }
    
    
    if(found) {    	   		
        if((smallest->size == size)) {
            //Send whole block
            printf("Found block of size %zu at address",smallest->size);          
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

        printf("Found block of size %zu at address",smallest->size);
            
    	return smallest;    
    }
    return NULL;
}

void    free_list_insert(Block *block) {	
    /* Append to head */
   Block *head;
    switch(binIndex) {
        case 0: head = FreeList0.next; break;
        case 1: head = FreeList1.next; break;
        case 2: head = FreeList2.next; break;
        case 3: head = FreeList3.next; break;
        case 4: head = FreeList4.next; break;
        case 5: head = FreeList5.next; break;
        case 6: head = FreeList6.next; break;
        case 7: head = FreeList7.next; break;
        case 8: head = FreeList8.next; break;
    }    
    head->prev = block;
    switch(binIndex) {
        case 0: FreeList0.next = block; break;
        case 1: FreeList1.next = block; break;
        case 2: FreeList2.next = block; break;
        case 3: FreeList3.next = block; break;
        case 4: FreeList4.next = block; break;
        case 5: FreeList5.next = block; break;
        case 6: FreeList6.next = block; break;
        case 7: FreeList7.next = block; break;
        case 8: FreeList8.next = block; break;
    }    
    
    block->next = head;
    switch(binIndex) {
        case 0: block->prev = &FreeList0; break;
        case 1: block->prev = &FreeList1; break;
        case 2: block->prev = &FreeList2; break;
        case 3: block->prev = &FreeList3; break;
        case 4: block->prev = &FreeList4; break;
        case 5: block->prev = &FreeList5; break;
        case 6: block->prev = &FreeList6; break;
        case 7: block->prev = &FreeList7; break;
        case 8: block->prev = &FreeList8; break;
    } 
}

void *myMalloc(size_t size) {      
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

void coalesce() {
    Block *curr;
    Block *next;
    switch(binIndex) {
    case 0: curr = FreeList0.next;
            next = curr->next;
            if(next != &FreeList0) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break; 
    case 1: curr = FreeList1.next;
            next = curr->next;
            if(next != &FreeList1) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;    
    case 2: curr = FreeList2.next;
            next = curr->next;
            if(next != &FreeList2) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;
    case 3: curr = FreeList3.next;
            next = curr->next;
            if(next != &FreeList3) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;
    case 4: curr = FreeList4.next;
            next = curr->next;
            if(next != &FreeList4) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;
    case 5: curr = FreeList5.next;
            next = curr->next;
            if(next != &FreeList5) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;
    case 6: curr = FreeList6.next;
            next = curr->next;
            if(next != &FreeList6) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;
    case 7: curr = FreeList7.next;
            next = curr->next;
            if(next != &FreeList7) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;
    case 8: curr = FreeList8.next;
            next = curr->next;
            if(next != &FreeList8) {        
                curr->next = next->next;
                curr->size = curr->size + next->size + sizeof(Block);
                curr->capacity = curr->capacity + next->capacity;       
                block_detach(next);     
            } break;
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


