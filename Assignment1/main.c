//main.c for processes P1-P2
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdbool.h>

#include "share.h"
#include "semaphores.h"

#define SHM_SIZE 300
#define MAX_STRING_SIZE 50

#define producer 0
#define consumer 1
#define prod_redo 2
#define cons_redo 3


key_t key = 9875;          //shared memory segment between P1-ENC1
key_t key4 = 9874;          //shared memory segment between ENC2-P2    



int main(int argc, char* argv[]){//arguments: process number, probability
    
    char str[MAX_STRING_SIZE];

//////////////CHECKING ARGUMENTS

    int process = atoi(argv[1]);
    
    if (process == 1){
        if (argc != 3){ perror("Wrong input! Give a number for process and one for probability.\n"); return 1; } 
    } else if (process == 2){
        if (argc != 2){ perror("Wrong input! Give only a number for process.\n"); return 1; }
    } else {
        printf("Process number must be 1 or 2\n");
    }
    

////////////////////////



    ////////////CREATE PROCESS ENC1
    if (process == 1){
        
        pid_t pid = fork();

        if (pid == 0){  // case: child process
                             
                char *args[] = {"./enc", argv[2], argv[1], NULL}; //sending probability, process number

                execv(args[0], args);           //execute process ENC1


        } else if (pid < 0){ // case: unsuccessful fork()

                perror("Unsuccessful fork!\n");
                return 1;
        }


    }
   

    ///////////////////////////////Case: PARENT PROCESS///////////////////////////////////
   
 
    int semid;              //semaphore between P1ENC1
    bool done = false;
    bool changed = false;   //variable to deal with change of direction
    int msg_size = 0;


    //memory segment for P1ENC1
    int shmid;
    char *seg;
    int error = 0;

    //memory segment for P2ENC2
    int semid4;
    int shmid4;
    char* seg4;
    int error4 = 0;

    if (process == 1){
        
        seg = create_shm(key, SHM_SIZE, &shmid, &error);   
        if (error == 1){return 1;}
        printf("Created Shared Memory Segment for P1-ENC1\n");

        //create semaphore for communication between P1ENC1
        semid = sem_create(2);
        sem_init(semid, producer, 0);          //producer
        sem_init(semid, consumer, 1);          //consumer


        memcpy(seg, &semid, sizeof(int));
        printf("Created Semaphores for P1-ENC1 shm\n");

    }


    if (process == 2){  
        
        //create shared memory segment between ENC2&P2
        seg4 = create_shm(key4, SHM_SIZE, &shmid4, &error4);   
        
        if (error4 == 1){return 1;}
        
        //semaphore for communication between ENC2&P2
        semid4 = sem_create(2);        
        sem_init(semid4, producer, 0);
        sem_init(semid4, consumer, 1);
        memcpy(seg4, &semid4, sizeof(int));
    }








    while (1){

        
        if (process == 1){      ///ONLY FOR PROCESS P1
            
                  
            if (changed){   //first time entering the loop for process that used to be P2 
        
                //get shared memory segment between P1&ENC1
                seg = get_shm(key, SHM_SIZE, &shmid, &error);   
                if (error == 1){return 1;}

                changed = false;

                //get semaphore for communication between P1&ENC1
                memcpy(&semid, seg, sizeof(int));
            }

            
            P(semid, consumer);                //gets in at the beggining or when P2 exits the printing part

            printf("\nGive a message:\n");
            scanf("%s", str);

            msg_size = sizeof(char)*(strlen(str) + 1);          
            memcpy( seg + sizeof(int), &msg_size, sizeof(int) );   //updating msg_size in shm        
            memcpy(seg + sizeof(int) + sizeof(int), str, msg_size );    //updating message in shm

            V(semid, producer);             //signal for enc to start process on message

            if ( strcmp(str, "TERM") == 0 ){                
                printf("P1: Message for termination received!\n");
                done = true;
                break;
            }
            
            
            /////////////////////////////
        }
        
        if (done) break;    //exit loop if done
        ////////////////////////////           
        

        if (process == 2){       //ONLY FOR PROCESS P2
            
             if (changed){   //first time entering the loop for prior process P1 
                
                //get shared memory segment between ENC2&P2
                seg4 = get_shm(key4, SHM_SIZE, &shmid4, &error4);   
                
                if (error4 == 1){return 1;}

                changed = false;
                
                //semaphore
                memcpy(&semid4, seg4, sizeof(int));
            }
            
            printf("P2: Waiting for a message!\n");

            //semaphore for communication with ENC2  
            P(semid4, producer);         //access must be denied until message arrived  
            

            printf("P2: Here is your message:\n");
            
            int mes_len;
            memcpy(&mes_len, seg4 + sizeof(int), sizeof(int));
            char *mes = malloc( mes_len );
            memcpy(mes, seg4 + 2*sizeof(int), mes_len);
            printf("%s\n\n", mes);
            

            if ( strcmp(mes, "TERM") == 0 ){                 
                printf("\nP2: Message for termination received!\n");
                done = true;
                free(mes);
                break;
            }

            free(mes);
            V(semid4, consumer);
        }

        changed = true;
        if (process == 2){ //changing process 2 to process 1
            process = 1;
        } else {      //changing process 1 to process 2
            process = 2;
        }

        if (done) break;

    }



    
    //////DESTROY SHARED MEMORY SEGMENT 
    if (process == 2){
        wait(NULL);  //wait untill all other processes are finished
        if (shmdt(seg4) == -1) {
        perror("Error in detatching the Shared Memory Segment");
        return 1;
        }
        printf("\nP2 detatched from SHM with ENC2\n");
    }
    


    if (process == 1){
        wait(NULL);  //wait untill all other processes are finished


        if (shmdt(seg) == -1) {
            perror("Error in detatching the Shared Memory Segment");
            return 1;
            }

        printf("P1 detatching and deleting SHM with ENC1\n");

        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("Error in destroying the Shared Memory Segment");
            return 1;
            }


    }

    

    printf("Goodbye!\n");

    return 0;
    
}



