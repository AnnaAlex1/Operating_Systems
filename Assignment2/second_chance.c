#include "second_chance.h"
#include "hashtable.h"
#include "simulator.h"

#include <stdio.h>

extern int num_of_frames;
extern struct Index* indexes;
extern struct Statistics statistics;



void SECC_algorithm(struct Table* ptable1, struct Table* ptable2, int page_num, char op, int new_pid, int reading){
    static int pointer = 0;
    int pid;

    //find page to be removed
    while ( indexes[pointer].sec_ch_bit != false ){      //search for a victim
        indexes[pointer].sec_ch_bit = false;
        pointer = (pointer + 1) % num_of_frames;
    }

    pid = indexes[pointer].processid;

    //increase number of writings in the disk if page was changed
    if ( indexes[pointer].changed){    //if the page was changed
        statistics.saves_in_disk++;
    }

    //Remove from the Hashed Page Table the page to be replaced in frames
    if (pid == 1){    //remove from hashed page table 1

        if ( !delete_from_hashtable(ptable1, indexes[pointer].page_number ) ){
            printf("Page with number %d not found in the Hashed page Table 1 :(\n", page_num);
        }

    } else {                //remove from hashed page table 2

        if ( !delete_from_hashtable(ptable2, indexes[pointer].page_number ) ){
            printf("Page with number %d not found in the Hashed page Table 2 :(\n", page_num);
        }

    }


    //Insert page in Hashed Page Table
    if (new_pid == 1){
        insert_in_hashtable(ptable1, page_num, pointer);
    }else {
        insert_in_hashtable(ptable2, page_num, pointer);
    }

    //Update index structure
    update_index(indexes, "SECC", page_num, op, reading, pointer, new_pid);     //+set second chance bit to one

    pointer = (pointer + 1) % num_of_frames;


}