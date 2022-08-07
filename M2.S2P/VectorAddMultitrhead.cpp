#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <pthread.h>

#define NUM_THREADS 2

using namespace std::chrono;
using namespace std;

//create a data structure as the input of add thread method
struct AddTask
{
    int *v1, *v2, *v3;
    int start;
    int end;
};

//create a data structure for random data generation
struct RandomTask
{
    int seed_factor;
    int *v;
    int start;
    int end;
};

//Random Thread method
void *randomVector(void *args)
{
    //cast the void* args to RandomTask*
    RandomTask *task = ((struct RandomTask *)args);

    //Seed the srand using time and random seed factor
    srand(time(NULL) * task->seed_factor);

    //for the identifed partition, randomly popoluate the vector 
    for (int i = task->start; i < task->end; i++)
    {
        task->v[i] = rand() % 100;
    }

    return NULL;
}

//Add Vector Thread method
void *addVector(void *args)
{

    //cast the void* args to AddTask*
    AddTask *task = ((struct AddTask *)args);

    //for the identifed partition, add v1 and v2 and store the result in v3
    for (int i = task->start; i < task->end; i++)
    {
        task->v3[i] = task->v1[i] + task->v2[i];
    }

    return NULL;
}

//a helper function the write arrays
void printArray(int *vector, int size)
{

    cout << vector[0];
    for (int i = 1; i < size; i++)
    {
        cout << ", " << vector[i];

        if( i % 20 == 0){
            cout <<"\n";
        }

    }
    cout << "\n";
    cout << "\n***************************************************************************************\n";
}

//The main function
int main()
{

    //init varaibles
    unsigned long size = 1000000000L;

    int *v1, *v2, *v3;

    //create an array of threads for add task
    pthread_t addThreads[NUM_THREADS];

    //create an array of threads for random data generation
    pthread_t randomThreads[NUM_THREADS];

    //get memory in the heap for 3 vectors
    v1 = (int *)malloc(size * sizeof(int *));
    v2 = (int *)malloc(size * sizeof(int *));
    v3 = (int *)malloc(size * sizeof(int *));

    //store the start time
    auto start = high_resolution_clock::now();

    //We use this seed factor to make sure the srand get a unique seed inside randomVector method
    int seed_factor = 16;

    //calculates the partition size for random data generation
    int partition_size = size / (NUM_THREADS / 2);

    //use half of the available threads for randomly populating first vector
    for (size_t i = 0; i < NUM_THREADS / 2; i++)
    {
        //create a pointer to Random Task
        struct RandomTask *random_task = (struct RandomTask *)malloc(sizeof(struct RandomTask));

        //set the random_task values

        //unique number for seed generation
        random_task->seed_factor = seed_factor++;

        //set vector that needs to be populated
        random_task->v = v1;

        //set the start index of for loop
        random_task->start = i * partition_size;

        //cacluates the end index of for loop, if it's the last thread, the end index should be same as vector size
        random_task->end = (i + 1) == (NUM_THREADS / 2) ? size : ((i + 1) * partition_size);

        //start random data generation thread
        pthread_create(&randomThreads[i], NULL, randomVector, (void *)random_task);
    }

    //do the same thing for vector 2  
    //p.s. (it's not a good practice to repeat your self. This code can be implemented in a way to eliminate this repetition. I leave it to you to think about it/improve it)
    for (size_t i = 0; i < NUM_THREADS / 2; i++)
    {
        struct RandomTask *random_task = (struct RandomTask *)malloc(sizeof(struct RandomTask));
        random_task->seed_factor = seed_factor++;
        random_task->v = v2;
        random_task->start = i * partition_size;
        random_task->end = (i + 1) == (NUM_THREADS / 2) ? size : ((i + 1) * partition_size);
        pthread_create(&randomThreads[i + NUM_THREADS / 2], NULL, randomVector, (void *)random_task);
    }

    //wait until all the random data generation threads joined the main thread
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(randomThreads[i], NULL);
    }


    //calcuates the partion size for the vector addtion
    partition_size = size / NUM_THREADS;

    //craete threads for vector additions
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        //create a pointer to AddTask construct and allocate memory for in the heap
        struct AddTask *task = (struct AddTask *)malloc(sizeof(struct AddTask));

        //initialise the Add Task
        task->v1 = v1;
        task->v2 = v2;
        task->v3 = v3;

        //calculates the start and end of each thread (based on partition size)
        task->start = i * partition_size;  
        task->end = (i + 1) == NUM_THREADS ? size : ((i + 1) * partition_size); 

        //start task addtion thread
        pthread_create(&addThreads[i], NULL, addVector, (void *)task);
    }

    //wait until all vector addition threads join the master thread
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(addThreads[i], NULL);
    }

    //store the time (stop time) in a variable
    auto stop = high_resolution_clock::now();

    //calculates the time difference (duration of execution)
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;


    // write the first 100 elements of the vectors
    cout <<"v1: \n\n  ";
    printArray(v1, size < 100 ? size : 100);
    cout <<"v2: \n\n  ";
    printArray(v2, size < 100 ? size : 100);
    cout <<"v3: \n\n  ";
    printArray(v3, size < 100 ? size : 100);

    return 0;
}