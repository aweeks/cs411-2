 //Header Files
#include <stdio.h> 
#include <stdlib.h>
#include "list.h"

struct foobar{
	int a;
	int b;
	struct list_head list;
};

int main(int argc, char ** argv) {
	
	//Init a bunch of structures
	struct foobar * newElement  = (struct foobar *)malloc(sizeof(struct foobar));//If a pointer, must be malloced 
	newElement->a = 1;
	newElement->b = 2;
	struct foobar * currentItem  = (struct foobar *)malloc(sizeof(struct foobar));
	currentItem ->a = 3;
	currentItem ->b = 4;
	struct foobar * currentItem2  = (struct foobar *)malloc(sizeof(struct foobar));
	currentItem2 ->a = 5;
	currentItem2 ->b = 6;

	INIT_LIST_HEAD(&(newElement->list));
	list_add(&(currentItem->list), &(newElement->list));
	list_add(&(currentItem2->list), &(currentItem->list));
	
	struct foobar * temp;
	temp = list_entry(currentItem->list.next, struct foobar, list);
	//printf("a = %d\n",temp->a);
	
	list_for_each_entry(temp, &(newElement->list), list){
		printf("b = %d\n",temp->b);
	}
	
	/*
	 * Final List:
	 * 
	 * newElement -> currentItem -> currentItem2
	 * 
	 * Output
	 * ---
	 * b = 4
	 * b = 6
	 */
	
	return EXIT_SUCCESS;
}
