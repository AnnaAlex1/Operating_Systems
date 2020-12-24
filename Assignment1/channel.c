//channel.c

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>
#include <stdbool.h>


#include "share.h"
#include "semaphores.h"

#define SHM_SIZE 300
#define producer 0
#define consumer 1
#define prod_redo 2
#define cons_redo 3

key_t key2 = 9876;          //shared memory segment between ENC1-CHAN    
key_t key3 = 9873;          //shared memory segment between CHAN-ENC2 
 



int main( int argc, char *argv[] ){ //argument: probability

    //checking for right number of arguments
    if (argc != 2){ perror("Wrong input! Give a number for probability.\n"); return 1; } 
    
    srand( time(0) );        //initialize with seed
   
   
    /////GET SHARED SEGMENTS

    int shmid2,shmid3;
    char *seg2, *seg3;
    
    int error = 0;

    //get the shared memory between ENC1&CHAN
    seg2 = get_shm(key2, SHM_SIZE, &shmid2, &error);  
    if (error == 1){return 1;} 
    //create the shared memory between CHAN&ENC2
    seg3 = create_shm(key3, SHM_SIZE, &shmid3, &error);
    if (error == 1){return 1;} 
    

    

    //////GET SEMAPHORES

    int semid2;
    memcpy(&semid2, seg2, sizeof(int));     //getting semaphore for communication between ENC1-CHAN
    

    int semid3 = sem_create(4);             //creating semaphore for communication between CHAN-ENC2
    sem_init(semid3, producer, 0);
    sem_init(semid3, consumer, 1);
    sem_init(semid3, prod_redo, 0);
    sem_init(semid3, cons_redo, 1);
    
    memcpy(seg3, &semid3, sizeof(int));


    ///////////CREATE PROCESS ENC2 
    pid_t pid = fork();

    if (pid == 0){  // case: child process
            
            char *args[] = {"./enc", argv[1], "2", NULL};   // "2": process number

            execv(args[0], args);           //execute process ENC2


    } else if (pid < 0){ // case: unsuccessful fork()

            perror("Unsuccessful fork!\n");
            return 1;
    }


    //////////////////////  





    int mes_length; 
    int probability = atoi(argv[1]);        //get probability
    bool redo = false;

    while (1){

        
        printf("CHAN: Waiting for a message from ENC1 to pass to ENC2\n");

        //GETTING IN IF ENC1 SENT A MESSAGE
        P(semid2, producer);
        int total_len;
        memcpy(&total_len, seg2 + sizeof(int), sizeof(int));
        mes_length = total_len - MD5_DIGEST_LENGTH; /* - 128 bits*/
        
        printf("CHAN: Getting message from ENC1\n");
        char message[mes_length];
        memcpy(message, seg2 + 2*sizeof(int), mes_length);
        message[mes_length-1] = '\0';

        V(semid2, consumer);

        int ran_num;
        
        

        if ( strcmp(message, "TERM") != 0 ){        //don't change it if it's the message TERM

            //printf("CHAN alters message\n");

            for (int i=0; i<mes_length-1; i++){

                ran_num = (rand() % 126) + 1; //from 1 to 126
                
                //printf("ran_num: %d\n", ran_num);

                if ( ran_num < probability ){
                    //printf("    %c  ", '0' + ran_num);
                    message[i] = '0' + ran_num;
                    if ( message[i] == '\\'){           //avoid character '\'
                        message[i]--;
                    }
                }
            }



        } 


        P(semid3, consumer);    //to send message to ENC2

        printf("CHAN: Sending \"%s\" to ENC2.\n", message);
        
        memcpy(seg3 + sizeof(int), &total_len, sizeof(int) );         //give to ENC2 the message length
        memcpy(seg3 + 2*sizeof(int), message,  mes_length-1);      //give to ENC2 the alternative message
        memcpy(seg3 + 2*sizeof(int) + mes_length-1, seg2+ 2*sizeof(int) + mes_length-1,  total_len-mes_length+1);     //give to ENC2 the checksum (take it from seg2)
 
        V(semid3, producer);     //access no longer denied for enc2


        //CHECKING RESENDING INFO FROM ENC2
        printf("CHAN: Waiting for resending info\n");
        P(semid3, prod_redo);  
        printf("CHAN: Receiving resending information from ENC2.\n");
        P(semid2, cons_redo);
        printf("CHAN: Sending resending information to ENC1.\n");
        
        int msg_length;
        memcpy(&msg_length, seg3 + sizeof(int), sizeof(int));

        memcpy(&redo ,seg3 + 2*sizeof(int) + msg_length, sizeof(bool));
        
        memcpy(seg2 + 2*sizeof(int) + msg_length, &redo, sizeof(bool));           //give redoing info to ENC1
        V(semid2, prod_redo);
        V(semid3, cons_redo);



        if ( strcmp(message, "TERM") == 0 ){    
            printf("CHAN: Message TERM went through CHANNEL! Terminating...\n");
            break;
        }

    }



    printf("CHAN detatching SHM with ENC1\n");

    if (shmdt(seg2) == -1) {
        perror("Error in detatching the Shared Memory Segment");
        return 1;
    }


    printf("CHAN detatching and deleting SHM with ENC2\n"); 
        
    if (shmdt(seg3) == -1) {
        perror("Error in detatching the Shared Memory Segment");
        return 1;
    }
 

    if (shmctl(shmid3, IPC_RMID, NULL) == -1) {
        perror("Error in destroying the Shared Memory Segment");
        return 1;
    }

    return 0;

}
