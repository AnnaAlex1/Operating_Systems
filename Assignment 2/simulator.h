#ifndef __SIMUL__
#define __SIMUL__

#include <stdbool.h>


struct Statistics{
    
    int w_counter;      //counts the number of references for writing
    int r_counter;      //counts the number of references for reading
    int page_faults;    //counts how many page-faults occured 
                          //maybe not just the number?
    int saves_in_disk;  //counts the number of writings to the disk
    int readings_from_disk; //counts the number of readings from the disk 
    //?                        //καταχωρήσεις που εξετάστηκαν από το αρχείο ίχνους αναφορών

};


struct Index{
    int page_number;
    int last_used; //number that represents when this page was used most recently
    bool changed;   //variable that shows if page was altered or not
    int processid;
    int sec_ch_bit;      //for second chance algorithm
};



void init_index(struct Index *indexes);
void update_index(struct Index *indexes, char* algorithm, int page_num, char op, int reading, int position, int processid);
void init_statistics(struct Statistics statistics);

void print_statistics(void);
int parser(FILE* file, char* address, char *operation);
void split_address(char* address, int *page_num, int *offset);




#endif