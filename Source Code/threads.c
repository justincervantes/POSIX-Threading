/*---------------------------------------------------------------------------------------
--      Source File:            threads.c - A simple illustration of threads and mutexes
--
--      Functions:              main
--                              ThreadFunc
--    
--      Date:                   January 23, 2020
--
--      Revisions:              (Date and Description)
--                                      
--      Designer:               Justin Cervantes
--                              
--      Programmer:             Justin Cervantes
--
--      Notes:
--      This program illustrates a simple pthread example, and uses flagging on
--      thread id's emphasizing pthread_self api and the pthread_t value which
--      is updated by the pthread_create() api.
--
--      Objective: To make the producer and consumer threads execute the same function,
--      but have the producer do something before the consumer (ie set defaults in
--      the struct, then having the consumer print it out).
--      
--      To compile the application:                
--      gcc -Wall -o threads threads.c -lpthread

---------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>


// Function Prototypes
void* ThreadFunc (void *);
void usage (char **);

// Mutex Variables
pthread_mutex_t printfLock = PTHREAD_MUTEX_INITIALIZER;

// Globals
typedef struct 
{
    char *structString;
    int structInt;
    double structDouble;
    bool publishCondition;
    long int producerThreadID;
    long int consumerThreadID;

}ThreadInfo;

int main (int argc, char **argv)
{
    // Create a pointer to a struct.
    ThreadInfo *Info_Ptr;
    
    // Allocate struct memory
    if ((Info_Ptr = malloc (sizeof (ThreadInfo))) == NULL)
    {
        perror ("malloc");
        exit (1);
    }
    

    // Set the defaults for the struct
    char inputString[100];
    int inputInt;
    double inputDouble;

    printf( "Enter a string: ");
    fgets( inputString, 100, stdin );
    Info_Ptr->structString = inputString;

    printf( "Enter an int: ");
    scanf("%d", &inputInt);  
    Info_Ptr->structInt = inputInt;


    printf( "Enter a double: ");
    scanf("%lf", &inputDouble);  
    Info_Ptr->structDouble = inputDouble;

    // Set a default value
    Info_Ptr->publishCondition = false;    
    printf ("\nOriginal input\nString: %sInt: %d\nDouble: %lf\n\n", Info_Ptr->structString, Info_Ptr->structInt, Info_Ptr->structDouble);

    pthread_t producerThread, consumerThread;

    // This for loop is used for creating threads, the loop is required to get unique pthread_t identifiers
    // Reference: https://stackoverflow.com/questions/23980591/why-does-pthread-self-return-the-same-id-multiple-times
    for(int i = 0; i < 2; i++ ) {
        if(i ==0 ) {
            pthread_create (&producerThread, NULL, ThreadFunc, (void*)Info_Ptr);
        }
        if(i == 1) {
            pthread_create (&consumerThread, NULL, ThreadFunc, (void*)Info_Ptr);
        }
    }
    Info_Ptr->producerThreadID = producerThread;
    Info_Ptr->consumerThreadID = consumerThread;
    
    pthread_join (producerThread, NULL);
    pthread_join (consumerThread, NULL);
    
    free (Info_Ptr);
    exit (0);
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ThreadFunc
--
-- DATE: January 23, 2020
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Justin Cervantes
--
-- PROGRAMMER: Justin Cervantes
--
-- INTERFACE: void* ThreadFunc (void *info_ptr)
-- void *info_ptr: Used to pass a pointer to a struct from main
--
-- RETURNS: void
--
-- NOTES:
-- This function is being run by both consumerThread and producerThread at the same time. The program will
-- only let the consumerThread finish once the producerThread has. Mutex code has been left in and commented out
-- to demonstrate that the mutex is not required to 'lock' a thread.
----------------------------------------------------------------------------------------------------------------------*/
void* ThreadFunc (void *info_ptr)
{
    ThreadInfo *UserInfo = (ThreadInfo *)info_ptr;


    
    while(UserInfo->producerThreadID == 0) {
        printf("Waiting for producer thread to update...\n");
    }

    while(UserInfo->consumerThreadID == 0) {
        printf("Waiting for consumer thread to update...\n");
    }


    if (pthread_self() == UserInfo->producerThreadID) {
        printf("Producer thread is now updating all values to: 'Producer's update', 1, 1.0\n");
        //pthread_mutex_lock(&printfLock);   
        UserInfo->structString = "Producer's Update";
        UserInfo->structInt = 1;
        UserInfo->structDouble = 1.0;  
        printf("Producer thread has finished updating all values\n");
        fflush(stdout);
        UserInfo->publishCondition = true;
        //pthread_mutex_unlock(&printfLock);
    } else {

        //If the consumer is running, wait until the publish flag is turned on, then print it out!
        while(!UserInfo->publishCondition) {
            printf("Waiting for the producer thread to flag that it has updated the struct for printing...\n");
        }
        printf ("Consumer is printing... String: %s, Int: %d, Double: %lf\n", UserInfo->structString, UserInfo->structInt, UserInfo->structDouble);
    }
    
    
    //Useful for testing the thread that is currently running
    printf("Curent thread in function id = %ld\n", pthread_self());
    



    fflush(stdout);    
    return NULL;

}

