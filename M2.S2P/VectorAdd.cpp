#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>


using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size)
{
    for (int i = 0; i < size; i++)
    {
        //Fill vector with random integers that are modulus 100
        vector[i] = rand() % 100;
    }
}


int main(){

    unsigned long size = 1000000000L;

    srand(time(0));

    int *v1, *v2, *v3;

    //Code to start clock - for recording runtime
    auto start = high_resolution_clock::now();

    //Memory allocation from heap for each vector
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));


    randomVector(v1, size);

    randomVector(v2, size);


    //for loop to add vector 1 to vector 2 = vector 3
    for (int i = 0; i < size; i++)
    {
        v3[i] = v1[i] + v2[i];
    }

    auto stop = high_resolution_clock::now();

    //Code to end clock - for recording runtime
    auto duration = duration_cast<microseconds>(stop - start);


    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}
