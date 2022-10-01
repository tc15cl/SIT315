#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <mpi.h>
#include <iostream>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace std::chrono;

//constant to define matrix size - row & column the same size
const int MATRIX_SIZE = 512;

//object to hold device IDs
cl_device_id device_id;

//object to store contexts - definition of environment
cl_context context;

//object to hold host program
cl_program program;

//this is your kernel function
cl_kernel kernel; 

//object to define queue of commands
cl_command_queue  queue;
cl_event event = NULL;

int err;


// create arrays/matrices for equation A X B = C
int m1[MATRIX_SIZE][MATRIX_SIZE]; // matrix A
int m2[MATRIX_SIZE][MATRIX_SIZE]; // matrix B
int m3[MATRIX_SIZE][MATRIX_SIZE]; // matrix C

//Allocate memory to buffer of elements
cl_mem bufA, bufB, bufC;

const int MATRIX_SIZE_size = MATRIX_SIZE;
const int TS = 2;
const size_t local[2] = { TS, TS };
const size_t global[2] = { MATRIX_SIZE_size, MATRIX_SIZE_size }; 
void init (int m1[MATRIX_SIZE][MATRIX_SIZE]);
void matrix_mul(int m1[MATRIX_SIZE][MATRIX_SIZE], int m2[MATRIX_SIZE][MATRIX_SIZE], int m3[MATRIX_SIZE][MATRIX_SIZE]) ;
void print_matrix(int m1[MATRIX_SIZE][MATRIX_SIZE]) ;

cl_device_id create_device();
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);

void setup_openCL_device_context_queue_kernel();
void setup_kernel_memory(int m1[][MATRIX_SIZE], int m2[][MATRIX_SIZE], int m3[][MATRIX_SIZE]);
void copy_kernel_args();
void free_memory();
int main(int argc, char *argv[]) {  
  int numtasks, rank, num_node_workers, tag=1, wtag=2, rowTotal, rowNum, rowWorker, rowLeft;
  double startClock, endClock;
  MPI_Status status;

  // Seed rand function
  srand(time(0));

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  // Number of node workers
  num_node_workers = numtasks - 1;

   //master code
  if(rank == 0) {
    
    //call OpenCL init function and populate A and B matrices
    init(m1);
    init(m2);

    // Code to start clock - for recording runtime of multiplication & print, uses MPI execution time inbuilt feature
    startClock = MPI_Wtime();

    // Send matrices to Node Workers
    rowWorker = MATRIX_SIZE / num_node_workers;
    rowLeft = MATRIX_SIZE % num_node_workers;
    rowNum = 0;
    for(int i = 1; i <= num_node_workers; i++) {
      rowTotal = i <= rowLeft ? rowWorker + 1 : rowWorker;
      
      
      // Send the number of rows, current row and matrices
      MPI_Send(&rowTotal, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
      MPI_Send(&rowNum, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
      MPI_Send(&m1[rowNum][0], rowTotal * MATRIX_SIZE, MPI_INT, i, tag, MPI_COMM_WORLD);
      MPI_Send(&m2, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, i, tag, MPI_COMM_WORLD);

      rowNum += rowTotal;
    }


    // Receive results from node workers
    for(int i = 1; i <= num_node_workers; i++) {
      MPI_Recv(&rowTotal, 1, MPI_INT, i, wtag, MPI_COMM_WORLD, &status);
      MPI_Recv(&rowNum, 1, MPI_INT, i, wtag, MPI_COMM_WORLD, &status);
      MPI_Recv(&m3[rowNum][0], rowTotal * MATRIX_SIZE, MPI_INT, i, wtag, MPI_COMM_WORLD, &status);
      
    }

    // Stop measuring execution time of multiplication operation
    endClock = MPI_Wtime();

    //Code to print runtime in us
    double runTime= double(endClock-startClock) * 1000 * 1000;
    cout << "\nRuntime: " << runTime << "ms\n";
  }

   //worker code
  if(rank > 0) {
    MPI_Recv(&rowTotal, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    MPI_Recv(&rowNum, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    MPI_Recv(&m1, rowTotal * MATRIX_SIZE, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    MPI_Recv(&m2, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    


    setup_openCL_device_context_queue_kernel();
    setup_kernel_memory(m1,m2,m3);
    copy_kernel_args();

    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
    clWaitForEvents(1, &event);

     //copying data from the device back to host c matrix
    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, MATRIX_SIZE * MATRIX_SIZE*sizeof(int), m3, 0, NULL, NULL);
    
                            
    free_memory();

    
    MPI_Send(&rowTotal, 1, MPI_INT, 0, wtag, MPI_COMM_WORLD);
    MPI_Send(&rowNum, 1, MPI_INT, 0, wtag, MPI_COMM_WORLD);
    MPI_Send(&m3, rowTotal * MATRIX_SIZE, MPI_INT, 0, wtag, MPI_COMM_WORLD);
  }
  MPI_Finalize();

}  

void free_memory() {
   
   clReleaseKernel(kernel);
   clReleaseMemObject(bufA);
   clReleaseMemObject(bufB);
   clReleaseMemObject(bufC);

   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
}
void copy_kernel_args() {
   clSetKernelArg(kernel, 0, sizeof(int), (void*)&MATRIX_SIZE_size);
   clSetKernelArg(kernel, 1, sizeof(int), (void*)&MATRIX_SIZE_size);
   clSetKernelArg(kernel, 2, sizeof(int), (void*)&MATRIX_SIZE_size);
   clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bufA);
   clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&bufB);
   clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&bufC);
   if(err < 0) {
      perror("Couldn't create a kernel argument");
      printf("error = %d", err);
      exit(1);
   }
}
void setup_kernel_memory(int m1[][MATRIX_SIZE], int m2[][MATRIX_SIZE], int m3[][MATRIX_SIZE]) {
   bufA = clCreateBuffer(context, CL_MEM_READ_ONLY,  MATRIX_SIZE*MATRIX_SIZE*sizeof(int), NULL, NULL);
   bufB = clCreateBuffer(context, CL_MEM_READ_ONLY,  MATRIX_SIZE*MATRIX_SIZE*sizeof(int), NULL, NULL);
   bufC = clCreateBuffer(context, CL_MEM_READ_WRITE, MATRIX_SIZE*MATRIX_SIZE*sizeof(int), NULL, NULL);

   // Copy matrices to the GPU
   clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0, MATRIX_SIZE*MATRIX_SIZE*sizeof(int), m1, 0, NULL, NULL);
   clEnqueueWriteBuffer(queue, bufB, CL_TRUE, 0, MATRIX_SIZE*MATRIX_SIZE*sizeof(int), m2, 0, NULL, NULL);
   clEnqueueWriteBuffer(queue, bufC, CL_TRUE, 0, MATRIX_SIZE*MATRIX_SIZE*sizeof(int), m3, 0, NULL, NULL);

}

void setup_openCL_device_context_queue_kernel() {
   device_id = create_device();
   cl_int err;
   context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
   if(err < 0) {
      perror("Couldn't create a context");
      exit(1);   
   }

   program = build_program(context, device_id, "MatMul.cl");

   

    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
   if(err < 0) {
      perror("Couldn't create a command queue");
      exit(1);   
    };

   kernel = clCreateKernel(program, "multiply_matrices", &err);
   if(err < 0) {
      perror("Couldn't create a kernel");
      printf("error =%d", err);
      exit(1);
   };

}
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
  

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file 

   Creates a program from the source code in the add_numbers.cl file. 
   Specifically, the code reads the file's content into a char array 
   called program_buffer, and then calls clCreateProgramWithSource.
   */
   program = clCreateProgramWithSource(ctx, 1, 
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program 

   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}


void init (int m1[MATRIX_SIZE][MATRIX_SIZE]) {
    for(int i=0; i < MATRIX_SIZE; i++) {
        int add = 0;
       for(int j=0; j < MATRIX_SIZE; j++) {
           m1[i][j] = rand() % 10 ;//+ add;
           add += 10;
        } 
    }
}
