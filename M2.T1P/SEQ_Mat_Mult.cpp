#include <iostream>
#include <time.h>
#include <chrono>
#include <fstream>

using namespace std::chrono;
using namespace std;

//constant to define matrix size - row & column the same size
const int MATRIX_SIZE = 512;

//define number of threads to use with debug output only
const int NUM_THREADS = 1;

// create arrays/matrices for equation A X B = C
int m1[MATRIX_SIZE][MATRIX_SIZE]; // matrix A
int m2[MATRIX_SIZE][MATRIX_SIZE]; // matrix B
int m3[MATRIX_SIZE][MATRIX_SIZE]; // matrix C


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


//function for multiplying matrices A X B = C
void multiplyMatrix(int matrix1[MATRIX_SIZE][MATRIX_SIZE], int matrix2[MATRIX_SIZE][MATRIX_SIZE], int matrix3[MATRIX_SIZE][MATRIX_SIZE])
{
	for (int i = 0; i < MATRIX_SIZE; i++)
	{
		for (int j = 0; j < MATRIX_SIZE; j++)
		{
			for (int k = 0; k < MATRIX_SIZE; k++)
			{
				matrix3[i][j] += matrix1[i][k] * matrix2[k][j];
			}
		}
	}
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

	//call matrix multiple function and pass A, B and C matrices
	multiplyMatrix(m1, m2, m3);

	// Code to end clock and print runtime to console
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	cout << "SEQ, " << "Threads: " << NUM_THREADS << ", " << "Mutex: " << 0 << ", " 
		<< MATRIX_SIZE << " X " << MATRIX_SIZE 
		<< " Matrix, Duration: " << duration.count() << "us" << endl; 

	//Code to write execution time vs matrix size and paradigm to file
	ofstream metricsFile;
	metricsFile.open("metrics.txt", ios::out | ios::app);

	if (metricsFile.is_open())
	{	
		metricsFile << "SEQ, " << "Threads: " << NUM_THREADS << ", " << "Mutex: " 
		<< 0 << ", " << MATRIX_SIZE << " X " << MATRIX_SIZE 
		<< " Matrix, Duration: " << duration.count() << "us" << endl; 
		metricsFile.close();
	}
	else cout << "Unable to open metrics file" << endl;
	

	//call function and print to file
	printMatrixFile(m3);


	return 0;
}

