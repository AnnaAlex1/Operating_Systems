#include "lru.h"
#include "hashtable.h"
#include "simulator.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int num_of_frames;
extern struct Index* indexes;
extern struct Statistics statistics;




void LRU_algorithm(struct Table* ptable1, struct Table* ptable2, int page_num, char op, int new_pid, int reading){    //reading: how many times we performed a file reading

    int min_position = 0;
    int minimum = indexes[0].last_used;
    int pid = indexes[0].processid;


    //find least recently used page
    for (int i=0; i<num_of_frames; i++){
        if (indexes[i].last_used < minimum){
            minimum = indexes[i].last_used;
            min_position = i;
            pid = indexes[i].processid;
        }
    }


    //increase number of writings in the disk if page was changed
    if ( indexes[min_position].changed){    //if the page was changed
        statistics.saves_in_disk++;
    }

    //Remove from the Hashed Page Table the page to be replaced in frames
    if (pid == 1){    //remove from hashed page table 1

        if ( !delete_from_hashtable(ptable1, indexes[min_position].page_number ) ){
            printf("Page with number %d not found in the Hashed page Table 1 :(\n", page_num);
        }

    } else {                //remove from hashed page table 2

        if ( !delete_from_hashtable(ptable2, indexes[min_position].page_number ) ){
            printf("Page with number %d not found in the Hashed page Table 2 :(\n", page_num);
        }

    }

    //Insert page in Hashed Page Table
    if (new_pid == 1){
        insert_in_hashtable(ptable1, page_num, min_position);
    }else {
        insert_in_hashtable(ptable2, page_num, min_position);
    }

    //Update index structure
    update_index(indexes, "LRU", page_num, op, reading, min_position, new_pid);


}

