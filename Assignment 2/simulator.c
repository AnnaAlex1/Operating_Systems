#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hashtable.h"
#include "second_chance.h"
#include "lru.h"


#define ADDRESS_SIZE 8                     //to_change
#define PAGE_SIZE 4096
#define FRAME_SIZE 4096
#define LINE_SIZE 50

int num_of_frames;
struct Index* index;
struct Statistics statistics;




int main(int argc, char* argv[]){

    //GHECKING ARGUMENTS
    if (argc != 4 && argc!= 5){
        printf("Wrong Number of Arguments!\n");
        return 1;

    }

    if ( ( strcmp(argv[1], "LRU") != 0) && ( strcmp(argv[1], "SECC") != 0) ){
        printf("Wrong input for algorithm value. Must be 'LRU'/'SECC'\n");
        return 1;
    }


    //GETTING ARGUMENTS
    char algorithm[10];
    int q;
    int max_ref=100;    //default value in case it's not given                //to change


    strcpy(algorithm, argv[1]);             //1st argument: algorithm to be used (LRU/Second Chance)
    num_of_frames = atoi(argv[2]);          //2nd argument: number of frames
    q = atoi(argv[3]);                      //3rd argument: number of references

    if (argc == 5) max_ref= atoi(argv[4]);    //case: 4 arguments given: max number of references was given   

 /* 
    printf("Algorithm to be used: %s\n", algorithm);
    printf("Number of frames in memory: %d\n", num_of_frames);
    printf("Number of references q: %d\n", q);
    printf("Maximum number of references: %d\n", max_ref);
*/

    /////////OPENING FILES
    FILE *file1, *file2;

    file1 = fopen("./bzip.trace", "r");
    if (file1 == NULL){
        perror("Error in opening 'bzip.trace'");
        return 1;
    }

    file2 = fopen("./gcc.trace", "r");
    if (file2 == NULL){
        perror("Error in opening 'gcc.trace'");
        return 1;
    }



    ////////////////////

    int cur_in_frames = 0;                  //counts nuber of pages currently in memory
    
    struct Table* hashed_ptable1;         //keeps pages in memory
    hashed_ptable1 = malloc(sizeof(struct Table)*TABLE_SIZE);    //allocation of the hashed page table for process1

    struct Table* hashed_ptable2;
    hashed_ptable2 = malloc(sizeof(struct Table)*TABLE_SIZE);    //allocation of the hashed page table for process2

    struct Table* ptable_ptr;            //hashtable to be used in this loop (points to hashed_ptable1/2)

    char address[ADDRESS_SIZE+1];
    char op;
    int page_num, offset;

    int process_num;    // 1: for process1 (file1), 2: for process2 (file2)
    bool finished1 = false, finished2 = false;  //helping variables for checking if the reading of the files is finished

    
    //Initialization of data structures
    index = malloc( sizeof(struct Index) *num_of_frames );
    init_index(index);

    init_statistics(statistics);


    int q_counter;
    int max_counter = 1;

    while ( max_counter < max_ref*2 ){    //max times for process 1 and max times for process 2
        
        process_num = 1;    //start with process1

        q_counter = 0;
        while (q_counter < 2*q){    // q references each time

            if (q_counter == q){    //after q readings, move to process 2
                process_num = 2;
            }


            /////////////////////////
            //Read from file
            if ( ( process_num == 1 ) && !finished1){   
                
                //read from file 1
                if ( parser(file1, address, &op) == -1 ){  //if file is finished
                    finished1 = true;
                    continue;
                }
                //calculate page number and offset
                split_address(address, &page_num, &offset);

                //choose hashtable
                ptable_ptr = hashed_ptable1;

                printf("File1 -> address: %s, op: %c\n", address,op);
                printf("Page number: %d\nOffset: %d\n", page_num, offset);
            
            } else if ( ( process_num == 2 ) && !finished2 ){
                
                //read from file 2
                if ( parser(file2, address, &op) == -1 ){  //if file is finished
                    finished2 = true;
                    continue;
                }
                //calculate page number and offset
                split_address(address, &page_num, &offset);

                //choose hashtable
                ptable_ptr = hashed_ptable2;

                printf("File2 -> address: %s, op: %c\n", address,op);
                printf("Page number: %d\nOffset: %d\n", page_num, offset);
            }
            ////////////////////////////
            
            if ( in_hashtable(ptable_ptr, page_num) ){  //check if the page is already in memory
                printf("Already in memory! Moving on...\n");


            } else {
                printf("Not in memory.\n");
                statistics.page_faults++;
                
                if (cur_in_frames < num_of_frames ){ //if memory frames are not all occupied
                    printf("There is space in frames. Placing...\n");
                    insert_in_hashtable(ptable_ptr, page_num, cur_in_frames);
                    update_index(index, page_num, op, cur_in_frames, cur_in_frames);
                    cur_in_frames++;
                } else {                              //if memory is full
                    printf("No space in frames! Executing the algorithm...\n");
                    if ( strcmp(algorithm, "LRU") == 0){
                        LRU_algorithm(ptable_ptr, page_num, max_counter, op);
                    } else {
                        SECC_algorithm(ptable_ptr, page_num, max_counter, op);
                    }
                }
            }


            //--ενημέρωση πίνακα σελίδων??
            //αν έχουμε εγγραφή στο δίσκο:          w_counter++;
            //αν έχουμε ανάγνωση από το δίσκο:      r_counter++;

            max_counter++;
            q_counter++;
            printf("\n");
        }

    
    }
    
    
    
    
    
    
    ///////STATISTICS
    print_statistics();


    //closing files
    fclose(file1);
    fclose(file2);

}



void init_index(struct Index *index){
    for (int i=0; i<num_of_frames; i++){
        index[i].page_number=-1;
        index[i].last_used=-1;
        index[i].changed=-1;
    }
}

void init_statistics(struct Statistics statistics){
    statistics.w_counter = 0;
    statistics.r_counter = 0;
    statistics.page_faults = 0;
}


void print_statistics(void){

    printf("\n\nSTATISTICS\n");
    printf("Number of writings to the disk performed: %d\n", statistics.w_counter);
    printf("Number of readings from the disk performed: %d\n", statistics.r_counter);
    printf("Number of page-faults that occured: %d\n", statistics.page_faults);
    printf("Number of frames in memory: %d\n", num_of_frames);
    
}

int parser(FILE* file, char* address, char *operation){

    char line[LINE_SIZE];
    if ( fgets(line, LINE_SIZE, file) != NULL){
        sscanf(line,"%s %c", address, &(*operation));
        return 0;
    }

    return -1;

}

void split_address(char* address, int *page_num, int *offset){


    int offset_hex = (log2(PAGE_SIZE)) / 4;             // offset bytes / 4
    int p_addr_hex = 8 - offset_hex;           //page address bytes / 4
    
    //printf("Offset size: %d characters\nPage Address size: %d characters\n",offset_hex, p_addr_hex);


    char temp_padd[p_addr_hex+1];
    char temp_offset[offset_hex+1];

    int i;

    for (i=0; i<p_addr_hex; i++){
        temp_padd[i] = address[i];
    }
    temp_padd[p_addr_hex] = '\0';

    for (i=0; i<offset_hex; i++){
        temp_offset[i] = address[i+p_addr_hex];
    }
    temp_offset[offset_hex] = '\0';

    //printf("address = %s\noffset = %s\n", temp_padd, temp_offset);

    *page_num = strtol(temp_padd, NULL, 16);
    *offset = strtol(temp_offset, NULL, 16);

    //printf("Page number: %d\nOffset: %d\n", *page_num, *offset);
}


void update_index(struct Index *index, int page_num, char op, int reading, int position){

    index[position].page_number = page_num;
    index[position].last_used = reading;
    if ( op == 'R'){
        index[position].changed = false;
    } else {
        index[position].changed = true;   
    }


}