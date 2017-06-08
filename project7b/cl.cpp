// 1. Program header

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <omp.h>
#include <fstream>

#include "CL/cl.h"
#include "CL/cl_platform.h"

#ifndef LOCAL_SIZE
#define	LOCAL_SIZE		64
#endif

#ifndef NMB
#define NMB         64
#endif

#define NUM_ELEMENTS        NMB*NUM
#define NUM_WORK_GROUPS     NUM_ELEMENTS/LOCAL_SIZE

const char *            CL_FILE_NAME = { "cl_parallelism.cl" };
const float TOL = 0.0001f;

void Wait( cl_command_queue );
int             LookAtTheBits( float );


int
main( int argc, char *argv[] )
{
        // see if we can even open the opencl kernel program
        // (no point going on if we can't):

        FILE *fp;
#ifdef WIN32
        errno_t err = fopen_s( &fp, CL_FILE_NAME, "r" );
        if( err != 0 )
#else
        fp = fopen( CL_FILE_NAME, "r" );
        if( fp == NULL )
#endif
        {
                fprintf( stderr, "Cannot open OpenCL source file '%s'\n", CL_FILE_NAME );
                return 1;
        }

        cl_int status;  // returned status from opencl calls
        // test against CL_SUCCESS

        // get the platform id:

        cl_platform_id platform;
        status = clGetPlatformIDs( 1, &platform, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clGetPlatformIDs failed (2)\n" );

        // get the device id:

        cl_device_id device;
        status = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clGetDeviceIDs failed (2)\n" );

        // read in the data
        FILE *file = fopen("signal.txt", "r");
        int size;
        fscanf(file, "%d", &size);

        int numWorkGroups = size / LOCAL_SIZE;

        // 2. allocate the host memory buffers:

        float *hArray = new float[2 * size];
    	float *hSums = new float[size];

        size_t arraySize = 2 * size * sizeof(float);
        size_t sumsSize = size * sizeof(float);

        // fill the host memory buffers:

        for (int i = 0; i < size; i++) {
            fscanf(file, "%f", &hArray[i]);
            hArray[i + size] = hArray[i];
        }
        fclose(file);

    	size_t dataSize = size * sizeof(float);

        // 3. create an opencl context:

        cl_context context = clCreateContext( NULL, 1, &device, NULL, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateContext failed\n" );

        // 4. create an opencl command queue:

        cl_command_queue cmdQueue = clCreateCommandQueue( context, device, 0, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateCommandQueue failed\n" );

        // 5. allocate the device memory buffers:

    	cl_mem dArray = clCreateBuffer( context, CL_MEM_READ_ONLY, arraySize, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateBuffer failed (1)\n" );

    	cl_mem dSums = clCreateBuffer( context, CL_MEM_WRITE_ONLY, sumsSize, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateBuffer failed (2)\n" );

        // 6. enqueue the 2 commands to write the data from the host buffers to the device buffers:

    	status = clEnqueueWriteBuffer( cmdQueue, dArray, CL_FALSE, 0, arraySize, hArray, 0, NULL, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clEnqueueWriteBuffer failed (1)\n" );

    	status = clEnqueueWriteBuffer( cmdQueue, dSums, CL_FALSE, 0, sumsSize, hSums, 0, NULL, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clEnqueueWriteBuffer failed (2)\n" );

        Wait( cmdQueue );

        // 7. read the kernel code from a file:

        fseek( fp, 0, SEEK_END );
        size_t fileSize = ftell( fp );
        fseek( fp, 0, SEEK_SET );
        char *clProgramText = new char[ fileSize+1 ]; // leave room for '\0'
        size_t n = fread( clProgramText, 1, fileSize, fp );
        clProgramText[fileSize] = '\0';
        fclose( fp );
        if( n != fileSize )
                fprintf( stderr, "Expected to read %d bytes read from '%s' -- actually read %d.\n", fileSize, CL_FILE_NAME, n );

        // create the text for the kernel program:

        char *strings[1];
        strings[0] = clProgramText;
        cl_program program = clCreateProgramWithSource( context, 1, (const char **)strings, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateProgramWithSource failed\n" );
        delete [] clProgramText;

        // 8. compile and link the kernel code:

        const char *options = { "" };

        status = clBuildProgram( program, 1, &device, options, NULL, NULL );
        if( status != CL_SUCCESS )
        {
                size_t size;
                clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size );
                cl_char *log = new cl_char[ size ];
                clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, size, log, NULL );
                fprintf( stderr, "clBuildProgram failed:\n%s\n", log );
                delete [] log;
        }

        // 9. create the kernel object:

        cl_kernel kernel = clCreateKernel( program, "AutoCorrelate", &status );
    	if( status != CL_SUCCESS )
    		fprintf( stderr, "clCreateKernel failed\n" );

        // 10. setup the arguments to the kernel object:

    	status = clSetKernelArg( kernel, 0, sizeof(cl_mem), &dArray );
    	if( status != CL_SUCCESS )
    		fprintf( stderr, "clSetKernelArg failed (1)\n" );

    	status = clSetKernelArg( kernel, 1, sizeof(cl_mem), &dSums );
    	if( status != CL_SUCCESS )
    		fprintf( stderr, "clSetKernelArg failed (2)\n" );


    	// 11. enqueue the kernel object for execution:

    	size_t globalWorkSize[3] = { size, 1, 1 };
    	size_t localWorkSize[3]  = { LOCAL_SIZE,   1, 1 };

    	Wait( cmdQueue );
    	double time0 = omp_get_wtime( );

    	time0 = omp_get_wtime( );

    	status = clEnqueueNDRangeKernel( cmdQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL );
    	if( status != CL_SUCCESS ) {
    		fprintf( stderr, "clEnqueueNDRangeKernel failed: %d\n", status );
        }

        status = clEnqueueReadBuffer( cmdQueue, dSums, CL_TRUE, 0, sumsSize, hSums, 0, NULL, NULL );
        if( status != CL_SUCCESS )
            fprintf( stderr, "clEnqueueReadBuffer failed\n" );

    	Wait( cmdQueue );
    	double time1 = omp_get_wtime( );

        // #if defined MULT
        // // Write to file
        // std::ofstream outputFile;
        // outputFile.open("outputMULT.csv", std::ios_base::app);
        // outputFile << NUM_ELEMENTS << "," << LOCAL_SIZE << "," << NUM_WORK_GROUPS << "," << sum_perfect / 10 << "," << max_perfect << std::endl;
        // #elif defined MULT_ADD
        // std::ofstream outputFile;
        // outputFile.open("outputMULT_ADD.csv", std::ios_base::app);
        // outputFile << NUM_ELEMENTS << "," << LOCAL_SIZE << "," << NUM_WORK_GROUPS << "," << sum_perfect / 10 << "," << max_perfect << std::endl;
        // #endif

        std::ofstream outputFile;
        outputFile.open("cl_parallelism.csv", std::ios_base::app);
        outputFile << "Peak Performance (Mega Adds Per Second)" << std::endl;
        outputFile << (double)size * size / (time1 - time0) / 1000000 << std::endl;
        outputFile << "Index" << "," << "Sum" << std::endl;

        for (int i = 0; i < size; i++) {
            outputFile << i << "," << hSums[i] << std::endl;
        }

        outputFile.close();


        // outputFile << NUM_ELEMENTS << "," << LOCAL_SIZE << "," << NUM_WORK_GROUPS << "," << sum_perfect / 10 << "," << max_perfect << std::endl;
        // fprintf(file, "Peak Performance (Mega Adds Per Second)\n");

        // fprintf(stderr, "\nNUM_ELEMENTS: %7d\tLOCAL_SIZE: %4d\tNUM_WORK_GROUPS: %10d\n", NUM_ELEMENTS, LOCAL_SIZE, NUM_WORK_GROUPS);
        // fprintf(stderr, "Avg Perfect %10.3lf MegaMultsPerSec\nMax Perfect %10.3lf MegaMultsPerSec\n", sum_perfect / 10, max_perfect );

#ifdef WIN32
        Sleep( 2000 );
#endif


        // 13. clean everything up:

        clReleaseKernel(        kernel   );
        clReleaseProgram(       program  );
        clReleaseCommandQueue(  cmdQueue );
        clReleaseMemObject(     dArray  );
        clReleaseMemObject(     dSums  );

        delete [ ] hArray;
        delete [ ] hSums;

        return 0;
}


int
LookAtTheBits( float fp )
{
        int *ip = (int *)&fp;
        return *ip;
}


// wait until all queued tasks have taken place:

void
Wait( cl_command_queue queue )
{
        cl_event wait;
        cl_int status;

        status = clEnqueueMarker( queue, &wait );
        if( status != CL_SUCCESS )
                fprintf( stderr, "Wait: clEnqueueMarker failed\n" );

        status = clWaitForEvents( 1, &wait );
        if( status != CL_SUCCESS )
                fprintf( stderr, "Wait: clWaitForEvents failed\n" );
}