#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "second_chance.h"
#include "lru.h"

#define ADDRESS_SIZE 30                     //to_change
#define PAGE_SIZE 4096
#define FRAME_SIZE 4096
#define LINE_SIZE 50




void print_statistics(int w_counter, int r_counter, int page_faults, int num_of_frames);
int parser(FILE* file, char** address, char *operation);




int main(int argc, char* argv[]){

    ///////GETTING ARGUMENTS
    if (argc != 4 && argc!= 5){
        printf("Wrong Number of Arguments!\n");
        return 1;

    }

    char algorithm[10];
    int num_of_frames, q;
    int max_ref=100;    //default value in case it's not given                //to change

    //get info
    strcpy(algorithm, argv[1]);             //1st argument: algorithm to be used (LRU/Second Chance)
    if ( ( strcmp(algorithm, "LRU") != 0) && ( strcmp(algorithm, "SECC") != 0) ){
        printf("Wrong input for algorithm value. Must be 'LRU'/'SECC'\n");
        return 1;
    }
    num_of_frames = atoi(argv[2]);          //2nd argument: number of frames
    q = atoi(argv[3]);                      //3rd argument: number of references


    if (argc == 5) max_ref= atoi(argv[4]);    //case: 4 arguments given: max number of references was given   

 /*   printf("Algorithm to be used: %s\n", algorithm);
    printf("Number of frames in memory: %d\n", num_of_frames);
    printf("Number of references q: %d\n", q);
    printf("Maximum number of references: %d\n", max_ref);
*/


    ///////FOR STATISTICS

    int w_counter = 0;      //counts the number of writings to the disk
    int r_counter = 0;      //counts the number of readings from the disk     
    int page_faults = 0;    //counts how many page-faults occured                       //maybe not just the number?
    //?                        //καταχωρήσεις που εξετάστηκαν από το αρχείο ίχνους αναφορών



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

    //int cur_in_frames = 0;
    struct Table* hashed_ptable1;         //keeps pages in memory
    hashed_ptable1 = malloc(sizeof(struct Table)*TABLE_SIZE);    //allocation of the hashed page table for process1

    struct Table* hashed_ptable2;
    hashed_ptable2 = malloc(sizeof(struct Table)*TABLE_SIZE);    //allocation of the hashed page table for process2

    char *address;
    char op;

    int process_num = 0;

    bool finished1 = false, finished2 = false;

    
    int max_counter = 1;

    while ( max_counter < max_ref*2 ){    //max times for process 1 and max times for process 2
        
        int q_counter = 0;

        process_num = 1;    //start with process1

        while (q_counter < 2*q){    // q references each time

            if (q_counter == q){    //after q readings, move to process 2
                process_num = 2;
            }

            //Read from file
            if ( ( process_num == 1 ) && !finished1){
                //read from file 1
                if ( parser(file1, &address, &op) == -1 ){  //if file is finished
                    finished1 = true;
                    continue;
                }
                printf("File1       address: %s, op: %c\n", address,op);
            
            } else if ( ( process_num == 2 ) && !finished2 ){
                //read from file 2
                if ( parser(file2, &address, &op) == -1 ){  //if file is finished
                    finished2 = true;
                    continue;
                }
                printf("File2       address: %s, op: %c\n", address,op);
            }
            

            //check if page is already in memory
            //if there is space
            
                //put page in memory
                // if ( process_num == 1 ){
                    //insert_in_hashtable(hashed_ptable1, vir_page_num, frame_num);
                //} else {
                    //insert_in_hashtable(hashed_ptable2, vir_page_num, frame_num);
                //}
                //cur_in_frames++;
            
            //else

            /*    if ( strcmp(algorithm, "LRU") == 0){
                    //LRU_algorithm();
                } else {
                    //SECC_algorithm();
                }
            */


            //--ενημέρωση πίνακα σελίδων??
            //αν έχουμε εγγραφή στο δίσκο:          w_counter++;
            //αν έχουμε ανάγνωση από το δίσκο:      r_counter++;

            max_counter++;
            q_counter++;
        }

    
    }
    
    
    
    
    
    
    ///////STATISTICS
    print_statistics(w_counter, r_counter, page_faults, num_of_frames);


    //closing files
    fclose(file1);
    fclose(file2);

}



void print_statistics(int w_counter, int r_counter, int page_faults, int num_of_frames){

    printf("Number of writings to the disk performed: %d\n", w_counter);
    printf("Number of readings from the disk performed: %d\n", r_counter);
    printf("Number of page-faults that occured: %d\n", page_faults);
    printf("Number of frames in memory: %d\n", num_of_frames);
    
}

int parser(FILE* file, char** address, char *operation){

    char line[LINE_SIZE];
    if ( fgets(line, LINE_SIZE, file) != NULL){
        sscanf(line,"%s %c", *address, &(*operation));
        return 0;
    }

    return -1;

}