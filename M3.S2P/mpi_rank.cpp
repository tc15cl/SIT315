#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    int numtasks, rank, name_len, source, count, tag=1, char_size=12, buf; 
    char name[MPI_MAX_PROCESSOR_NAME], inmsg[char_size], outmsg[]="Hello World!", bcastmsg[char_size];
    MPI_Status Stat;

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


//BROADCAST SECTION - QUESTION 2, comment/uncomment to run

    //if master (0) then send to all other workers
    if(rank == 0)
    {
        strcpy(bcastmsg,"Hello World!");
    }

    MPI_Bcast(&bcastmsg, char_size, MPI_CHAR, 0, MPI_COMM_WORLD);     
    printf("Task %d: Broadcast: %s\n", rank, bcastmsg);




//SEND/RECEIVE SECTION - QUESTION 1, uncomment to run
/**
    //if master (0) then send to all other workers
    if(rank == 0)
    {
        //start at 1 (first worker), iterate through all tasks and send message
        for (int dest=1; dest<numtasks; dest++)
        {
            MPI_Send(outmsg, strlen(outmsg), MPI_CHAR, dest, tag, MPI_COMM_WORLD);
        }
    }
    else if (rank > 0) //if worker receive message from master
    {
        //receive message and print Hello World + info about sender/receiver
        source = 0;
        MPI_Recv(&inmsg, char_size, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
        printf("Task %d: Received this message from Task %d: %s \n", rank, source, inmsg);
    }
**/


    // Finalize the MPI environment
    MPI_Finalize();
}

