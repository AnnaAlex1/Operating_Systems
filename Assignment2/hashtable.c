#include "hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>




int hashfunction(int num){
    return num%TABLE_SIZE;
    
}


    // VIRTUAL PAGE NUM = HASH VALUE????????????

void insert_in_hashtable(struct Table* hashtable, int vir_page_num, int frame_num){

    //allocate the bucket
    struct Bucket* new_bucket = malloc(sizeof(struct Bucket)); 

    //set the bucket
    new_bucket->page_num = vir_page_num; //hashvalue???
    new_bucket->frame_num = frame_num;
    new_bucket->nextbuck = NULL;


    //find corresponding entry in hashtable
    int row;                                   //row of hashtable (which chain to follow)
    struct Bucket* current_bucket;             //the bucket in which the record will be stored 

    row = hashfunction(vir_page_num);
    printf("Inserting page %d in row %d\n", vir_page_num, row);


    //put in hash table
    if (hashtable[row].bucket == NULL){     //case: entry has no buckets
        hashtable[row].bucket = new_bucket;
    } else {                                //case: entry already has buckets

        current_bucket = hashtable[row].bucket;        
        while ( current_bucket->nextbuck != NULL ){
            current_bucket = current_bucket->nextbuck;
        }

        current_bucket->nextbuck = new_bucket;

    }
    
    return;


}


bool in_hashtable(struct Table* hashtable, int page_num, int* frame) {

    if (hashtable == NULL){
        return false;
    }

    int row = hashfunction(page_num);                   //row of hashtable (which chain to follow)
    
    struct Bucket* current_bucket; 

    current_bucket = hashtable[row].bucket;

    while ( current_bucket != NULL){

        if ( current_bucket->page_num == page_num ){
            *frame = current_bucket->frame_num;
            return true;
        }

        current_bucket = current_bucket->nextbuck;

    }

    return false;

}



struct Bucket* get_page(struct Table* hashtable, int page_num){

    int row = hashfunction(page_num);                   //row of hashtable (which chain to follow)
    
    struct Bucket* current_bucket;
    struct Bucket* page_ptr;         

    current_bucket = hashtable[row].bucket;

    while ( current_bucket != NULL){

        if ( current_bucket->page_num == page_num ){
            page_ptr = malloc(sizeof(struct Bucket));
            memcpy(page_ptr, current_bucket, sizeof(struct Bucket));
            return page_ptr;
        }

        current_bucket = current_bucket->nextbuck;

    }

    return NULL;
}


bool delete_from_hashtable(struct Table* hashtable, int page_num){

    int row = hashfunction(page_num);                   //row of hashtable (which chain to follow)
    
    struct Bucket* current_bucket, *temp;
        
    printf("Deleting page %d from row %d\n", page_num, row);

    if (hashtable[row].bucket == NULL){
        printf("Chain in row %d is empty\n", row);
        return false;
    }

    
    if ( hashtable[row].bucket->page_num == page_num){  //case: in first bucket of chain
        temp = hashtable[row].bucket->nextbuck;
        free(hashtable[row].bucket);
        hashtable[row].bucket = temp;
        return true;
    }


    current_bucket = hashtable[row].bucket;


    while ( current_bucket->nextbuck != NULL){

        if ( current_bucket->nextbuck->page_num == page_num ){
            temp = current_bucket->nextbuck->nextbuck;
            free(current_bucket->nextbuck);
            current_bucket->nextbuck = temp;
            return true;
        }

        current_bucket = current_bucket->nextbuck;

    }

    return false;   //not found


}
