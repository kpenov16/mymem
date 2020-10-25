#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>
#include <stdbool.h>

//to run my custom tests found in this file you need to
//change main2 to main
//in the console run: gcc mymem.c && ./a.out
//UNIT_TEST 0 -> will run the original unit tests, use 'less tests.log' to see the output 
//UNIT_TEST 1 -> will run my unit tests, no output means that everything works 
				//at least after my tests, one test is not working: givenScatteredFreeBlocksInTotalSizeBiggerThanRequested_returnCompactAndSatisfyTheRequest() 
				//as the compaction of scattered freed blocks is not implemented 
#define UNIT_TEST 0  

void my_do_randomized_test(int strategyToUse, int totalSize, float fillRatio, int minBlockSize, int maxBlockSize, int iterations);

void * alloc_worst(size_t requested);
void given_block_bigger_than_max_requested_return_null();
void print_my_list();
void print_my_list_to_log(FILE *log);
struct node * get_p_to_mem_in_list(void *);

/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct node
{
  int  i;
  int  size;   // How many bytes in this block?
  bool is_free;        // 1 if this block is allocated,
                       // 0 if this block is free.
  
  char * ptr_start;           // location of block in memory pool.
 
  // doubly-linked list
  struct node *prev;
  struct node *next;
};

struct map_node{
	int i;
	char * ptr_start_prev;           // location of block in memory pool.
	char * ptr_start_curr;           // location of block in memory pool.

	// doubly-linked list
	struct map_node *prev;
	struct map_node *next;
};
static struct map_node *_map_head = NULL;
static struct map_node *_map_tail = NULL;

strategies _strategy = 0;    // Current strategy 

size_t _main_mem_size_total;
//size_t _main_mem_size_free;
//size_t _main_mem_size_used;
char * _main_mem = NULL;

static struct node *_head;
static struct node *_next;

struct node * _worst_node;

struct node * getNextWorst(){
	struct node dummy = { 0 };
	struct node * next_worst = &dummy; 
	for (struct node * p = _head; p != NULL; p = p->next){
		if( p !=_worst_node && p->is_free == true && next_worst->size < p->size){
			next_worst = p;
		}
	}
	return next_worst->size > 0 ? next_worst : NULL;	
}

bool comp_will_do(size_t req_size){
	struct node * next_worst = getNextWorst();
	if(_worst_node != NULL && next_worst != NULL){
		return (_worst_node->size + next_worst->size) >= req_size;		
	}	
}

bool needs_mapping(char * ptr_start_prev){
	if(_map_head == NULL)
		return false;
	for (struct map_node* p = _map_head->next; p != NULL; p = p->next){
		if(p->ptr_start_prev == ptr_start_prev)
			return true;
	}
	return false;
}

char * get_mapping(char * ptr_start_prev){
	for (struct map_node* p = _map_head->next; p != NULL; p = p->next){
		if(p->ptr_start_prev == ptr_start_prev)
			return p->ptr_start_curr;
	}	
	return NULL;
}

void remove_mapping(char * ptr_start_prev){
	for (struct map_node* p = _map_head->next; p != NULL; p = p->next){
		if(p->ptr_start_prev == ptr_start_prev){
			if(p->prev != NULL){
				p->prev->next = p->next;
			}
			if(p->next !=NULL ){
				p->next->prev = p->prev;
			}			
			for (struct map_node* pp = pp->next; pp != NULL; pp = pp->next){
				pp->i--;
			}
			free(p);
			return;			
		}
	}	
}

bool is_mapped(char * ptr_start_curr){
	if(_map_head == NULL)
		return false;
	for (struct map_node* p = _map_head->next; p != NULL; p = p->next){
		if(p->ptr_start_curr == ptr_start_curr)
			return true;
	}
	return false;
}

void compact(size_t req_size){
	size_t total_freed = 0; 
	for (struct node* p = _head; p != NULL; p = p->next){
		if(p->is_free && p == _head && 
		   p->next != NULL && p->next->is_free == false){
			struct node * _old_head = _head; 
			total_freed =+ _old_head->size;
			
			_head = _head->next;
			free(_old_head);
			_old_head = NULL;

			_head->i = _head->i - 1;
			if(_map_head == NULL){
				_map_head = calloc(1, sizeof(struct map_node));
				_map_head->ptr_start_prev = _head->ptr_start;
				_map_head->ptr_start_curr = _head->ptr_start - total_freed;
				_map_head->i = 0;

				_map_tail = _map_head;				
			}
			
			_head->ptr_start = _head->ptr_start - total_freed;
			_head->prev = NULL;

			for(struct node * pp = _head->next; pp != NULL; pp = pp->next){
				if(pp->is_free){
					//clients don't know the memory address
					pp->i--;
					pp->ptr_start = pp->ptr_start - total_freed;

				}else{
					//clients know the memory address 
					//so we need to map the memory location
					struct map_node * _map_tail_temp = _map_tail;
					_map_tail->next = calloc(1, sizeof(struct map_node));				
					_map_tail = _map_tail->next;
					_map_tail->next = NULL;
					_map_tail->prev = _map_tail_temp;

					_map_tail->i = _map_tail_temp->i + 1;
					_map_tail->ptr_start_prev = pp->ptr_start;
					_map_tail->ptr_start_curr = pp->ptr_start - total_freed;					
					
					//modify the metadata for the nodes
					pp->i = pp->i - 1;
					pp->ptr_start = pp->ptr_start - total_freed;
				}
				
			}

			p = p->next;
		}

	}


	//very basic solution
	//just extend the tail if tail is free otherwise append  
	if(total_freed > 0){
		//find tail
		struct node * tail = _head;
		for(; tail != NULL; tail = tail->next){
			if(tail->next == NULL){
				break;
			}
		}
		if(	tail->is_free ){ 
			tail->size = tail->size + total_freed;
			tail->ptr_start = tail->ptr_start - total_freed;
		}else{
			struct node * new_tail = calloc(1, sizeof(struct node));
			tail->next = new_tail;
			new_tail->prev = tail;
			new_tail->next = NULL;
			new_tail->i = tail->i + 1;
			new_tail->is_free = true;
			new_tail->size = total_freed;
			new_tail->ptr_start = tail->ptr_start + tail->size;
		}
	}
	
	//map from 
}

void * alloc_worst(size_t req_size){
	if(_worst_node == NULL)
		return NULL;	
	else if(_worst_node->size < req_size){ 
		//this is a kind of work around, if f.eks. the current worst is not good enought but there is a new worst node    
		//we just find the new worst 
		struct node * next_worst = getNextWorst();
		if(next_worst != NULL && next_worst->size >= req_size){
			//printf("\n%s\n", "getNextWorst()->size > (_worst_node->size - req_size)");
			_worst_node = next_worst;
			return alloc_worst(req_size);
		}
		
		//this compaction is very primive, it will only work on one of the basic scenario
		//when the first node is free and when compaction is required f.eks. 5 size is required the max is 3 but there is another free block of size 2
		//but shows how to implement compaction and can be extened for the other scenario
		compact(req_size); 
		if(_worst_node->size >= req_size){
			return alloc_worst(req_size);
		}
		return NULL;	
	}else{
		//there are different situations we need to cover 
		//it is possible that there is duplications :( and refactoring is required 
		if(_worst_node->size == req_size){   //requested size is exactly the size of the worst
			if(_worst_node->prev == NULL){
				_worst_node->i = 0;                       //I keep updated index for debugging
			}else{
				_worst_node->i = _worst_node->prev->i + 1;
			}
			_worst_node->is_free = false;
			struct node * tmp = _worst_node;

			struct node * next_worst = getNextWorst(); //we need new worst if any
			if(next_worst != NULL && next_worst->size >= req_size){
				_worst_node = next_worst;
			}else{
				_worst_node = NULL;
			}		
			return UNIT_TEST ? (void *)tmp : (void *)tmp->ptr_start; //my custom tests use the linked list but the real world uses the memory locations			
		}else if(_worst_node->size > req_size){   //there is space in the worst for the requested size

			struct node * new_node = NULL;
			while (new_node == NULL){
				new_node = calloc(1, sizeof(struct node) ); //left overs from the time I belived there was no space left - this is not the case and this is a bad way to implement it 
			}
			//init the new node			
			new_node->size = req_size;
			new_node->is_free = false;	
			new_node->ptr_start = _worst_node->ptr_start;
			new_node->next = _worst_node;
			new_node->prev = _worst_node->prev;
			
			//insert the new node and update the worst
			if(_worst_node->prev == NULL){
				new_node->i = 0; 
				_worst_node->i = 1;
				
				_head = new_node;
				
				_worst_node->prev = new_node;
				_worst_node->size = _worst_node->size - req_size;
				_worst_node->ptr_start = _worst_node->ptr_start + req_size;
			}else{
				new_node->i = new_node->prev->i + 1; 
				_worst_node->i = new_node->i + 1;
				
				_worst_node->prev->next = new_node;
				_worst_node->prev = new_node;

				_worst_node->size = _worst_node->size - req_size;
				_worst_node->ptr_start = _worst_node->ptr_start + req_size;
			}
			for(struct node * p = _worst_node->next; p != NULL; p = p->next){
				p->i += 1; //update the index after the insertion
			}
			//we need to update the new worst node if that has changed
			struct node * next_worst = getNextWorst();
			if(next_worst != NULL && next_worst->size > _worst_node->size){
				_worst_node = next_worst;
			}
			return UNIT_TEST ? (void *)new_node : (void *)new_node->ptr_start; //my custom tests use the linked list but the real world uses the memory locations
		}
	} 
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void * blk){
	//the clients will try to free based on the location in memory of the block returned by myalloc
	//well when we compact to make space for bigger blocks we actually 'move' the locations
	//so to find the current locations we use a map (simple linked list) to map current to original locations
	void * block = blk;
	if(needs_mapping(blk)){ 
		block = get_mapping(blk);
		remove_mapping(blk);
	}
	//as the code works with the linked list we need a reference to the node that is requested to be freed
	struct node * node_to_del = UNIT_TEST ? (struct node *)block : get_p_to_mem_in_list(block); 


	//here we go over all the possible freeing scenarios 
	//most of them can possibly be generalized by refactoring

	if(node_to_del->size == _main_mem_size_total){ 
		node_to_del->is_free = true;
		_worst_node = _head;
		return;
	}

	//here we merge the block to be deleted with the one on the right (next) and the one on the left (prev)
	//as they are free
	//we remember to update which block is the worst 
	if(node_to_del->prev != NULL &&
	   node_to_del->prev->is_free == true && 
	   node_to_del->next != NULL &&
	   node_to_del->next->is_free == true){
	
		int size_freed = node_to_del->prev->size + node_to_del->size + node_to_del->next->size;
		if(node_to_del->prev == _worst_node){
			_worst_node->size = size_freed;
			_worst_node->next = node_to_del->next->next;
			if(node_to_del->next->next != NULL){
				node_to_del->next->next->prev = _worst_node;
				free(node_to_del->next);
				node_to_del->next = NULL;
			}
			free(node_to_del);
			node_to_del = NULL;
			for(struct node * p = _worst_node->next; p != NULL; p = p->next){
				p->i -= 2;
			}
			return;
		}
		if(node_to_del->next == _worst_node){
			node_to_del->prev->size = size_freed;
			node_to_del->prev->next = node_to_del->next->next; 
			if(node_to_del->next->next != NULL){
				node_to_del->next->next->prev = node_to_del->prev;
				free(node_to_del->next);
				node_to_del->next = NULL;
			}
			_worst_node = node_to_del->prev;
			free(node_to_del);
			node_to_del = NULL;
			for(struct node * p = _worst_node->next; p != NULL; p = p->next){
				p->i -= 2;
			}
			return;
		}
		
		if(_worst_node->size < size_freed){
			_worst_node = node_to_del->prev;
		}
		struct node * prev = node_to_del->prev;
		node_to_del->prev->size = size_freed;
		node_to_del->prev->next = node_to_del->next->next; 
		if(node_to_del->next->next != NULL){
			node_to_del->next->next->prev = node_to_del->prev;
			free(node_to_del->next);
			node_to_del->next = NULL;
		}
		//_worst_node = node_to_del->prev;
		free(node_to_del);
		node_to_del = NULL;
		for(struct node * p = prev->next; p != NULL; p = p->next){
			p->i -= 2;
		}
		return;
	}

	//here we merge the block to be deleted with the one of the right (next)
	//we remember to update which block is the worst 
	if(node_to_del->prev != NULL &&
	   node_to_del->prev->is_free == false && 
	   node_to_del->next != NULL &&
	   node_to_del->next->is_free == true){
		struct node * next_before = node_to_del->next;
		int size_freed = node_to_del->size + node_to_del->next->size;
		node_to_del->is_free = true;
		node_to_del->size = size_freed;			   
		if(node_to_del->next->next != NULL){
			node_to_del->next->next->prev = node_to_del;
		}
		node_to_del->next = node_to_del->next->next;

		if(node_to_del->next == _worst_node || _worst_node->size < size_freed){
			_worst_node = node_to_del;
		}
		free(next_before);
		next_before = NULL;
		for(struct node * p = node_to_del->next; p != NULL; p = p->next){
			p->i -= 1;
		}
		return;
	}

	//here we merge the block to be deleted with the one of the left (prev)
	//we remember to update which block is the worst 
	if(node_to_del->prev != NULL &&
	   node_to_del->prev->is_free == true && 
	   node_to_del->next != NULL &&
	   node_to_del->next->is_free == false){
		struct node * prev = node_to_del->prev;
		int size_freed = node_to_del->size + node_to_del->prev->size;
		prev->next = node_to_del->next;
		prev->size = size_freed;
		if(node_to_del->next->prev != NULL){
			node_to_del->next->prev = prev;
		}

		if(prev != _worst_node && _worst_node->size < size_freed){
			_worst_node = prev;
		}
		free(node_to_del);
		node_to_del = NULL;
		for(struct node * p = prev->next; p != NULL; p = p->next){
			p->i -= 1;
		}
		return;
	}

	//hm I am kind of writing the comments after I wrote the code and now I don't think this is needed
	//But I will keep it here anyhow nor now
	if(node_to_del->prev != NULL && 
	   node_to_del->prev == _worst_node){

		_worst_node->size = _worst_node->size + node_to_del->size;
		_worst_node->next = node_to_del->next;
		//if(node_to_del->next != NULL){
		//	node_to_del->next->prev = _worst_node;
		//}
		free(node_to_del);
		node_to_del = NULL;
		return;
	}

	//some more special situations I was writing tests for
	//we free for the first time after all memory was allocated last time
	//we free in the end
	if(node_to_del->prev != NULL &&
	   node_to_del->prev->is_free == false && 
	   node_to_del->next == NULL &&
	   _worst_node == NULL){
		_worst_node = node_to_del;
		_worst_node->is_free = true;
		node_to_del = NULL;
		return;
	}

	//we free at the end and there is a worst node
	if(node_to_del->prev != NULL &&
	   node_to_del->prev->is_free == false && 
	   node_to_del->next == NULL &&
	   _worst_node != NULL){
		if(node_to_del->size > _worst_node->size){
			_worst_node = node_to_del;
		}	
		node_to_del->is_free = true;
		return;
	}

	//free in the middle when there is no worst
	if(node_to_del->prev != NULL && 
	   node_to_del->prev->is_free == false && 
	   node_to_del->next != NULL &&
	   node_to_del->next->is_free == false &&
	   _worst_node == NULL){
		_worst_node = node_to_del;
		_worst_node->is_free = true;
		node_to_del = NULL;
		return;
	}

	//free in the middle when there is a worst
	//left and ringt nodes are not free
	if(node_to_del->prev != NULL && 
	   node_to_del->prev->is_free == false && 
	   node_to_del->next != NULL &&
	   node_to_del->next->is_free == false &&
	   _worst_node != NULL){
		if(node_to_del->size > _worst_node->size){
			_worst_node = node_to_del;
		}	
		node_to_del->is_free = true;
		return;
	}
	
	//freeing at the head when the next is worst, TODO: refectoring please ...
	if(node_to_del->prev == NULL && 
	   node_to_del->next != NULL && node_to_del->next == _worst_node){
		
		node_to_del->i = 0;
		node_to_del->is_free = true;
		node_to_del->next = NULL;
		node_to_del->prev = NULL;
		node_to_del->ptr_start = _main_mem;
		node_to_del->size = _main_mem_size_total;
		
		free(_worst_node);
		_worst_node = NULL;

		_worst_node = _head;
		return;
	}

	//free the head when the right is free
	if(node_to_del->prev == NULL && 
	   node_to_del->next != NULL && 
	   node_to_del->next->is_free == true){
		
		//no change
		node_to_del->i = 0;
		node_to_del->prev = NULL;
		node_to_del->ptr_start = _main_mem;


		node_to_del->is_free = true;
		node_to_del->size = node_to_del->size + node_to_del->next->size;
		struct node * next = node_to_del->next;
		node_to_del->next = node_to_del->next->next;
		node_to_del->next->prev = node_to_del;
		free(next);
		next = NULL;

		for(struct node * p = node_to_del->next; p != NULL; p = p->next){
			p->i -= 1;
		}

		if(node_to_del->size > _worst_node->size){
			_worst_node = node_to_del;
		}

		return;
	}

	//free the head when the right is not free
	if(node_to_del->prev == NULL && 
	   node_to_del->next != NULL && 
	   node_to_del->next->is_free == false){

		node_to_del->i = 0;
		node_to_del->is_free = true;
		node_to_del->ptr_start = _main_mem;

		if(_worst_node == NULL || 
			(_worst_node != NULL && _worst_node->size < node_to_del->size)){
			
			_worst_node = node_to_del;
			return;
		}
		if(_worst_node != NULL &&
			_worst_node->size > node_to_del->size){
			return;
		}
		return;
	}

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

void initmem(strategies strategy, size_t size){
	_strategy = strategy;

	/* All implementations will need an actual block of memory to use */
	_main_mem_size_total = size;

	if (_main_mem != NULL){
		free(_main_mem); /* in case this is not the first time initmem2 is called */
		_main_mem = NULL;
	} 
	/* Release any other memory you were using for bookkeeping when doing a re-initialization! */
	//free the metadata for the memory blocks - the linked list
	while (_head != NULL){
		void * tmp = _head;
		_head = _head->next;
		free(tmp);
		tmp = NULL; //make it a habit
	}

	//free the mapping list for memory locations 
	while (_map_head != NULL){
		void * tmp = _map_head;
		_map_head = _map_head->next;
		free(tmp);
		tmp = NULL; 
	}
		
	/* Initialize memory management structure. */
	_main_mem = (char *)calloc(size, sizeof(char));
				
	//I start with the full size as the worst node
	_head = calloc(1, sizeof(struct node));
	_head->i = 0;
	_head->is_free = true;
	_head->ptr_start = _main_mem;
	_head->size = size;
	_head->next = NULL;
	_head->prev = NULL;
	
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
int mem_holes(){
	int holes = 0;
	for(struct node * p = _head; p != NULL; p = p->next){
		if(p->is_free) 
			holes++;
	}
	return holes;
}

/* Get the number of bytes allocated */
int mem_allocated(){
	int bytes = 0;
	for(struct node * p = _head; p != NULL; p = p->next){
		if(!p->is_free) 
			bytes += p->size;
	}
	return bytes;
}

/* Number of non-allocated bytes */
int mem_free(){
	int bytes = 0;
	for(struct node * p = _head; p != NULL; p = p->next){
		if(p->is_free) 
			bytes += p->size;
	}
	return bytes;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free(){
	if(_worst_node != NULL)
		return _worst_node->size;
	return 0;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size){
	int blocks = 0;
	for(struct node * p = _head; p != NULL; p = p->next){
		if(p->is_free && p->size < size) 
			blocks++;
	}
	return blocks;
}       

char mem_is_alloc(void *ptr){
	char * l = (char *) ptr;
	for(struct node * p = _head; p != NULL; p = p->next){
		char * low = p->ptr_start;
		char * high = p->ptr_start + p->size;
		//printf("\nlow = %p, l = %p, high = %p, main_start = %p\n", low, l, high, _main_mem);
		if(low <= l && l < high){
			return !p->is_free;
		} 		
	}	
	return -1;
}

struct node * get_p_to_mem_in_list(void *ptr){
	char * l = (char *) ptr;
	for(struct node * p = _head; p != NULL; p = p->next){
		char * low = p->ptr_start;
		char * high = p->ptr_start + p->size;
		//printf("\nlow = %p, l = %p, high = %p, main_start = %p\n", low, l, high, _main_mem);
		if(low <= l && l < high){
			return p;
		} 		
	}	
	return NULL;
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
	print_my_list();
	return;
}

void print_memory_to_log(FILE *log)
{
	print_my_list_to_log(log);
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
	print_my_list();
	print_memory();
	print_memory_status();
	
}

void print_my_list(){
	printf("\n_memory starts = %p\n", &_main_mem);
	for(struct node * p = _head; p != NULL; p = p->next)
		printf("\nnode: is_free = %s, size = %d, ptr_start = %p, i = %d\n", 
				p->is_free?"true":"false", 
				p->size,
				p->ptr_start,//&p->ptr_start,
				p->i);

	printf("\n_map_node:\n");
	for(struct map_node * p = _map_head; p != NULL; p = p->next)
		printf("\nptr_start_curr = %p, p->ptr_start_prev = %p, i = %d\n", 
				p->ptr_start_curr,
				p->ptr_start_prev,
				p->i);
}

void print_my_list_to_log(FILE *log){
	fprintf(log,"\n_memory starts = %p\n", &_main_mem);
	for(struct node * p = _head; p != NULL; p = p->next)
		fprintf(log,"\nnode: is_free = %s, size = %d, ptr_start = %p, i = %d\n", 
				p->is_free?"true":"false", 
				p->size,
				p->ptr_start,//&p->ptr_start,
				p->i);
}

//#######################################################################################
//################## If you whan to see my custom tests then look down ##################
//#######################################################################################
//to run my custom tests found in this file you need to
//change main2 to main
//in the console run: gcc mymem.c && ./a.out
//UNIT_TEST 0 -> will run the original unit tests, use 'less tests.log' to see the output 
//UNIT_TEST 1 -> will run my unit tests, no output means that everything works 
				//at least after my tests, one test is not working: givenScatteredFreeBlocksInTotalSizeBiggerThanRequested_returnCompactAndSatisfyTheRequest() 
				//as the compaction of scattered freed blocks is not implemented 

void givenInitMemoryWithSize_returnEmptyBlockWithSizeAlocated(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	
	//act
	initmem(strat, block_size); //free old mamory if any and alocate new main memory block

	//assert
	assert( _head->is_free == true && "One block is free" );
    assert( _head->size == block_size && "One block of given size");

	assert( _worst_node == _head && "Worst block points to the only free block");
	assert( _worst_node->is_free == true && "Worst block is free");
	assert( _worst_node->size == block_size && "Worst block has the same size as the total");
	assert( _worst_node->ptr_start == _main_mem && "Worst block points to the first location in main memory");

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
}

void givenRequestMoemoryMoreThanAvailable_returnNULL(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem
	mymalloc(400);              //alocate some

	//act
	struct node * node_req = (struct node *)mymalloc(101); //request more than available

	//assert
	assert( node_req == NULL && "Memory left but not enough to alocate - node_req == NULL" );
	assert( _worst_node != NULL && "Memory left but not enough - _worst_node != NULL" );
}

void givenRequestMoemoryLessThanTheAvailable_returnTheNodeAndWorstNodeIsNULL(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	//act
	int req_size = 400;
	struct node * node_req = (struct node *)mymalloc(req_size); //request more than available

	//assert
	assert( node_req->size == req_size && "node_req->size == req_size");
	assert( node_req->is_free == false && "node_req->is_free == false");
	assert( node_req->next == _worst_node && "node_req->next == _worst_node");
	
	assert( _worst_node != NULL && "_worst_node != NULL");
    assert( _worst_node->prev == node_req && "_worst_node->prev == node_req");
	assert( _worst_node->size == block_size - req_size && "_worst_node->size == block_size - req_size");
}

void given2BlocksRequestedOfTotalSizeOfTheTotalMemory_return2NodesCreatedWithNoFreeSpaceInMemory(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	//act
	int req_size01 = 400;
	int req_size02 = block_size - req_size01;
	struct node * node_req01 = (struct node *)mymalloc(req_size01); //request more than available
	struct node * node_req02 = (struct node *)mymalloc(req_size02);
	
	//assert
	assert( node_req01->size == req_size01 && "node_req01->size == req_size");
	assert( node_req01->is_free == false && "node_req01->is_free == false");
	assert( node_req01->next == node_req02 && "node_req01->next == node_req02");
	assert( node_req01->prev == NULL && "node_req01->prev == NULL");
	
	assert( node_req02->size == req_size02 && "node_req02->size == req_size");
	assert( node_req02->is_free == false && "node_req02->is_free == false");
	assert( node_req02->prev == node_req01 && "node_req02->prev == node_req01");
	assert( node_req02->next == NULL && "node_req02->next == NULL");

	assert( _head == node_req01 && "_head == node_req01");
	assert( _worst_node == NULL && "_worst_node == NULL");
}

void given3BlocksRequestedOfTotalSizeOfTheTotalMemory_return3NodesCreatedWithNoFreeSpaceInMemory(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	//act
	int req_size01 = 300;
	int req_size02 = 100;
	int req_size03 = block_size - req_size01 - req_size02;
	struct node * node_req01 = (struct node *)mymalloc(req_size01); //request more than available
	struct node * node_req02 = (struct node *)mymalloc(req_size02);
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	//print_my_list();
	
	//assert
	assert( node_req01->size == req_size01 && "node_req01->size == req_size");
	assert( node_req01->is_free == false && "node_req01->is_free == false");
	assert( node_req01->next == node_req02 && "node_req01->next == node_req02");
	assert( node_req01->prev == NULL && "node_req01->prev == NULL");
	
	assert( node_req02->size == req_size02 && "node_req02->size == req_size02");
	assert( node_req02->is_free == false && "node_req02->is_free == false");
	assert( node_req02->prev == node_req01 && "node_req02->prev == node_req01");
	assert( node_req02->next == node_req03 && "node_req02->next == node_req03");

	assert( node_req03->size == req_size03 && "node_req03->size == req_size03");
	assert( node_req03->is_free == false && "node_req03->is_free == false");
	assert( node_req03->prev == node_req02 && "node_req03->prev == node_req02");
	assert( node_req03->next == NULL && "node_req03->next == NULL");

	assert( _head == node_req01 && "_head == node_req01");
	assert( _worst_node == NULL && "_worst_node == NULL");
}

void given4BlocksRequestedOfTotalSizeOfTheTotalMemory_return4NodesCreatedWithNoFreeSpaceInMemory(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	//act
	int req_size01 = 200;
	int req_size02 = 150;
	int req_size03 = 100;
	int req_size04 = block_size - req_size01 - req_size02 - req_size03; //50
	struct node * node_req01 = (struct node *)mymalloc(req_size01); //request more than available
	struct node * node_req02 = (struct node *)mymalloc(req_size02);
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	//print_my_list();
	
	//assert
	assert( node_req01->size == req_size01 && "node_req01->size == req_size");
	assert( node_req01->is_free == false && "node_req01->is_free == false");
	assert( node_req01->next == node_req02 && "node_req01->next == node_req02");
	assert( node_req01->prev == NULL && "node_req01->prev == NULL");
	
	assert( node_req02->size == req_size02 && "node_req02->size == req_size02");
	assert( node_req02->is_free == false && "node_req02->is_free == false");
	assert( node_req02->prev == node_req01 && "node_req02->prev == node_req01");
	assert( node_req02->next == node_req03 && "node_req02->next == node_req03");

	assert( node_req03->size == req_size03 && "node_req03->size == req_size03");
	assert( node_req03->is_free == false && "node_req03->is_free == false");
	assert( node_req03->prev == node_req02 && "node_req03->prev == node_req02");
	assert( node_req03->next == node_req04 && "node_req03->next == node_req04");
	
	
	assert( node_req04->size == req_size04 && "node_req04->size == req_size04");
	assert( node_req04->is_free == false && "node_req04->is_free == false");
	assert( node_req04->prev == node_req03 && "node_req04->prev == node_req03");
	assert( node_req04->next == NULL && "node_req04->next == NULL");

	assert( _head == node_req01 && "_head == node_req01");
	assert( _worst_node == NULL && "_worst_node == NULL");
}

void givenAddOneBlockMaxSizeAndRemoveTheBlock_returnInitialStatus(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size = 500;
	struct node * node_req = (struct node *)mymalloc(req_size); 
	//printf("\n%s\n","Setup");
	//print_my_list();

	//act
	myfree(node_req);

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == NULL && "_head->next == NULL");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == block_size && "_head->size == block_size");
	
	assert( _worst_node == _head && "_worst_node == _head");
}

void givenAddOneBlockOfSizeLessThanMaxAndRemoveTheBlock_returnInitialStatus(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size = 400;
	struct node * node_req = (struct node *)mymalloc(req_size); 
	//printf("\n%s\n","Setup");
	//print_my_list();

	//act
	myfree(node_req);

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == NULL && "_head->next == NULL");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == block_size && "_head->size == block_size");
	
	assert( _worst_node == _head && "_worst_node == _head");
}

void givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheFirstBlock_returnTheFirstIsFreedAndTheSecondIsThereNotFreed(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 400;
	int req_size02 = block_size - req_size01;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req01);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == node_req02 && "_head->next == node_req02");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 && "_head->size == block_size");
	
	assert( _worst_node == _head && "_worst_node == _head");

	assert(node_req02->i == 1 && "node_req02->i == 1");
	assert(node_req02->is_free == false && "node_req02->is_free == false");
	assert(node_req02->next == NULL && "node_req02->next == NULL");
	assert(node_req02->prev == _worst_node && "node_req02->prev == _worst_node");
	assert(node_req02->ptr_start == _main_mem + _head->size && "node_req02->ptr_start == _main_mem + _head->size");
	assert(node_req02->size == req_size02 && "node_req02->size == req_size02");
}

void givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheBlocks_returnTheInitialStatus(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 400;
	int req_size02 = block_size - req_size01;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req01);
	myfree(node_req02);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == NULL && "_head->next == NULL");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == block_size && "_head->size == block_size");
	
	assert( _worst_node == _head && "_worst_node == _head");
}


void givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheSecondBlock_returnTheSecondIsFreedAndTheFirstIsThereNotFreed(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 400;
	int req_size02 = block_size - req_size01;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == false && "_head->is_free == false");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == node_req02 && "_head->next == node_req02");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 && "_head->size == block_size");
	
	assert(_worst_node->i == 1 && "_worst_node->i == 1");
	assert(_worst_node->is_free == true && "_worst_node->is_free == true");
	assert(_worst_node->next == NULL && "_worst_node->next == NULL");
	assert(_worst_node->prev == node_req01 && "_worst_node->prev == node_req01");
	assert(_worst_node->ptr_start == _main_mem + req_size01 && "_worst_node->ptr_start == _main_mem + req_size01");
	assert(_worst_node->size == req_size02 && "_worst_node->size == req_size02");
}

void givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheBlocksInReverseOrder_returnTheInitialStatus(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 400;
	int req_size02 = block_size - req_size01;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	myfree(node_req01);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == NULL && "_head->next == NULL");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == block_size && "_head->size == block_size");
	
	assert( _worst_node == _head && "_worst_node == _head");
}

void givenAddThreeBlocksOfTotalSizeToTheMaxAndRemoveTheSecondBlock_returnTheSecondIsFreedAndTheFirstAndLastAreThereNotFreed(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 300;
	int req_size02 = 150;
	int req_size03 = block_size - req_size01 - req_size02;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == false && "_head->is_free == false");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == _worst_node && "_head->next == _worst_node");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 && "_head->size == block_size");
	
	assert(_worst_node->i == 1 && "_worst_node->i == 1");
	assert(_worst_node->is_free == true && "_worst_node->is_free == true");
	assert(_worst_node->next == node_req03 && "_worst_node->next == node_req03");
	assert(_worst_node->prev == node_req01 && "_worst_node->prev == node_req01");
	assert(_worst_node->ptr_start == _main_mem + req_size01 && "_worst_node->ptr_start == _main_mem + req_size01");
	assert(_worst_node->size == req_size02 && "_worst_node->size == req_size02");

	assert(node_req03->i == 2 && "node_req03->i == 2");
	assert(node_req03->is_free == false && "node_req03->is_free == false");
	assert(node_req03->next == NULL && "node_req03->next == NULL");
	assert(node_req03->prev == _worst_node && "node_req03->prev == _worst_node");
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02 && "node_req03->ptr_start == _main_mem + req_size01 + req_size02");
	assert(node_req03->size == req_size03 && "node_req03->size == req_size03");
}

void givenAddThreeBlocksOfTotalSizeToTheMaxAndRemoveTheLastBlock_returnTheLastIsFreedAndTheFirstAndSecondAreThereNotFreed(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 300;
	int req_size02 = 150;
	int req_size03 = block_size - req_size01 - req_size02;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req03);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == false && "_head->is_free == false");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == node_req02 && "_head->next == node_req02");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 && "_head->size == block_size");

	assert(node_req02->i == 1 && "node_req02->i == 1");
	assert(node_req02->is_free == false && "node_req02->is_free == false");
	assert(node_req02->next == _worst_node && "node_req02->next == _worst_node");
	assert(node_req02->prev == _head && "node_req03->prev == _head");
	assert(node_req02->ptr_start == _main_mem + req_size01 && "node_req02->ptr_start == _main_mem + req_size01");
	assert(node_req02->size == req_size02 && "node_req02->size == req_size02");

	assert(_worst_node->i == 2 && "_worst_node->i == 2");
	assert(_worst_node->is_free == true && "_worst_node->is_free == true");
	assert(_worst_node->next == NULL && "_worst_node->next == NULL");
	assert(_worst_node->prev == node_req02 && "_worst_node->prev == node_req02");
	assert(_worst_node->ptr_start == _main_mem + req_size01 + req_size02 && "_worst_node->ptr_start == _main_mem + req_size01 + req_size02");
	assert(_worst_node->size == req_size03 && "_worst_node->size == req_size03");
}

void givenAddThreeBlocksOfTotalSizeToTheMaxAndRemoveTheFirstBlock_returnTheFirstIsFreedAndTheSecondAndLastAreThereNotFreed(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 300;
	int req_size02 = 150;
	int req_size03 = block_size - req_size01 - req_size02;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req01);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head == _worst_node && "_head == _worst_node");

	assert(_worst_node->i == 0 && "_worst_node->i == 0");
	assert(_worst_node->is_free == true && "_worst_node->is_free == true");
	assert(_worst_node->next == node_req02 && "_worst_node->next == node_req02");
	assert(_worst_node->prev == NULL && "_worst_node->prev == NULL");
	assert(_worst_node->ptr_start == _main_mem && "_worst_node->ptr_start == _main_mem");
	assert(_worst_node->size == req_size01 && "_worst_node->size == req_size01");

	assert(node_req02->i == 1 && "node_req02->i == 1");
	assert(node_req02->is_free == false && "node_req02->is_free == false");
	assert(node_req02->next == node_req03 && "node_req02->next == node_req03");
	assert(node_req02->prev == _head && "node_req02->prev == _head");
	assert(node_req02->ptr_start == _main_mem + req_size01 && "node_req02->ptr_start == _main_mem + req_size01");
	assert(node_req02->size == req_size02 && "node_req02->size == req_size02");

	assert(node_req03->i == 2 && "node_req03->i == 2");
	assert(node_req03->is_free == false && "node_req03->is_free == false");
	assert(node_req03->next == NULL && "node_req03->next == NULL");
	assert(node_req03->prev == node_req02 && "node_req03->prev == node_req02");
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02 && "node_req03->ptr_start == _main_mem + req_size01 + req_size02");
	assert(node_req03->size == req_size03 && "node_req03->size == req_size03");

}

void givenAddThreeBlocksOfTotalSizeLessThenTheMaxAndFreeTheSecondBlockBiggerThanTheCurrentWorst_returnTheSecondIsFreedAndTheFirstAndLastAreThereNotFreedAndWorstIsTheFreedSecondBlock(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 300;
	int req_size02 = 100;
	int req_size03 = 50;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03); 
	struct node * worst_before = _worst_node;
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == false && "_head->is_free == false");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == _worst_node && "_head->next == _worst_node");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 && "_head->size == block_size");
	
	assert(_worst_node->i == 1 && "_worst_node->i == 1");
	assert(_worst_node->is_free == true && "_worst_node->is_free == true");
	assert(_worst_node->next == node_req03 && "_worst_node->next == node_req03");
	assert(_worst_node->prev == node_req01 && "_worst_node->prev == node_req01");
	assert(_worst_node->ptr_start == _main_mem + req_size01 && "_worst_node->ptr_start == _main_mem + req_size01");
	assert(_worst_node->size == req_size02 && "_worst_node->size == req_size02");

	assert(node_req03->i == 2 && "node_req03->i == 2");
	assert(node_req03->is_free == false && "node_req03->is_free == false");
	assert(node_req03->next == worst_before && "node_req03->next == worst_before");
	assert(node_req03->prev == _worst_node && "node_req03->prev == _worst_node");
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02 && "node_req03->ptr_start == _main_mem + req_size01 + req_size02");
	assert(node_req03->size == req_size03 && "node_req03->size == req_size03");

	assert(worst_before->i == 3 && "worst_before->i == 3");
	assert(worst_before->is_free == true && "worst_before->is_free == true");
	assert(worst_before->next == NULL && "worst_before->next == NULL");
	assert(worst_before->prev == node_req03 && "worst_before->prev == node_req03");
	assert(worst_before->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 && "worst_before->ptr_start == _main_mem + req_size01 + req_size02 + req_size03");
	assert(worst_before->size == block_size - req_size01 - req_size02 - req_size03 && "worst_before->size == block_size - req_size01 - req_size02 - req_size03");
}

void givenAddThreeBlocksOfTotalSizeLessThenTheMaxAndFreeTheSecondBlockSmallerThanTheCurrentWorst_returnTheSecondIsFreedAndTheFirstAndLastAreThereNotFreedAndWorstIsNotChangedBlock(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 300;
	int req_size02 = 50;
	int req_size03 = 50;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == false && "_head->is_free == false");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == node_req02 && "_head->next == node_req02");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 && "_head->size == block_size");

	assert(node_req02->i == 1 && "node_req02->i == 1");
	assert(node_req02->is_free == true && "node_req02->is_free == true");
	assert(node_req02->next == node_req03 && "node_req02->next == node_req03");
	assert(node_req02->prev == _head && "node_req02->prev == _head");
	assert(node_req02->ptr_start == _main_mem + req_size01 && "node_req02->ptr_start == _main_mem + req_size01");
	assert(node_req02->size == req_size02 && "node_req02->size == req_size02");

	assert(node_req03->i == 2 && "node_req03->i == 2");
	assert(node_req03->is_free == false && "node_req03->is_free == false");
	assert(node_req03->next == _worst_node && "node_req03->next == _worst_node");
	assert(node_req03->prev == node_req02 && "node_req03->prev == node_req02");
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02 && "node_req03->ptr_start == _main_mem + req_size01 + req_size02");
	assert(node_req03->size == req_size03 && "node_req03->size == req_size03");

	assert(_worst_node->i == 3 && "_worst_node->i == 3");
	assert(_worst_node->is_free == true && "_worst_node->is_free == true");
	assert(_worst_node->next == NULL && "_worst_node->next == NULL");
	assert(_worst_node->prev == node_req03 && "_worst_node->prev == node_req03");
	assert(_worst_node->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 && "_worst_node->ptr_start == _main_mem + req_size01 + req_size02 + req_size03");
	assert(_worst_node->size == block_size - req_size01 - req_size02 - req_size03 && "_worst_node->size == block_size - req_size01 - req_size02 - req_size03");
}

void givenAddThreeBlocksFreeSecondAndThenFirstInTotalSizeLessThanWorst_returnHeadBlockOfNewSizeSumOfFirstAndSecondAndFreedAndWorstNotChanged(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 50;
	int req_size02 = 100;
	int req_size03 = 50;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03); 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	myfree(node_req01);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == node_req03 && "_head->next == node_req03");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 + req_size02 && "_head->size == req_size01 + req_size02");

	assert(node_req03->i == 1 && "node_req03->i == 1");
	assert(node_req03->is_free == false && "node_req03->is_free == false");
	assert(node_req03->next == _worst_node && "node_req03->next == _worst_node");
	assert(node_req03->prev == _head && "node_req03->prev == _head");
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02 && "node_req03->ptr_start == _main_mem + req_size01 + req_size02");
	assert(node_req03->size == req_size03 && "node_req03->size == req_size03");

	assert(_worst_node->i == 2 && "_worst_node->i == 2");
	assert(_worst_node->is_free == true && "_worst_node->is_free == true");
	assert(_worst_node->next == NULL && "_worst_node->next == NULL");
	assert(_worst_node->prev == node_req03 && "_worst_node->prev == node_req03");
	assert(_worst_node->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 && "_worst_node->ptr_start == _main_mem + req_size01 + req_size02 + req_size03");
	assert(_worst_node->size == block_size - req_size01 - req_size02 - req_size03 && "_worst_node->size == block_size - req_size01 - req_size02 - req_size03");
}

void givenAddThreeBlocksFreeSecondAndThenFirstInTotalSizeBiggerThanWorst_returnHeadBlockOfNewSizeSumOfFirstAndSecondAndFreedAndWorstChangedToHead(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 120;
	int req_size02 = 130;
	int req_size03 = 100;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * worst_before = node_req03->next; 
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	myfree(node_req01);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head == _worst_node && "_head == _worst_node");

	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == node_req03 && "_head->next == node_req03");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 + req_size02 && "_head->size == req_size01 + req_size02");

	assert(node_req03->i == 1 && "node_req03->i == 1");
	assert(node_req03->is_free == false && "node_req03->is_free == false");
	assert(node_req03->next == worst_before && "node_req03->next == worst_before");
	assert(node_req03->prev == _head && "node_req03->prev == _head");
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02 && "node_req03->ptr_start == _main_mem + req_size01 + req_size02");
	assert(node_req03->size == req_size03 && "node_req03->size == req_size03");

	assert(worst_before->i == 2 && "worst_before->i == 2");
	assert(worst_before->is_free == true && "worst_before->is_free == true");
	assert(worst_before->next == NULL && "worst_before->next == NULL");
	assert(worst_before->prev == node_req03 && "worst_before->prev == node_req03");
	assert(worst_before->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 && "worst_before->ptr_start == _main_mem + req_size01 + req_size02 + req_size03");
	assert(worst_before->size == block_size - req_size01 - req_size02 - req_size03 && "worst_before->size == block_size - req_size01 - req_size02 - req_size03");
}

void givenAddThreeBlocksFreeSecondAndThenFirstThenLastInTotalSizeLessThanTotal_returnInitialState(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 120;
	int req_size02 = 130;
	int req_size03 = 100;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	myfree(node_req01);
	myfree(node_req03);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head == _worst_node && "_head == _worst_node");

	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == NULL && "_head->next == NULL");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == block_size && "_head->size == block_size");
}

void givenAddThreeBlocksFreeSecondAndThenFirstThenLastInTotalSizeLessThanTotalSoThatWorstIsInEnd_returnInitialState(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 30;
	int req_size03 = 100;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	myfree(node_req01);
	myfree(node_req03);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head == _worst_node && "_head == _worst_node");

	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == true && "_head->is_free == true");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == NULL && "_head->next == NULL");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == block_size && "_head->size == block_size");
}


void givenAddThreeBlocksFreeSecondAndThenThirdInTotalSizeLessThanTotalSoThatWorstIsInEnd_returnFirstIsThereSecondIsWorst(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 30;
	int req_size03 = 100;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	myfree(node_req03);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 && "_head->i == 0");
	assert( _head->is_free == false && "_head->is_free == false");
	assert( _head->prev == NULL && "assert( _head->prev == NULL");
	assert( _head->next == node_req02 && "_head->next == node_req02");
	assert( _head->ptr_start == _main_mem && "_head->ptr_start == _main_mem");
	assert( _head->size == req_size01 && "_head->size == req_size01");

	assert( _worst_node == node_req02 && "_worst_node == node_req02");
	assert( _worst_node->i == 1 && "_worst_node->i == 1");
	assert( _worst_node->is_free == true && "_worst_node->is_free == true");
	assert( _worst_node->next == NULL && "_worst_node->next == NULL");
	assert( _worst_node->prev == _head && "_worst_node->prev == _head");
	assert( _worst_node->ptr_start == _main_mem + _head->size && "_worst_node->ptr_start == _main_mem + _head->size");
	assert( _worst_node->size == block_size - req_size01 && "_worst_node->size == block_size - req_size01");
}

void givenFreeBlockBetweenFreedBlocksButNotTheWorstSoThatNewBlockLessThanWorst_returnWorstUnchangedNewBiggerFreedBlockInsteadOfTheTreeBlocks(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 30;
	int req_size03 = 50;
	int req_size04 = 100;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req01);
	myfree(node_req03);
	myfree(node_req02);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 );
	assert( _head->is_free == true );
	assert( _head->prev == NULL );
	assert( _head->next == node_req04 );
	assert( _head->ptr_start == _main_mem );
	assert( _head->size == req_size01 + req_size02 + req_size03 );

	assert(node_req04->i == 1);
	assert(node_req04->is_free == false);
	assert(node_req04->next == _worst_node);
	assert(node_req04->prev == _head);
	assert(node_req04->ptr_start == _main_mem + req_size01 + req_size02 + req_size03);	
	assert(node_req04->size == req_size04);

	assert( _worst_node == node_req04->next && "_worst_node == node_req04->next");
	assert( _worst_node->i == 2 && "_worst_node->i == 2");
	assert( _worst_node->is_free == true && "_worst_node->is_free == true");
	assert( _worst_node->next == NULL && "_worst_node->next == NULL");
	assert( _worst_node->prev == node_req04 && "_worst_node->prev == node_req04");
	assert( _worst_node->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 + req_size04);
	assert( _worst_node->size == block_size - (req_size01 + req_size02 + req_size03 + req_size04));
}

void givenFreeBlockBetweenFreedBlocksButNotTheWorstSoThatNewBlockBiggerThanWorst_returnWorstIsTheNewBiggerFreedBlock(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 200;
	int req_size03 = 30;
	int req_size04 = 50;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * worst_before = node_req04->next;
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req01);
	myfree(node_req03);
	myfree(node_req02);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert(_worst_node == _head);

	assert( _head->i == 0 );
	assert( _head->is_free == true );
	assert( _head->prev == NULL );
	assert( _head->next == node_req04 );
	assert( _head->ptr_start == _main_mem );
	assert( _head->size == req_size01 + req_size02 + req_size03 );

	assert(node_req04->i == 1);
	assert(node_req04->is_free == false);
	assert(node_req04->next == worst_before);
	assert(node_req04->prev == _head);
	assert(node_req04->ptr_start == _main_mem + req_size01 + req_size02 + req_size03);	
	assert(node_req04->size == req_size04);

	assert( worst_before == node_req04->next );
	assert( worst_before->i == 2 );
	assert( worst_before->is_free == true );
	assert( worst_before->next == NULL );
	assert( worst_before->prev == node_req04 );
	assert( worst_before->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 + req_size04);
	assert( worst_before->size == block_size - (req_size01 + req_size02 + req_size03 + req_size04));
}

void givenFreeBlockBetweenFreedBlocksInMiddleOfListButNotTheWorstSoThatNewBlockBiggerThanWorst_returnWorstIsTheNewBiggerFreedBlock(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 30;
	int req_size03 = 200;
	int req_size04 = 25;
	int req_size05 = 25;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	struct node * worst_before = node_req05->next;
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	myfree(node_req04);
	myfree(node_req03);

	//printf("\n%s\n","After Act");
	//print_my_list();

	//assert
	assert( _head->i == 0 );
	assert( _head->is_free == false );
	assert( _head->prev == NULL );
	assert( _head->next == _worst_node );
	assert( _head->ptr_start == _main_mem );
	assert( _head->size == req_size01 );

	assert(_worst_node == _head->next);
	assert(_worst_node->i == 1);
	assert(_worst_node->is_free == true);
	assert(_worst_node->next == node_req05);
	assert(_worst_node->prev == _head);
	assert(_worst_node->ptr_start == _main_mem + req_size01);
	assert(_worst_node->size == req_size02 + req_size03 + req_size04);

	assert(node_req05->i == 2);
	assert(node_req05->is_free == false);
	assert(node_req05->next == worst_before);
	assert(node_req05->prev == _worst_node);
	assert(node_req05->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 + req_size04);	
	assert(node_req05->size == req_size05);

	assert( worst_before == node_req05->next );
	assert( worst_before->i == 3 );
	assert( worst_before->is_free == true );
	assert( worst_before->next == NULL );
	assert( worst_before->prev == node_req05 );
	assert( worst_before->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 + req_size04 + req_size05);
	assert( worst_before->size == block_size - (req_size01 + req_size02 + req_size03 + req_size04 + req_size05));
}

void givenFreeBlockBeforeWorstAndAfterNotFreed_returnFreedBlockMergedWithWorst(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 30;
	int req_size03 = 200;
	int req_size04 = 25;
	int req_size05 = 25;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	struct node * worst_before = node_req05->next;
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req05);
	
	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(_worst_node == node_req04->next);
	assert(_worst_node->i == 4);
	assert(_worst_node->is_free == true);
	assert(_worst_node->next == NULL);
	assert(_worst_node->prev == node_req04);
	assert(_worst_node->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 + req_size04);
	assert(_worst_node->size == block_size - (req_size01 + req_size02 + req_size03 + req_size04));

}

void givenFreeBlockAfterNotFreedAndBeforeFreedAwayFromWorstAndLessThanWorst_returnBlockIsMergedWithNextFreeBlock(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 30;
	int req_size03 = 50;
	int req_size04 = 100;
	int req_size05 = 25;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req04);
	//print_my_list();
	myfree(node_req03);
	//print_my_list();

	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(node_req03->i == 2);
	assert(node_req03->is_free == true);
	assert(node_req03->next == node_req05);
	assert(node_req03->prev == node_req02);
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02);	
	assert(node_req03->size == (req_size03 + req_size04));
}

void givenFreeBlockAfterNotFreedAndBeforeFreedAwayFromWorstAndBiggerThanWorst_returnBlockIsMergedWithNextFreeBlockAndIsTheNewWorst(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 20;
	int req_size02 = 30;
	int req_size03 = 200;
	int req_size04 = 50;
	int req_size05 = 25;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req04);
	//print_my_list();
	myfree(node_req03);
	//print_my_list();

	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(node_req03 == _worst_node);
	assert(node_req03->i == 2);
	assert(node_req03->is_free == true);
	assert(node_req03->next == node_req05);
	assert(node_req03->prev == node_req02);
	assert(node_req03->ptr_start == _main_mem + req_size01 + req_size02);	
	assert(node_req03->size == (req_size03 + req_size04));
}

void givenFreeMiddleBlockBetweenFreedBlocksInHeadInTotalBiggerThanWorst_returnWorstInHead(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 150;
	int req_size02 = 50;
	int req_size03 = 150;
	int req_size04 = 30;
	int req_size05 = 20;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
//	printf("\n%s\n","After Setup");
//	print_my_list();

	//act
	myfree(node_req01);
//	print_my_list();
	myfree(node_req04);
//	print_my_list();
	myfree(node_req03);
//	print_my_list();
	myfree(node_req02);
//	print_my_list();

	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(node_req01 == _worst_node);
	assert(_head == _worst_node);

	assert(_worst_node->i == 0);
	assert(_worst_node->is_free == true);
	assert(_worst_node->next == node_req05);
	assert(_worst_node->prev == NULL);
	assert(_worst_node->ptr_start == _main_mem);	
	assert(_worst_node->size == (req_size01 + req_size02 + req_size03 + req_size04));
	
	assert(node_req05->prev == _worst_node);
}

void givenRemovefTotalSizeBiggerThenWorst_returnNewBlockIsWorst(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 150;
	int req_size02 = 50;
	int req_size03 = 120;
	int req_size04 = 30;
	int req_size05 = 20;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req03);
	//print_my_list();
	myfree(node_req04);
	//print_my_list();

	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(node_req03 == _worst_node);
	assert(_head == node_req01);

	assert(_worst_node->i == 2);
	assert(_worst_node->is_free == true);
	assert(_worst_node->next == node_req05);
	assert(_worst_node->prev == node_req02);
	assert(_worst_node->ptr_start == _main_mem + req_size01 + req_size02);	
	assert(_worst_node->size == (req_size03 + req_size04));

	assert(node_req05->prev == _worst_node);
}

void givenFreeTheFirstBlockSoThatItBecomesWorstAndAlocateNewBlock_returnTheBlockIsAlocatedFromTheNewWorst(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 150;
	int req_size02 = 50;
	int req_size03 = 120;
	int req_size04 = 30;
	int req_size05 = 20;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req01);
	//print_my_list();
	int req_size_new01 = 1;
	struct node * node_new01 = (struct node *)mymalloc(req_size_new01);
	//print_my_list();

	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(node_new01->next == _worst_node);
	assert(node_new01->prev == NULL);
	assert(_head == node_new01);
	assert(node_new01->i == 0);
	assert(node_new01->is_free == false);
	assert(node_new01->ptr_start == _main_mem);
	assert(node_new01->size == req_size_new01);

	assert(_worst_node->i == 1);
	assert(_worst_node->is_free == true);
	assert(_worst_node->next == node_req02);
	assert(_worst_node->prev == node_new01);
	assert(_worst_node->ptr_start == _main_mem + req_size_new01);	
	assert(_worst_node->size ==  req_size01 - req_size_new01);

	assert(node_req02->i == 2);
}

void givenFreeTheSecondBlockSoThatItBecomesWorstAndAlocateANewBlock_returnTheBlockIsAlocatedFromTheNewWorst(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 50;
	int req_size02 = 150;
	int req_size03 = 120;
	int req_size04 = 30;
	int req_size05 = 20;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	//print_my_list();
	int req_size_new01 = 1;
	struct node * node_new01 = (struct node *)mymalloc(req_size_new01);
	//print_my_list();

	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(node_new01->next == _worst_node);
	assert(node_new01->prev == node_req01);
	assert(_head == node_req01);
	assert(node_new01->i == 1);
	assert(node_new01->is_free == false);
	assert(node_new01->ptr_start == _main_mem + req_size01);
	//printf("\nnode_new01->ptr_start = %p\n", node_new01->ptr_start);
	assert(node_new01->size == req_size_new01);

	assert(_worst_node->i == 2);
	assert(_worst_node->is_free == true);
	assert(_worst_node->next == node_req03);
	assert(_worst_node->prev == node_new01);
	assert(_worst_node->ptr_start == _main_mem + req_size01 + req_size_new01);
	//printf("\n_worst_node->ptr_start = %p\n", _worst_node->ptr_start);	
	assert(_worst_node->size ==  req_size02 - req_size_new01);

	assert(node_req03->i == 3);
}

void givenCreateNewWorstByCallingMymalloc_returnNextWorstIsChosen(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size); //init mem

	int req_size01 = 50;
	int req_size02 = 150;
	int req_size03 = 120;
	int req_size04 = 30;
	int req_size05 = 20;	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02); 
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	struct node * node_req05 = (struct node *)mymalloc(req_size05);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	myfree(node_req02);
	///print_my_list();
	int req_size_new01 = 100;
	struct node * node_new01 = (struct node *)mymalloc(req_size_new01);
	//print_my_list();
	//printf("\n%s\n","After Act");
	//print_my_list();
	
	//assert
	assert(_worst_node == node_req05->next);
	assert(_worst_node->i == 6);
	assert(_worst_node->is_free == true);
	assert(_worst_node->next == NULL);
	assert(_worst_node->prev == node_req05);
	assert(_worst_node->ptr_start == _main_mem + req_size01 + req_size02 + req_size03 + req_size04 + req_size05);
	assert(_worst_node->size ==  block_size - (req_size01 + req_size02 + req_size03 + req_size04 + req_size05));
}

void givenZeroHoles_returnZeroHoles(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size);

	int req_size01 = 500;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	int holes =	mem_holes();

	//assert
	assert(holes == 0);
}

void givenOneHole_returnOneHole(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size);

	int req_size01 = 400;
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	int holes =	mem_holes();

	//assert
	assert(holes == 1);
}

void givenMoreThanOneHoles_returnHolesCount(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size);

	int req_size01 = 100;
	int req_size02 = 40;
	int req_size03 = 5;
	int req_size04 = 55;
	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	struct node * node_req02 = (struct node *)mymalloc(req_size02);
	struct node * node_req03 = (struct node *)mymalloc(req_size03);
	struct node * node_req04 = (struct node *)mymalloc(req_size04);
	myfree(node_req01);
	myfree(node_req03);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	int holes =	mem_holes();

	//assert
	assert(holes == 3);
}

void givenAlocatedBlockAndPointerInIt_returnBlockIsAlocated(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size);

	int req_size01 = 500;
	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	//myfree(node_req01);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	char is_alloc01 = mem_is_alloc(node_req01->ptr_start);
	char is_alloc02 = mem_is_alloc(node_req01->ptr_start + 5);
	//assert
	assert(is_alloc01 == 1 && "It is alocated == 1, is not alocated == 0");
	assert(is_alloc02 == 1 && "It is alocated == 1, is not alocated == 0");
}

void givenFreeBlockAndPointerInIt_returnBlockIsFree(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size);

	int req_size01 = 500;
	
	struct node * node_req01 = (struct node *)mymalloc(req_size01);
	myfree(node_req01);
	//printf("\n%s\n","After Setup");
	//print_my_list();

	//act
	char is_alloc01 = mem_is_alloc(_worst_node->ptr_start);
	char is_alloc02 = mem_is_alloc(_worst_node->ptr_start + 5);
	//assert
	assert(is_alloc01 == 0 && "It is alocated == 1, is not alocated == 0");
	assert(is_alloc02 == 0 && "It is alocated == 1, is not alocated == 0");
}


void givenCreate10EqualBlocksAndFreeEverySecond_returnCorrectLocationsToFreedBlocksInMemory(){
	//setup
	strategies strat = Worst;
	int block_size = 10;		
	initmem(strat, block_size);

	int req_size01 = 1, req_size02 = 1, req_size03 = 1, req_size04 = 1,req_size05 = 1;
	int req_size06 = 1, req_size07 = 1, req_size08 = 1, req_size09 = 1, req_size10 = 1;
	
	char * node01_loc = (char *)mymalloc(req_size01);
	char * node02_loc = (char *)mymalloc(req_size02);
	char * node03_loc = (char *)mymalloc(req_size03);
	char * node04_loc = (char *)mymalloc(req_size04);
	char * node05_loc = (char *)mymalloc(req_size05);
	char * node06_loc = (char *)mymalloc(req_size06);
	char * node07_loc = (char *)mymalloc(req_size07);
	char * node08_loc = (char *)mymalloc(req_size08);
	char * node09_loc = (char *)mymalloc(req_size09);
	char * node10_loc = (char *)mymalloc(req_size10);
	
	myfree(node02_loc);
	myfree(node04_loc);
	myfree(node06_loc);
	myfree(node08_loc);
	myfree(node10_loc);
	//printf("\n%s\n","After Setup");
	print_my_list();
	/*
	//act
	char is_alloc01 = mem_is_alloc(_worst_node->ptr_start);
	char is_alloc02 = mem_is_alloc(_worst_node->ptr_start + 5);
	//assert
	assert(is_alloc01 == 0 && "It is alocated == 1, is not alocated == 0");
	assert(is_alloc02 == 0 && "It is alocated == 1, is not alocated == 0");*/
}


int last_test_main_loop(){
		strategies strat = Worst;
		void* lastPointer = NULL;
		initmem(strat,100);
		for (int i = 0; i < 100; i++)
		{
			void* pointer = mymalloc(1);
			if ( i > 0 && pointer != (lastPointer+1) )
			{
				printf("Allocation with %s was not sequential at %i; expected %p, actual %p\n", 
				strategy_name(strat), i,lastPointer+1,pointer);
				return 1;
			}
			lastPointer = pointer;
		}
		print_my_list();
		return 0;
}

void givenScatteredFreeBlocksInTotalSizeBiggerThanRequested_returnCompactAndSatisfyTheRequest(){
	//setup
	strategies strat = Worst;
	int block_size = 1500;		
	initmem(strat, block_size);

	int req_size01 = 100; 
	int req_size02 = 200; 
	int req_size03 = 300;
	int req_size04 = 400; 
	int req_size05 = 500;
	
	char * node01_loc = (char *)mymalloc(req_size01);
	char * node02_loc = (char *)mymalloc(req_size02);
	char * node03_loc = (char *)mymalloc(req_size03);
	char * node04_loc = (char *)mymalloc(req_size04);
	char * node05_loc = (char *)mymalloc(req_size05);
	
	myfree(node02_loc);
	myfree(node04_loc);

	char * node_new = (char *)mymalloc(600);
	if(node_new == NULL){
		printf("\nnode_new == NULL\n");
	}
	//printf("\n%s\n","After Setup");
	print_my_list();
	/*
	//act
	char is_alloc01 = mem_is_alloc(_worst_node->ptr_start);
	char is_alloc02 = mem_is_alloc(_worst_node->ptr_start + 5);
	//assert
	assert(is_alloc01 == 0 && "It is alocated == 1, is not alocated == 0");
	assert(is_alloc02 == 0 && "It is alocated == 1, is not alocated == 0");*/
}


void givenCompactionRequested_returnCompacted(){
	//setup
	strategies strat = Worst;
	int block_size = 500;		
	initmem(strat, block_size);

	int req_size01 = 100; 
	int req_size02 = 50; 
	int req_size03 = 175;
	int req_size04 = 25;
	
	char * node01_loc = (char *)mymalloc(req_size01);
	char * node02_loc = (char *)mymalloc(req_size02);
	char * node03_loc = (char *)mymalloc(req_size03);
	char * node04_loc = (char *)mymalloc(req_size04);
	
	myfree(node01_loc);

	printf("\n%s\n","After Setup");
	print_my_list();


	char * node05_loc = (char *)mymalloc(200);

	//compact(200);
	
	printf("\n%s\n","After Compaction");
	print_my_list();


	printf("\n%s%p\n","After freeing mapped memory = ", node03_loc);

	myfree(node03_loc);

	printf("\n%s\n","After freeing mapped memory");
	print_my_list();
}

int main2(){
	//givenCreate10EqualBlocksAndFreeEverySecond_returnCorrectLocationsToFreedBlocksInMemory();
	
	if(UNIT_TEST){
		//givenCompactionRequested_returnCompacted();
		//givenScatteredFreeBlocksInTotalSizeBiggerThanRequested_returnCompactAndSatisfyTheRequest();
		//last_test_main_loop()();
		
		givenFreeBlockAndPointerInIt_returnBlockIsFree();
		givenAlocatedBlockAndPointerInIt_returnBlockIsAlocated();

		givenMoreThanOneHoles_returnHolesCount();
		givenOneHole_returnOneHole();
		givenZeroHoles_returnZeroHoles();
		
		givenCreateNewWorstByCallingMymalloc_returnNextWorstIsChosen();
		givenFreeTheSecondBlockSoThatItBecomesWorstAndAlocateANewBlock_returnTheBlockIsAlocatedFromTheNewWorst();
		givenFreeTheFirstBlockSoThatItBecomesWorstAndAlocateNewBlock_returnTheBlockIsAlocatedFromTheNewWorst();

		givenRemovefTotalSizeBiggerThenWorst_returnNewBlockIsWorst();
		givenFreeMiddleBlockBetweenFreedBlocksInHeadInTotalBiggerThanWorst_returnWorstInHead();
		givenFreeBlockAfterNotFreedAndBeforeFreedAwayFromWorstAndBiggerThanWorst_returnBlockIsMergedWithNextFreeBlockAndIsTheNewWorst();
		givenFreeBlockAfterNotFreedAndBeforeFreedAwayFromWorstAndLessThanWorst_returnBlockIsMergedWithNextFreeBlock();
		givenFreeBlockBeforeWorstAndAfterNotFreed_returnFreedBlockMergedWithWorst();
		givenFreeBlockBetweenFreedBlocksInMiddleOfListButNotTheWorstSoThatNewBlockBiggerThanWorst_returnWorstIsTheNewBiggerFreedBlock();
		givenFreeBlockBetweenFreedBlocksButNotTheWorstSoThatNewBlockBiggerThanWorst_returnWorstIsTheNewBiggerFreedBlock();
		givenFreeBlockBetweenFreedBlocksButNotTheWorstSoThatNewBlockLessThanWorst_returnWorstUnchangedNewBiggerFreedBlockInsteadOfTheTreeBlocks();

		givenAddThreeBlocksFreeSecondAndThenThirdInTotalSizeLessThanTotalSoThatWorstIsInEnd_returnFirstIsThereSecondIsWorst();
		givenAddThreeBlocksFreeSecondAndThenFirstThenLastInTotalSizeLessThanTotalSoThatWorstIsInEnd_returnInitialState();
		givenAddThreeBlocksFreeSecondAndThenFirstThenLastInTotalSizeLessThanTotal_returnInitialState();
		givenAddThreeBlocksFreeSecondAndThenFirstInTotalSizeBiggerThanWorst_returnHeadBlockOfNewSizeSumOfFirstAndSecondAndFreedAndWorstChangedToHead();	
		givenAddThreeBlocksFreeSecondAndThenFirstInTotalSizeLessThanWorst_returnHeadBlockOfNewSizeSumOfFirstAndSecondAndFreedAndWorstNotChanged();	givenAddThreeBlocksOfTotalSizeLessThenTheMaxAndFreeTheSecondBlockSmallerThanTheCurrentWorst_returnTheSecondIsFreedAndTheFirstAndLastAreThereNotFreedAndWorstIsNotChangedBlock();
		givenAddThreeBlocksOfTotalSizeLessThenTheMaxAndFreeTheSecondBlockBiggerThanTheCurrentWorst_returnTheSecondIsFreedAndTheFirstAndLastAreThereNotFreedAndWorstIsTheFreedSecondBlock();

		givenAddThreeBlocksOfTotalSizeToTheMaxAndRemoveTheFirstBlock_returnTheFirstIsFreedAndTheSecondAndLastAreThereNotFreed();
		givenAddThreeBlocksOfTotalSizeToTheMaxAndRemoveTheLastBlock_returnTheLastIsFreedAndTheFirstAndSecondAreThereNotFreed();
		givenAddThreeBlocksOfTotalSizeToTheMaxAndRemoveTheSecondBlock_returnTheSecondIsFreedAndTheFirstAndLastAreThereNotFreed();
		givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheBlocksInReverseOrder_returnTheInitialStatus();
		givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheSecondBlock_returnTheSecondIsFreedAndTheFirstIsThereNotFreed();
		givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheBlocks_returnTheInitialStatus();
		givenAddTwoBlocksOfTotalSizeEqualToTheMaxAndRemoveTheFirstBlock_returnTheFirstIsFreedAndTheSecondIsThereNotFreed();
		givenAddOneBlockOfSizeLessThanMaxAndRemoveTheBlock_returnInitialStatus();
		givenAddOneBlockMaxSizeAndRemoveTheBlock_returnInitialStatus();
		
		given4BlocksRequestedOfTotalSizeOfTheTotalMemory_return4NodesCreatedWithNoFreeSpaceInMemory();
		given3BlocksRequestedOfTotalSizeOfTheTotalMemory_return3NodesCreatedWithNoFreeSpaceInMemory();
		given2BlocksRequestedOfTotalSizeOfTheTotalMemory_return2NodesCreatedWithNoFreeSpaceInMemory();
		givenRequestMoemoryLessThanTheAvailable_returnTheNodeAndWorstNodeIsNULL();
		givenRequestMoemoryMoreThanAvailable_returnNULL();
		givenAnyBlockIsRequestedWhenNoSpaceInMemory_returnNULL();
		givenBlockSizeIsMaxRequested_returnNodeMaxSize();
		givenInitMemoryWithSize_returnEmptyBlockWithSizeAlocated();
				
	}else
	{
	//do_randomized_test(strategyToUse, totalSize, fillRatio, minBlockSize, maxBlockSize, iterations);
		my_do_randomized_test(Worst,      10000,     0.25,      1,            3000,         10000);
	}
	return 0;
}

void my_do_randomized_test(int strategyToUse, int totalSize, float fillRatio, int minBlockSize, int maxBlockSize, int iterations)
{
	printf("\n%s\n", "Check tests.log for the resut!");
	void * pointers[10000];
	int storedPointers = 0;
	int strategy;
	int lbound = 1;
	int ubound = 4;
	int smallBlockSize = maxBlockSize/10;

	if (strategyToUse>0)
		lbound=ubound=strategyToUse;

	FILE *log;
	log = fopen("tests.log","a");
	if(log == NULL) {
	  perror("Can't append to log file.\n");
	  return;
	}

	fprintf(log,"Running randomized tests: pool size == %d, fill ratio == %f, block size is from %d to %d, %d iterations\n",totalSize,fillRatio,minBlockSize,maxBlockSize,iterations);

	fclose(log);

	for (strategy = lbound; strategy <= ubound; strategy++)
	{
		double sum_largest_free = 0;
		double sum_hole_size = 0;
		double sum_allocated = 0;
		int failed_allocations = 0;
		double sum_small = 0;
		struct timespec execstart, execend;
		int force_free = 0;
		int i;
		storedPointers = 0;
		log = fopen("tests.log","a");
		
		initmem(strategy,totalSize);

		clock_gettime(CLOCK_REALTIME, &execstart);

		for (i = 0; i < iterations; i++)
		{
			if ( (i % 10000)==0 )
				srand ( time(NULL) );
			if (!force_free && (mem_free() > (totalSize * (1-fillRatio))))
			{
				int newBlockSize = (rand()%(maxBlockSize-minBlockSize+1))+minBlockSize;
				/* allocate */
				void * pointer = mymalloc(newBlockSize);
				if (pointer != NULL)
					pointers[storedPointers++] = pointer;
				else
				{ 
					fprintf(log,"\ntotalSize = %d, fillRatio = %.2f, minBlockSize = %d, maxBlockSize = %d, iterations = %d\n", totalSize, fillRatio, minBlockSize, maxBlockSize, iterations);
					fprintf(log,"\nnewBlockSize = %d, i = %d\n", newBlockSize, i);
					failed_allocations++;
					force_free = 1;
				}
			}
			else
			{
				int chosen;
				void * pointer;

				/* free */
				force_free = 0;

				if (storedPointers == 0)
					continue;

				chosen = rand() % storedPointers;
				pointer = pointers[chosen];
				pointers[chosen] = pointers[storedPointers-1];

				storedPointers--;

				myfree(pointer);
			}

			sum_largest_free += mem_largest_free();
			sum_hole_size += (mem_free() / mem_holes());
			sum_allocated += mem_allocated();
			sum_small += mem_small_free(smallBlockSize);
		}

		clock_gettime(CLOCK_REALTIME, &execend);

		//log = fopen("tests.log","a");
		if(log == NULL) {
		  perror("Can't append to log file.\n");
		  return;
		}
		
		fprintf(log,"\t=== %s ===\n",strategy_name(strategy));
		fprintf(log,"\tTest took %.2fms.\n", (execend.tv_sec - execstart.tv_sec) * 1000 + (execend.tv_nsec - execstart.tv_nsec) / 1000000.0);
		fprintf(log,"\tAverage hole size: %f\n",sum_hole_size/iterations);
		fprintf(log,"\tAverage largest free block: %f\n",sum_largest_free/iterations);
		fprintf(log,"\tAverage allocated bytes: %f\n",sum_allocated/iterations);
		fprintf(log,"\tAverage number of small blocks: %f\n",sum_small/iterations);
		fprintf(log,"\tFailed allocations: %d\n",failed_allocations);
		fclose(log);


	}
}