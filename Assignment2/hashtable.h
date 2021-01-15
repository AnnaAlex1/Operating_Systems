#ifndef __HASH__
#define __HASH__

#define TABLE_SIZE 500   //number of rows in the hashtable

#include <stdbool.h>

struct Table{
    struct Bucket *bucket;
};

struct Bucket{
    int page_num;
    int frame_num;
    struct Bucket *nextbuck;
};



int hashfunction(int num);
void insert_in_hashtable(struct Table* hashtable, int vir_page_num, int frame_num);
bool in_hashtable(struct Table* hashtable, int page_num, int* frame);
bool delete_from_hashtable(struct Table* hashtable, int page_num);


#endif