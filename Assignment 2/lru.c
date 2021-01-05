#include "lru.h"
#include "hashtable.h"
#include "simulator.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int num_of_frames;
extern struct Index *index;
extern struct Statistics statistics;




void LRU_algorithm(struct Table* ptable, int page_num, char op, int reading){    //reading: how many times we performed a file reading

    int min_position = 0;
    int minimum = index[0].last_used;

    for (int i=0; i<num_of_frames; i++){
        if (index[i].last_used < minimum){
            minimum = index[i].last_used;
            min_position = i;
        }
    }

    //replaceinhashtable()
    if ( !delete_from_hashtable(ptable, index[min_position].page_number ) ){
        printf("Page with number %d not found in the Hashed page Table :(\n", page_num);
    }
    
    insert_in_hashtable(ptable, page_num, min_position);

    update_index(index, page_num, op, reading, min_position);



}

