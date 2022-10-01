#include <iostream>
#include <array>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <mpi.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

//constant to define matrix size - row & column the same size
const int MATRIX_SIZE = 512;

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


int main(int argc, char *argv[]) {
  int numtasks, rank, num_node_workers, source, tag=1, rowTotal, rowNum, rowWorker, rowLeft;

  double startClock, endClock;
  MPI_Status Stat;

	// using time ensures new random ints generated every execution
	srand(time(0)); 

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  // Number of node workers
  num_node_workers = numtasks - 1;

  //master code
  if(rank == 0) {

	// call function and populate A and B matrices
	populateMatrix(m1);
	populateMatrix(m2);

  // Code to start clock - for recording runtime of multiplication & print, uses MPI execution time inbuilt feature
  startClock = MPI_Wtime();

    // Send matrices to Node Workers
    rowWorker = MATRIX_SIZE / num_node_workers;
    rowLeft = MATRIX_SIZE % num_node_workers;
    rowNum = 0;
    for(int dest = 1; dest <= num_node_workers; dest++) {
      rowTotal = dest <= rowLeft ? rowWorker + 1 : rowWorker;
      
      // Send the number of rows
      MPI_Send(&rowTotal, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
      // Send the row number
      MPI_Send(&rowNum, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
      // Send matrix A
      MPI_Send(&m1[rowNum][0], rowTotal * MATRIX_SIZE, MPI_INT, dest, tag, MPI_COMM_WORLD);
      // Send matrix B
      MPI_Send(&m2, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, dest, tag, MPI_COMM_WORLD);

      rowNum += rowTotal;
    }


    // Receive results from node workers
    for(int i = 1; i <= num_node_workers; i++) {
      MPI_Recv(&rowTotal, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &Stat);
      MPI_Recv(&rowNum, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &Stat);
      MPI_Recv(&m3[rowNum][0], rowTotal * MATRIX_SIZE, MPI_INT, i, 2, MPI_COMM_WORLD, &Stat);
    }

    //Code to stop clock
    endClock = MPI_Wtime();


    //print_matrix(m3); //print matrix output if desired

    //Code to print runtime in ms
    double runTime= double(endClock-startClock) * 1000 * 1000;
    cout << "\nRuntime: " << runTime << "ms\n";
  }

 
  //worker code
  if(rank > 0) {
    source = 0;
    MPI_Recv(&rowTotal, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);
    MPI_Recv(&rowNum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);
    MPI_Recv(&m1, rowTotal * MATRIX_SIZE, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);
    MPI_Recv(&m2, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, 0, tag, MPI_COMM_WORLD, &Stat);
    
    //Matrix multiplication 
    for(int c = 0; c < MATRIX_SIZE; c++) {
      for(int r = 0; r < rowTotal; r++) {
        m3[r][c] = 0;
        for(int i = 0; i < MATRIX_SIZE; i++) {
          m3[r][c] += (m1[r][i] * m2[i][c]);
        }
      }
    }

    MPI_Send(&rowTotal, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(&rowNum, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(&m3, rowTotal * MATRIX_SIZE, MPI_INT, 0, 2, MPI_COMM_WORLD);
  }
  MPI_Finalize();
  return 0;
}

