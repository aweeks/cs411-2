#include<stdio.h>
#include<stdlib.h>

#include "macros.h"
#include "list.h"

struct test_list {
    struct list_head list;
    int data;
};

int main() {
    struct test_list test, *temp, *temp2;
    struct list_head *pos, *q;
    
    INIT_LIST_HEAD(&(test.list));

    int n;

    for(n = 0; n < 10; n++) {
        temp = (struct test_list *) malloc( sizeof(struct test_list *)  );
        temp -> data = n;

        list_add( &(temp->list), &(test.list) );
    }
   /* 
    printf("\n\n");
    
    pos = & test.list;

    while( pos != NULL ) {
        temp = list_entry(pos, struct test_list, list);
        printf("%d\n", temp->data);

        pos = pos->next;

    }
*/

    list_for_each_entry(temp, &test.list, list) {
        printf("%d\n", temp->data);
    }

    printf("\n\n");
    list_for_each_entry_safe(temp, temp2, &test.list, list) {
        if (temp->data == 4) {
            list_del(& (temp->list) );
        }
    }


    list_for_each_entry(temp, &test.list, list) {
        printf("%d\n", temp->data);
    }

}
