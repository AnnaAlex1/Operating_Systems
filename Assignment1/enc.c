//enc.c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <stdbool.h>


#include "share.h"
#include "semaphores.h"

#define SHM_SIZE 300
#define producer 0
#define consumer 1
#define prod_redo 2
#define cons_redo 3

key_t key = 9875;          //shared memory segment between P1-ENC1
key_t key2 = 9876;          //shared memory segment between ENC1-CHAN    
key_t key3 = 9873;          //shared memory segment between CHAN-ENC2 
key_t key4 = 9874;          //shared memory segment between ENC2-P2    
  


int main(int argc, char* argv[]){
    
    //checking for right number of arguments

    if (argc != 3){ perror("Wrong input! Give a number for probability.\n"); return 1; } 
    
    int process = atoi(argv[2]);


    /////GET SHARED SEGMENTS

    int shmid,shmid2,shmid3,shmid4;
    char *seg, *seg2, *seg3, *seg4;
    
    int error = 0;

    int semid, semid2, semid3, semid4;

    bool done = false;

    if (process == 1){


        //get the shared memory between P1&ENC1
        seg = get_shm(key, SHM_SIZE, &shmid, &error);
        if (error == 1){return 1;}
        //create shared memory segment between ENC1&CHAN
        seg2 = create_shm(key2, SHM_SIZE, &shmid2, &error);  
        if (error == 1){return 1;} 


        /////GET SEMAPHORES                
        memcpy(&semid, seg, sizeof(int));       //semaphore for getting message from P1 to ENC1
         
        semid2 = sem_create(4);                  //creating semaphore for sharing message between ENC1 & CHAN
        sem_init(semid2, producer, 0);
        sem_init(semid2, consumer, 1);
        sem_init(semid2, prod_redo, 0);
        sem_init(semid2, cons_redo, 1);

        memcpy(seg2, &semid2, sizeof(int));

    } else if (process == 2){
        
        
        //get the shared memory between CHAN&ENC2
        seg3 = create_shm(key3, SHM_SIZE, &shmid3, &error);
        if (error == 1){return 1;} 
        
        //create shared memory segment between ENC2&P2
        seg4 = create_shm(key4, SHM_SIZE, &shmid4, &error);
        if (error == 1){return 1;} 
        

        memcpy(&semid3, seg3, sizeof(int));     //getting semaphore for sharing message between CHAN & ENC2 

        memcpy( &semid4, seg4, sizeof(int));       //getting semaphore for sharing message between ENC2 & P2

    }
    



    
    //////

    pid_t pid;

    int mes_length;
    int new_length;
    char *mixed;
    bool redo = false;
    bool first_time = true;     //flag for entering "if" statement only once  
    int i = 1;      //message counter

    while(1){
        
        
        if (process == 1){  //ENC1          //case: a message was sent from process P1, need to sent it to CHAN
            
            if (i == 2){   //first time entering the loop for process that used to be ENC2
                    
                    //get shared memory segment between P1&ENC1
                    seg = get_shm(key, SHM_SIZE, &shmid, &error);   
                    if (error == 1){return 1;}

                    //get shared memory segment between ENC1&CHAN
                    seg2 = get_shm(key2, SHM_SIZE, &shmid2, &error);   
                    if (error == 1){return 1;}


                    //get semaphore for communication between P1-ENC1 & ENC1-CHAN
                    memcpy(&semid, seg, sizeof(int));
                    memcpy(&semid2, seg2, sizeof(int));

            }
           
            printf("\nENC1: Waiting for message from P1\n");
            P(semid, producer);             //access denied until a message from P1 was sent                            
            
            printf("ENC1: Got the message from P1\n");
            memcpy( &mes_length, seg + sizeof(int), sizeof(int) );  //get length of message
            char message[mes_length];
            memcpy( message, seg + 2*sizeof(int), mes_length );     //get message

            V(semid, consumer);        //finished getting the data from P1

            //printf("Message in ENC1: %s\n", message);

            char checksum[MD5_DIGEST_LENGTH];
            MD5((const unsigned char *)message, sizeof(message),(unsigned char*)checksum );    //calculating checksum

            char checksum0[MD5_DIGEST_LENGTH+1];                    //adding '\0' to end of checksum
            memcpy(checksum0,checksum,MD5_DIGEST_LENGTH );
            checksum0[MD5_DIGEST_LENGTH-1] = '\0'; 


            //printf("Checksum: %s: DONE\n", checksum0);

            new_length = sizeof(char)*( mes_length + MD5_DIGEST_LENGTH );       //adding checksum to the end of the message
            mixed = malloc( new_length );
            
            strcpy(mixed, message);
            strcat(mixed, checksum0);

            //printf("Mixed: %s\n", mixed);


            P(semid2, consumer);              //does not let CHAN to get data from shm2

            printf("ENC1: sending message to CHAN\n");
            
            memcpy(seg2 + sizeof(int), &new_length, sizeof(int) );
            memcpy(seg2 + 2*sizeof(int), mixed, new_length );
            
            V(semid2, producer);                //when this finishes chan can access the shm


            //create process chan
            if (first_time){    //do it once

                char *args[] = {"./chan", argv[1], NULL};

                pid = fork();
                if (pid == 0){

                    execv(args[0], args);                         //execute process CHAN

                } else if (pid < 0){ // case: unsuccessful fork()

                    perror("Unsuccessful fork!\n");
                    return 1;
                }

                first_time = false;

            }

            if ( strcmp(message, "TERM") == 0 ){    
                    free(mixed);             
                    printf("ENC1: Message for termination received!\n");
                    process = 1; //for correct detatching 
                    break;
                }
            
        }   

        
        
        if (process == 2){  //FOR ENC2      //case: CHAN sent a message to proceed to P

            first_time = false;

            if ( i == 2){   //first time entering the loop for process that used to be ENC2
                    
                    //get shared memory segment between CHAN&ENC2
                    seg3 = get_shm(key3, SHM_SIZE, &shmid3, &error);   
                    if (error == 1){return 1;}

                    //get shared memory segment between ENC2&P2
                    seg4 = get_shm(key4, SHM_SIZE, &shmid4, &error);   
                    if (error == 1){return 1;}


                    //get semaphore for communication between CHAN-ENC2 & ENC2-P2
                    memcpy(&semid3, seg3, sizeof(int));
                    memcpy(&semid4, seg4, sizeof(int));
                    
            }


            printf("ENC2: Waiting for CHAN to send a message\n");
            //now stored in message: the changed version (done in CHAN) + checksum

            P(semid3, producer);         //getting in if chan is done sending the message
            
            int total_len;
            memcpy(&total_len, seg3 + sizeof(int), sizeof(int));
            mes_length = total_len - MD5_DIGEST_LENGTH;             //get message size (without checksum)

            
            char alt_msg[mes_length];                               //get message (without checksum)
            memcpy(alt_msg, seg3 + 2*sizeof(int), mes_length);
            alt_msg[mes_length-1] = '\0';
            
            printf("ENC2: Got the message \"%s\"\n", alt_msg);


            char ori_chksum[MD5_DIGEST_LENGTH];
            memcpy(ori_chksum, seg3 + 2*sizeof(int) + mes_length - 1, MD5_DIGEST_LENGTH );       //gets checksum calculated in ENC1
            
            char ori_chksum0[MD5_DIGEST_LENGTH+1];
            memcpy(ori_chksum0, seg3 + 2*sizeof(int) + mes_length - 1, MD5_DIGEST_LENGTH +1 );       //adds '\0'
            

            V(semid3, consumer);  //done reading what CHAN sent

            char checksum2[MD5_DIGEST_LENGTH];  
            MD5((const unsigned char *) alt_msg, sizeof(alt_msg), (unsigned char*)checksum2 );     //calculates checksum of altered message
            
            char checksum20[MD5_DIGEST_LENGTH+1];
            memcpy(checksum20,checksum2,MD5_DIGEST_LENGTH );                                         //adds '\0'
            checksum20[MD5_DIGEST_LENGTH-1] = '\0'; 


            //printf("Checksum = %s while Original Checksum = %s\n", checksum20, ori_chksum0);
            

            if  ( strcmp( checksum20, ori_chksum0  ) == 0  ){ //case: message is right, proceed to P2

                P(semid3, cons_redo);       //ENC2 tells CHAN that redo is not needed
                printf("ENC2: Correct message! Tells CHAN there is NO need for resend!\n");
                
                P(semid4, consumer);            //sending message to P2
                
                redo = false;
                
                memcpy(seg4 + sizeof(int), &mes_length, sizeof(int));        //send message(without checksum)
                memcpy(seg4 + 2*sizeof(int),alt_msg ,mes_length);

                printf("ENC2: Sending to P2 to print.\n");
                
                memcpy(seg3 + 2*sizeof(int) + total_len, &redo , sizeof(bool));             //redo

                V(semid3, prod_redo);   //access for CHAN for redoing part no longer denied
                V(semid4, producer);    //access to P2 no longer denied
                
                                                                
                
                i++;                        //next round
                process = 1;                //change job
            } else {                                      //case: message is wrong, go back to CHAN

                P(semid3, cons_redo);                    //ENC2 sends redo message to CHAN
                redo = true;
                memcpy(seg3 + 2*sizeof(int)+ total_len, &redo, sizeof(bool));
                printf("ENC2: Wrong message! Sending signal to CHAN for resending.\n");
                V(semid3, prod_redo);                   //CHAN to check redo information
                
            }

            if ( strcmp(alt_msg, "TERM") == 0 ){  
                    process = 2; //for correct detatching               
                    printf("ENC2: Message for termination received!\n");
                    break;
            }

        } else if (process == 1){     //FOR ENC1                   //CHAN sent a "redo" instruction
            
            bool redo;
            
            while(1){


                P(semid2, prod_redo);     //getting in when chan sent redo info
                
                int total_length;
                memcpy(&total_length, seg2 + sizeof(int), sizeof(int));
                memcpy(&redo, seg2 + 2*sizeof(int) + total_length, sizeof(bool));       //getting redo flag

                if ( redo ){    //if there is a need for redoing
                    
                    P(semid2, consumer);
                    printf("ENC1: Resending the message to CHAN\n\n");
                    
                    memcpy(seg2 + sizeof(int), &new_length, sizeof(int) );
                    memcpy(seg2 + 2*sizeof(int), mixed, new_length );

                    V(semid2, producer);

                } else{
                    free(mixed);
                    printf("ENC1: No need to resend.\n");
                    done = true;

                }
                
                V(semid2, cons_redo); 

                if (done){      //done with resending
                    done = false;
                    break;
                }
            }

            process = 2;        //change job
            i++;                //next round

        }
        
        
    }


    
    //DETATCHING SHARED MEMORY SEGMENT  
    if (process == 1){
        sleep(1);


        printf("ENC1 detatching SHM with P1\n");

        if (shmdt(seg) == -1) {
            perror("Error in detatching the Shared Memory Segment");
            return 1;
        }


        if (shmdt(seg2) == -1) {
            perror("Error in detatching the Shared Memory Segment");
            return 1;
        }

        printf("ENC1 detatching and deleting SHM with CHAN\n");

        if (shmctl(shmid2, IPC_RMID, NULL) == -1) {
            perror("Error in destroying the Shared Memory Segment");
            return 1;
        }
        
    
    } else if (process == 2){
        

        printf("ENC2 detatching SHM with CHAN\n");

        if (shmdt(seg3) == -1) {
            perror("Error in detatching the Shared Memory Segment");
            return 1;
        }

        printf("ENC2 detatching and deleting SHM with P2\n");

        if (shmdt(seg4) == -1) {
            perror("Error in detatching the Shared Memory Segment");
            return 1;
        }

        sleep(1);

        if (shmctl(shmid4, IPC_RMID, NULL) == -1) {
            perror("Error in destroying the Shared Memory Segment");
            return 1;
        }


    }

    return 0;

}
