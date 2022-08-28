#include <iostream>
#include <time.h>
#include <chrono>
#include <fstream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std::chrono;
using namespace std;

//constant to define matrix size - row & column the same size
const int MATRIX_SIZE = 512;

//define number of threads to use with pthread
const int NUM_THREADS = 128;

// create arrays/matrices for equation A X B = C
int m1[MATRIX_SIZE][MATRIX_SIZE]; // matrix A
int m2[MATRIX_SIZE][MATRIX_SIZE]; // matrix B
int m3[MATRIX_SIZE][MATRIX_SIZE]; // matrix C

//struct for defining start & end of thread generation
struct taskMulti
{
	int start;
	int end;
};


//function for populating any matrix with random integars 1-100
void populateMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
	for (int i = 0; i < MATRIX_SIZE; i++)
	{
		for (int j = 0; j < MATRIX_SIZE; j++)
		{
			matrix[i][j] = rand() % 100 + 1;
		}
	}
}


//function for multiplying matrices A X B = C - using threads
void *multiplyPTHMatrix(void *arg)
{
	struct taskMulti * range = (struct taskMulti *) arg;
	for (int i = 0; i < MATRIX_SIZE; i++)
	{
		for (int j = 0; j < MATRIX_SIZE; j++)
		{
			for (int k = range->start; k < range->end; k++)
			{
				m3[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return NULL;
}


//funtion to print any matrix passed to it
void printMatrixFile(int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
	ofstream matrixFile;
	matrixFile.open("matrix.txt", ios::out);

	if (matrixFile.is_open())
	{	
		for (int i = 0; i < MATRIX_SIZE; i++)
		{
			for (int j = 0; j < MATRIX_SIZE; j++)
			{
				matrixFile << matrix[i][j] << "\t";
			}
			matrixFile << "\n";
		}
	}
	else cout << "Unable to open file" << endl;
}


//main function
int main()
{
	// using time ensures new random ints generated every execution
	srand(time(0)); 

	// call function and populate A and B matrices
	populateMatrix(m1);
	populateMatrix(m2);

	// Code to start clock - for recording runtime of multiplication & print
	auto start = high_resolution_clock::now();

	//create array of threads indexed to max number of threads constant
	pthread_t PTH[NUM_THREADS];

	//array to hold range
	struct taskMulti rangeCount[NUM_THREADS];

	//partition size and initial/current temp variable
	int initialNow = 0;
	int partitionSize = MATRIX_SIZE / NUM_THREADS;
	

	//create threads for matrix multiplication
	for (int i=0; i < NUM_THREADS; i++)
	{
		rangeCount[i].start = initialNow;
		rangeCount[i].end = initialNow + partitionSize;
		initialNow += partitionSize;
	}
	rangeCount[NUM_THREADS-1].end = MATRIX_SIZE; //final setting of matrix size at end of for loop


	//create pthread objects and reference to multiply function 
	//and range argument for size and partition management
	for (int i=0; i < NUM_THREADS; i++)
	{
		pthread_create(&PTH[i], NULL, multiplyPTHMatrix, &rangeCount[i]);
	}

	//join all threads
	for (int i=0; i < NUM_THREADS; i++)
	{
		pthread_join(PTH[i], NULL);
	}


	// Code to end clock and print runtime to console
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	cout << "PTH, " << "Threads: " << NUM_THREADS << ", " << MATRIX_SIZE << " X " << MATRIX_SIZE 
		<< " Matrix, Duration: " << duration.count() << "us" << endl; 

	//Code to write execution time vs matrix size and paradigm to file
	ofstream metricsFile;
	metricsFile.open("metrics.txt", ios::out | ios::app);

	if (metricsFile.is_open())
	{	
		metricsFile << "PTH, " << "Threads: " << NUM_THREADS << ", " << MATRIX_SIZE 
		<< " X " << MATRIX_SIZE << " Matrix, Duration: " << duration.count() << "us" << endl; 
		metricsFile.close();
	}
	else cout << "Unable to open metrics file" << endl;
	
	//call function and print to file
	printMatrixFile(m3);

	return 0;
}
