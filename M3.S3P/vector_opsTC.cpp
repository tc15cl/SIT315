
#include <iostream>
#include<stdio.h>
#include <stdlib.h>
#include<CL/cl.h>
#include <chrono>

using namespace std::chrono;
using namespace std;


int NUM_THREADS = 1;
int SZ = 8;
int **A, **B, **C;

//Allocate memory to buffer of elements
cl_mem bufA, bufB, bufC;

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

const int maxSize = SZ;
const int TS = 4;
const size_t local[2] = { (size_t)TS, (size_t)TS };

//SZ = vector size, which can be defined by command line parse during execution, set to default 8
//allocation of memory to global and define as 2-dimensional array
const size_t global[2] = { (size_t) maxSize, (size_t)maxSize }; 


//call create device function, which creates CPU or GPU devices
cl_device_id create_device();

//call function to setup context for device and command queue
void setup_openCL_device_context_queue_kernel(char* filename, char* kernelname);

//call function that creates the program object
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);

//allocates kernal memory to a buffer object
void setup_kernel_memory();

//sets the various kernal arguments
void copy_kernel_args();

//purges old momery from kernels
void free_memory();



void init(int** &A, int rows, int cols, bool initialise);
void add(int** &A, int** &B, int** &C, int rows, int cols);
void print( int** A, int rows, int cols);


int main(int argc, char** argv) {  
    if(argc > 1) SZ = atoi(argv[1]);

    init(A, SZ, SZ, true), init(B, SZ, SZ, true), init(C, SZ, SZ, false);
    
    //print(A, SZ, SZ);
    //print(B, SZ, SZ);

    
    //Clock start
    auto start = high_resolution_clock::now();

    //this function is used to setup the device, context, queue program and kernel -- make sure to set file name (1) and kernel name (2)
   setup_openCL_device_context_queue_kernel( (char*) "./matrix_ops.cl" , (char*) "add_matrices");


   //this function is used to load/copy memory and link arguments -- you will need to change this 
   //for your program as this varies from one implementation to another
   setup_kernel_memory();
   copy_kernel_args();


   //setup the queue and wait for commands
   clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
   clWaitForEvents(1, &event);


   //reading data back from the queue buffer
   clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, SZ * SZ *sizeof(int), &C[0][0], 0, NULL, NULL);

   //print(C, SZ, SZ);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Time taken by function: "
    << duration.count() << " microseconds" <<endl;
   
   //frees memory for device, kernel, queue, etc.
   //you will need to modify this to free your own buffers
   free_memory();
}  


void init(int** &A, int rows, int cols, bool initialise) {
    A = (int **) malloc(sizeof(int*) * rows * cols);  // number of rows * size of int* address in the memory
    int* tmp = (int *) malloc(sizeof(int) * cols * rows); 

    for(int i = 0 ; i < SZ ; i++) {
        A[i] = &tmp[i * cols];
    }
  

    if(!initialise) return;

    for(long i = 0 ; i < rows; i++) {
        for(long j = 0 ; j < cols; j++) {
            A[i][j] = rand() % 100; // any number less than 100
        }
    }
}

void print( int** A, int rows, int cols) {
  for(long i = 0 ; i < rows; i++) { //rows
        for(long j = 0 ; j < cols; j++) {  //cols
            printf("%d ",  A[i][j]); // print the cell value
        }
        printf("\n"); //at the end of the row, print a new line
    }
    printf("----------------------------\n");
}

void add(int** &A, int** &B, int** &C, int rows, int cols) {
    for(long i = 0 ; i < rows; i++) { //rows
        for(long j = 0 ; j < cols; j++) {  //cols
            C[i][j] = A[i][j] + B[i][j];
        }
    }
}
void free_memory() {
   //free the buffers
   clReleaseMemObject(bufA);
   clReleaseMemObject(bufB);
   clReleaseMemObject(bufC);

    //free opencl objects
   clReleaseKernel(kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
   
}
void copy_kernel_args() {
	//sets kernel arguments, defining kernal, argument index, argument value and argument size
    clSetKernelArg(kernel, 0, sizeof(int), (void*)&maxSize);
    clSetKernelArg(kernel, 1, sizeof(int), (void*)&maxSize);
    clSetKernelArg(kernel, 2, sizeof(int), (void*)&maxSize);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bufA);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&bufB);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&bufC);
    if(err < 0) {
      perror("Couldn't create a kernel argument");
      printf("error = %d", err);
      exit(1);
   }
}
void setup_kernel_memory() {
	//creates buffer and passes arguments context, memory flags, size, pointer to host and and error code handler
    //cl_mem_flags defines the type of memory: read/write/pointer etc
     bufA = clCreateBuffer(context, CL_MEM_READ_ONLY,  SZ*SZ*sizeof(int), NULL, NULL);
     bufB = clCreateBuffer(context, CL_MEM_READ_ONLY,  SZ*SZ*sizeof(int), NULL, NULL);
     bufC = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ*SZ*sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0, SZ*SZ*sizeof(int), &A[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufB, CL_TRUE, 0, SZ*SZ*sizeof(int), &B[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufC, CL_TRUE, 0, SZ*SZ*sizeof(int), &C[0][0], 0, NULL, NULL);

}

void setup_openCL_device_context_queue_kernel(char* filename, char* kernelname) {
    device_id = create_device();
    cl_int err;
	//creates temporary buffer to hold data during execution
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
   if(err < 0) {
      perror("Couldn't create a context");
      exit(1);   
    }

    program = build_program(context, device_id, filename );
	//creates command queue object 
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if(err < 0) {
      perror("Couldn't create a command queue");
      exit(1);   
    };

    kernel = clCreateKernel(program, kernelname, &err);
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
