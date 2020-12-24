//share.h
#include <stdbool.h>
#include <sys/shm.h>

#ifndef _SHARE_H
#define _SHARE_H


char* create_shm(key_t key, int size, int *shmid, int* error);
char* get_shm(key_t key, int size, int *shmid, int * error);


#endif