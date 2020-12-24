//share.c

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include "share.h"
#include <stdbool.h>
#include <sys/shm.h>



char* create_shm(key_t key, int size, int *shmid, int* error){

   
    *shmid = shmget(key, size, IPC_CREAT |  0666);

    if (*shmid < 0 ){
        perror("Error in creating Shared Memory Segment!\n");
        *error = 1;
    }

    char *seg = shmat(*shmid, NULL, 0);
    if (seg == (char *) -1){
        perror("Error in attatching the Shared Memory Segment!\n");
        *error = 1;
    }

    *error = 0;
    return seg;
}




char* get_shm(key_t key, int size, int *shmid, int * error){

   
    *shmid = shmget(key, size, 0666);

     if (*shmid < 0 ){
        perror("Error in creating Shared Memory Segment!\n");
        *error = 1;
    }

    char *seg = shmat(*shmid, NULL, 0);
    if (seg == (char *) -1){
        perror("Error in attatching the Shared Memory Segment!\n");
        *error = 1;
    }

    *error = 0;
    return seg;

}

