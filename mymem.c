#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>

#include <stdbool.h>

void * alloc_worst(size_t requested);
void given_block_bigger_than_max_requested_return_null();

/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct node
{
  int  size;   // How many bytes in this block?
  bool is_free;        // 1 if this block is allocated,
                       // 0 if this block is free.
  
  void * ptr_start;           // location of block in memory pool.
 
  // doubly-linked list
  struct node *prev;
  struct node *next;
};

strategies _strategy = 0;    // Current strategy 

size_t _main_mem_size_total;
//size_t _main_mem_size_free;
//size_t _main_mem_size_used;
void * _main_mem = NULL;

static struct node *_head;
static struct node *_next;

void free_list_from_head(){
	while (_head != NULL){
		void * tmp = _head;
		_head = _head->next;
		free(tmp);
	}
}
struct node * _worst_node;

void * alloc_worst(size_t req_size){
	if(_worst_node != NULL){
		_worst_node->is_free = false;
		_worst_node->size = req_size;
		struct node * tmp = _worst_node;
		_worst_node = NULL;
		return tmp;
	}else{
		return _worst_node;
	}	
}

void given_block_size_eq_max_requested_return_node_max_size__OLD__(){
	void *a;
	strategies strat = Worst;		
	initmem(strat,500); //free old mamory if any and alocate new main memory block

	//act
	a = mymalloc(500);

	//assert
	assert( ((struct node *)a)->is_free == 1 && "My first unit test in c" );
    
	free(_main_mem);
}
void givenInitMemoryWithSize_returnEmptyBlockWithSizeAlocated(){

	//struct node *a = malloc(sizeof (struct node *));
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //free old mamory if any and alocate new main memory block
	
	//act
	//a = (struct node *)mymalloc(500);

	//assert
	//assert( a->is_free == true && "One block is free" );
    //assert( a->size == block_size && "One block of given size");
	assert( _head->is_free == true && "One block is free" );
    assert( _head->size == block_size && "One block of given size");

	assert( _worst_node == _head && "Worst block points to the only free block");
	assert( _worst_node->is_free == true && "Worst block is free");
	assert( _worst_node->size == block_size && "Worst block has the same size as the total");
	assert( _worst_node->ptr_start == _main_mem && "Worst block points to the first location in main memory");

	free(_main_mem);
	free_list_from_head();
}

void givenBlockSizeIsMaxRequested_returnNodeMaxSize(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //free old mamory if any and alocate new main memory block
	
	//act
	struct node * node_req = (struct node *)mymalloc(500);

	//assert
	assert( node_req->is_free == false && "The block is free" );
    assert( node_req->size == block_size && "The block is of given size");
	assert( _worst_node == NULL && "There is now worst node - NULL");
	
	free(_main_mem);
	free_list_from_head();
}

void givenAnyBlockIsRequestedWhenNoSpaceInMemory_returnNULL(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem
	mymalloc(block_size);       //alocate all

	//act
	struct node * node_req = (struct node *)mymalloc(10);

	//assert
	assert( node_req == NULL && "No Memory left - node_req" );
	assert( _worst_node == NULL && "No Memory left - _worst_node" );
    
	free(_main_mem);
	free_list_from_head();
}

int main(){
	givenAnyBlockIsRequestedWhenNoSpaceInMemory_returnNULL();
	givenBlockSizeIsMaxRequested_returnNodeMaxSize();
	givenInitMemoryWithSize_returnEmptyBlockWithSizeAlocated();
	//given_block_bigger_than_max_requested_return_null();
	//given_block_size_eq_max_requested_return_node_max_size();
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{
	//if (_main_mem != NULL) free(_main_mem); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
	/*while (_head != NULL){
		void * tmp = _head;
		_head = _head->next;
		free(tmp);
	}*/

	return;
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
	_main_mem_size_total = size;

	if (_main_mem != NULL) free(_main_mem); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
	while (_head != NULL){
		void * tmp = _head;
		_head = _head->next;
		free(tmp);
	}

	_main_mem = malloc(size);
	
	
	/* TODO: Initialize memory management structure. */
		/*
		struct node
	{
	int  offset;   
	bool is_free;          
	void * ptr_start; 
	struct node *prev;
	struct node *next;
	};
	struct node * _main_mem = NULL;

	static struct node *_head;
	static struct node *_next;
	*/

	_head = malloc(sizeof(struct node));
	_head->is_free = true;
	_head->ptr_start = _main_mem;
	_head->size = size;
	_head->next = _next;
	_worst_node = _head;
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
	return _main_mem_size_total;
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

