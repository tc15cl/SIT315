#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>
#include <fstream>
#include <string.h>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size)
{
    #pragma omp parallel for default(none) shared(size, vector)
    for (int i = 0; i < size; i++)
    {
        //ToDo: Add Comment
        vector[i] = rand() % 100;
    }
}


int main()
{
    
    unsigned long size = 1000000000L;
    srand(time(0));
    int *v1, *v2, *v3;
    int total = 0;
    int criticalTotal = 0;
    //ToDo: Add Comment
    auto start = high_resolution_clock::now();

    //ToDo: Add Comment
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));

        randomVector(v1, size);
        randomVector(v2, size);

        //OMP pragma for main parallel block - with private and shared variable flags
        #pragma omp parallel default(none) shared(v1, v2, v3, total, criticalTotal) private(size)
        {
            //OMP pragma for loop - with different dynamic and chunk size flags
            #pragma omp for schedule(dynamic, 100)
            for (int i = 0; i < size; i++)
            {
                v3[i] = v1[i] + v2[i];      
                total += v3[i];
            }


            //OMP pragma for reduction, commented out to allow other testing
            /*
            #pragma omp parallel for reduction(+ : total)
            for (int i=0; i<size; i++)
            {
                #pragma omp atomic update
                total += v3[i];
            }
            */

            //OMP critical flag - for updating total
            #pragma omp critical
            {
                criticalTotal += total;
            }

            //OMP atomic update flag - commented out to allow other testing
            /*
            for (int i=0; i<size; i++)
            {
                #pragma omp atomic update
                total += v3[i];  
            }        
            */
        }

        //print statements for critical and total element counts
        printf("Total of elements: %d\n", &total);
        //printf("Critical Total of elements: %d\n", &criticalTotal);

        //OMP barrier flag
        #pragma omp barrier

        auto stop = high_resolution_clock::now();

        //ToDo: Add Comment
        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function: "
            << duration.count() << " microseconds" << endl;

        return 0;
    
}