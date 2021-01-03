#ifndef __HASH__
#define __HASH__

#define TABLE_SIZE 10    //number of rows in hte hashtable
#define BUC_CAPACITY 10 //number of records that fit in a bucket


struct Table{
    struct Bucket *bucket;
};

struct Bucket{
    struct Page* pages;
    int num_of_pages;
    struct Bucket *nextbuck;
};



struct Page{
    int page_num;
};



struct Table* init_hashtable(struct Table* hashtable);
void init_bucket(struct Bucket* bucket);
int hashfunction(int num);
void insert_in_hashtable(struct Table* hashtable, struct Page new_page);
struct Page* get_page(struct Table* hashtable, int page_num);


#endif