#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>

#include <stdbool.h>

void * alloc_worst(size_t requested);
void given_block_bigger_than_max_requested_return_null();
void clean_up();

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

strategies _strategy = 0;    // Current strategy 

size_t _main_mem_size_total;
//size_t _main_mem_size_free;
//size_t _main_mem_size_used;
char * _main_mem = NULL;

static struct node *_head;
static struct node *_next;

void free_list_from_head(){
	while (_head != NULL){
		struct node * tmp = _head;
		_head = _head->next;
		free(tmp);
	}
}
struct node * _worst_node;

void * alloc_worst(size_t req_size){
	if(_worst_node == NULL)
		return NULL;	
	else if(_worst_node->size < req_size)
		return NULL;	
	else if(_worst_node->size == req_size){ //exact size as the size of the worst node 
		if(_worst_node->prev == NULL){
			_worst_node->i = 0;
		}else{
			_worst_node->i = _worst_node->prev->i + 1;
		}
		_worst_node->is_free = false;
		struct node * tmp = _worst_node;
		_worst_node = NULL;
		return tmp;		
	}else if(_worst_node->size > req_size){
		struct node * new_node = calloc(1, sizeof(struct node) );
		new_node->size = req_size;
		new_node->is_free = false;	
		new_node->ptr_start = _worst_node->ptr_start;
		new_node->next = _worst_node;
		new_node->prev = _worst_node->prev;
		
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
		return new_node;
	}
}

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

	//clean
	//clean_up();
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
	
	//clean
	//clean_up();
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
    
	//clean
	//clean_up();
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
    
	//clean
	//clean_up();
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
    
	//clean
	//clean_up();
}

void print_my_list(){
	printf("\n_memory starts = %p\n", &_main_mem);
	for(struct node * p = _head; p != NULL; p = p->next)
		printf("\nnode: is_free = %s, size = %d, ptr_start = %p, i = %d\n", 
				p->is_free?"true":"false", 
				p->size,
				&p->ptr_start,
				p->i);
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
	
	//clean
	//clean_up();
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
	
	//clean
	//clean_up();
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
	printf("\n%s\n","After Setup");
	print_my_list();

	//act
	myfree(node_req02);

	printf("\n%s\n","After Act");
	print_my_list();

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

void clean_up(){
	//free(_main_mem);
	//free_list_from_head();
	_worst_node = NULL;
	_head = NULL;
	//_main_mem = NULL;
}

int main(){
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
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void * block)
{
	struct node * node_to_del = (struct node *)block;
	if(node_to_del->size == _main_mem_size_total){
		node_to_del->is_free = true;
		_worst_node = _head;
		return;
	}

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

	if(node_to_del->prev != NULL &&
	   node_to_del->prev->is_free == false && 
	   node_to_del->next == NULL &&
	   _worst_node == NULL){
		_worst_node = node_to_del;
		_worst_node->is_free = true;
		node_to_del = NULL;
		return;
	}

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

	if(node_to_del->prev == NULL && 
	   node_to_del->next != NULL && node_to_del->next->is_free == false){

		node_to_del->i = 0;
		node_to_del->is_free = true;
		node_to_del->ptr_start = _main_mem;

		if(_worst_node != NULL){
			free(_worst_node);
			_worst_node = NULL;
		}
		_worst_node = node_to_del;

		_worst_node = _head;

		return;
	}





	//if (_main_mem != NULL) free(_main_mem); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
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
	while (_head != NULL){
		void * tmp = _head;
		_head = _head->next;
		free(tmp);
		tmp = NULL; //make it a habit
	}
		
	/* Initialize memory management structure. */
	_main_mem = (char *)calloc(size, sizeof(char));
				
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

