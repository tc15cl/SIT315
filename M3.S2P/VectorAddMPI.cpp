#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <mpi.h>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size)
{
    for (int i = 0; i < size; i++)
    {
        vector[i] = rand() % 100;
    }
}

int main(int argc, char** argv){
    int numtasks, rank, res, name_len, tag=1, localSum=0;
    char name[MPI_MAX_PROCESSOR_NAME];
    int *v1, *v2, *v3;
    int *v1_sub, *v2_sub, *v3_sub, totalSum;
	
    // Initialize the MPI environment
    MPI_Init(&argc,&argv);
	
    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	
    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
    // Find the processor name
    MPI_Get_processor_name(name, &name_len);
	
    //long variables for vector and partition size
    unsigned long vectorSize = 10000000;
    unsigned long partition = vectorSize / numtasks;
   
    //random initialiser
    srand(time(0));
	
    //vector sub creation
    v1_sub = (int *) malloc(vectorSize * sizeof(int *));
    v2_sub = (int *) malloc(vectorSize * sizeof(int *));
    v3_sub = (int *) malloc(vectorSize * sizeof(int *));
	
    //vector creation only if head node
    if(rank == 0){
        v1 = (int *) malloc(vectorSize * sizeof(int *));
        v2 = (int *) malloc(vectorSize * sizeof(int *));
        v3 = (int *) malloc(vectorSize * sizeof(int *));
        randomVector(v1, vectorSize);
        randomVector(v2, vectorSize);
    }
	
    //Clock start
    auto start = high_resolution_clock::now();
	
    //Scatter both input vectors
    MPI_Scatter(v1, partition, MPI_INT, v1_sub, partition, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, partition, MPI_INT, v2_sub, partition, MPI_INT, 0, MPI_COMM_WORLD);
	
    //For loop to calculate vector adds and allocate to vectors 3 and local sum (for reducing)
    for (size_t i = 0; i < partition; i++)
    {
        v3_sub[i] = v1_sub[i] + v2_sub[i];
        localSum += v3_sub[i];
    }
	
    //gather vector 3 calculation
    MPI_Gather(v3_sub, partition, MPI_INT, v3, partition, MPI_INT, 0, MPI_COMM_WORLD);
	
    //reduce function for local sum variable
    MPI_Reduce(&localSum, &totalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0)
    {
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time taken by function: "
         << duration.count() << " microseconds"  
         <<" and the total sum is " << totalSum  
         <<endl;
    }
	
    MPI_Finalize();
	
    return 0;
}