#include "hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


struct Table* init_hashtable(struct Table* hashtable){
    
    hashtable = malloc(sizeof(struct Table)*TABLE_SIZE);    //allocation of the table

    for (int i=0; i<TABLE_SIZE; i++){
        hashtable[i].bucket = malloc(sizeof(struct Bucket));   //allocation of the first bucket for each row
        init_bucket( hashtable[i].bucket );
    }

    //bucket_capacity = BUCKETSIZE / sizeof(struct Bucket);

    return hashtable;

}


void init_bucket(struct Bucket* bucket){
    bucket->num_of_pages = 0;
    bucket->nextbuck = NULL;
    bucket->pages = malloc(sizeof(struct Page)*BUC_CAPACITY);
}


int hashfunction(int num){
    return num%TABLE_SIZE;
    
}



void insert_in_hashtable(struct Table* hashtable, struct Page new_page){
    int row;                                   //row of hashtable (which chain to follow)
    struct Bucket* current_bucket;             //the bucket in which the record will be stored 

    row = hashfunction(new_page.page_num);
    current_bucket = hashtable[row].bucket;         //first bucket of the chain

    int place_in_bucket = 0;

    while (current_bucket->nextbuck != NULL){                   //stop if it's the last bucket
        if (current_bucket->num_of_pages != BUC_CAPACITY){   //stop if you find a bucket with space
            place_in_bucket = current_bucket->num_of_pages;
            break;
        }
        current_bucket = current_bucket->nextbuck;

    }

    if (current_bucket->num_of_pages == BUC_CAPACITY){   //if the bucket is full (case: last one and full)
        current_bucket->nextbuck = malloc(sizeof(struct Bucket));
        init_bucket(current_bucket->nextbuck);
        current_bucket = current_bucket->nextbuck;
    }

    
    memcpy( &(current_bucket->pages[place_in_bucket]), &new_page, sizeof(struct Page));
    current_bucket->num_of_pages++;
    return;

}


struct Page* get_page(struct Table* hashtable, int page_num){

    int row = hashfunction(page_num);                   //row of hashtable (which chain to follow)
    
    struct Bucket* current_bucket;
    struct Page* page_ptr;         

    current_bucket = hashtable[row].bucket;

    while ( current_bucket != NULL){

        int i=0;
        while ( i<current_bucket->num_of_pages ){       //for every record in the bucket
            
            if ( current_bucket->pages[i].page_num == -1 ){ //if position is empty, skip
                i++;
                continue;
            }

            if ( current_bucket->pages[i].page_num == page_num ){
                page_ptr = malloc(sizeof(struct Page));
                memcpy(page_ptr, &current_bucket->pages[i], sizeof(struct Page));
                return page_ptr;
            }

            i++;
        }



        current_bucket = current_bucket->nextbuck;


    }

    return NULL;
}

