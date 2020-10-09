#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>

#include <stdbool.h>

void * alloc_worst(size_t requested);

/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct node
{
  // doubly-linked list
  struct node *prev;
  struct node *next;

  int  bytes_count;   // How many bytes in this block?
  char is_free;        // 1 if this block is allocated,
                       // 0 if this block is free.
  void *data_ptr;           // location of block in memory pool.
};

strategies _strategy = 0;    // Current strategy 

size_t _main_mem_size;
void * _main_mem = NULL;

static struct node *_head;
static struct node *_next;

void * alloc_worst(size_t req_size){
	//void *a;
	//a = mymalloc(100);
	return NULL;
}

void given_block_bigger_than_max_requested_return_null(){
	void *a;
	strategies strat = Worst;		
	initmem(strat,500); //free old mamory if any and alocate new main memory block

	//act
	a = mymalloc(501);

	//assert
	assert( a==NULL && "My first unit test in c" );
    
	free(_main_mem);
}

int main(){
	given_block_bigger_than_max_requested_return_null();
}

/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t size)
{
	_strategy = strategy;

	/* all implementations will need an actual block of memory to use */
	_main_mem_size = size;

	if (_main_mem != NULL) free(_main_mem); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */


	_main_mem = malloc(size);
	
	/* TODO: Initialize memory management structure. */


}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */

void *mymalloc(size_t requested)
{
	assert((int)_strategy > 0);
	
	switch (_strategy)
	  {
	  case NotSet: 
	            return NULL;
	  case First:
	            return NULL;
	  case Best:
	            return NULL;
	  case Worst:
	            return alloc_worst(requested);
	  case Next:
	            return NULL;
	  }
	return NULL;
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{
	return;
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when refered to "memory" here, it is meant that the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
	return 0;
}

/* Get the number of bytes allocated */
int mem_allocated()
{
	return 0;
}

/* Number of non-allocated bytes */
int mem_free()
{
	return 0;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
	return 0;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	return 0;
}       

char mem_is_alloc(void *ptr)
{
        return 0;
}

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may find them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
	return _main_mem;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return _main_mem_size;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	return;
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */ 
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
    void *a, *b, *c, *d, *e;
	strategies strat = argc > 1 ? strategyFromString(argv[1]) : First; 
		
	/* A simple example.  
	   Each algorithm should produce a different layout. */
	
	initmem(strat,500); //free old mamory if any and alocate new main memory block
	
	a = mymalloc(100);
	b = mymalloc(100);
	c = mymalloc(100);
	myfree(b);
	d = mymalloc(50);
	myfree(a);
	e = mymalloc(25);
	
	print_memory();
	print_memory_status();
	
}

