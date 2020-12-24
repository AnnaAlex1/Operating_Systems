#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/sem.h>


#ifndef __SEMAPHORES__
#define __SEMAPHORES__


int sem_create(int num_semaphores);
void sem_init(int semid, int index, int value);
char* get_shm(key_t key, int size, int *shmid, int * error);
void P(int semid, int index);
void V(int semid, int index);

#endif