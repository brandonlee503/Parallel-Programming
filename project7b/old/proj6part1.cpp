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


#ifndef NMB
#define NMB         64
#endif

#define NUM_ELEMENTS        NMB*NUM
#define NUM_WORK_GROUPS     NUM_ELEMENTS/LOCAL_SIZE

const char *            CL_FILE_NAME = { "project6.cl" };
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

        // 2. allocate the host memory buffers:

        float *hA = new float[ NUM_ELEMENTS ];
        float *hB = new float[ NUM_ELEMENTS ];
        float *hC = new float[ NUM_ELEMENTS ];
        float *hF = new float[ NUM_ELEMENTS ];


#ifdef MULT_ADD
        float *hD = new float [ NUM_ELEMENTS ];
#endif

        // fill the host memory buffers:

        for( int i = 0; i < NUM_ELEMENTS; i++ )
        {
                hA[i] = hB[i] = (float) sqrt(  (double)i  );
                hF[i] = 0.0;

#ifdef MULT_ADD
                hD[i] = hA[i];
#endif
        }

        size_t dataSize = NUM_ELEMENTS * sizeof(float);

        // 3. create an opencl context:

        cl_context context = clCreateContext( NULL, 1, &device, NULL, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateContext failed\n" );

        // 4. create an opencl command queue:

        cl_command_queue cmdQueue = clCreateCommandQueue( context, device, 0, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateCommandQueue failed\n" );

        // 5. allocate the device memory buffers:

        cl_mem dA = clCreateBuffer( context, CL_MEM_READ_ONLY,  dataSize, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateBuffer failed (1)\n" );

        cl_mem dB = clCreateBuffer( context, CL_MEM_READ_ONLY,  dataSize, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateBuffer failed (2)\n" );

        cl_mem dC = clCreateBuffer( context, CL_MEM_WRITE_ONLY, dataSize, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateBuffer failed (3)\n" );

        cl_mem dZ = clCreateBuffer( context, CL_MEM_WRITE_ONLY, dataSize, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateBuffer failed (3)\n" );


#ifdef MULT_ADD
        cl_mem dD = clCreateBuffer( context, CL_MEM_WRITE_ONLY, dataSize, NULL, &status );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateBuffer failed (3)\n" );
#endif
        // 6. enqueue the 2 commands to write the data from the host buffers to the device buffers:

        status = clEnqueueWriteBuffer( cmdQueue, dA, CL_FALSE, 0, dataSize, hA, 0, NULL, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clEnqueueWriteBuffer failed (1)\n" );

        status = clEnqueueWriteBuffer( cmdQueue, dB, CL_FALSE, 0, dataSize, hB, 0, NULL, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clEnqueueWriteBuffer failed (2)\n" );

        status = clEnqueueWriteBuffer( cmdQueue, dZ, CL_FALSE, 0, dataSize, hF, 0, NULL, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clEnqueueWriteBuffer failed (2)\n" );

#ifdef MULT_ADD
        status = clEnqueueWriteBuffer( cmdQueue, dD, CL_FALSE, 0, dataSize, hD, 0, NULL, NULL );
        if( status != CL_SUCCESS )
                fprintf( stderr, "clEnqueueWriteBuffer failed (2)\n" );
#endif
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

#if defined MULT
        cl_kernel kernel = clCreateKernel( program, "ArrayMult", &status );
#elif defined MULT_ADD
        cl_kernel kernel = clCreateKernel( program, "ArrayMultAdd", &status );
#endif
        if( status != CL_SUCCESS )
                fprintf( stderr, "clCreateKernel failed\n" );

        double sum_perfect = 0.0;
        double max_perfect = 0.0;

        for (int i = 0; i < 10; i++) {
                cl_float pattern = 0.0;
                status =  clEnqueueCopyBuffer ( cmdQueue, dZ, dC, 0, 0, dataSize, 0, NULL, NULL);
                if( status != CL_SUCCESS )
                        fprintf( stderr, "clCopyBuffer failed\n" );
                // 10. setup the arguments to the kernel object:

                status = clSetKernelArg( kernel, 0, sizeof(cl_mem), &dA );
                if( status != CL_SUCCESS )
                        fprintf( stderr, "clSetKernelArg failed (1)\n" );

                status = clSetKernelArg( kernel, 1, sizeof(cl_mem), &dB );
                if( status != CL_SUCCESS )
                        fprintf( stderr, "clSetKernelArg failed (2)\n" );

                status = clSetKernelArg( kernel, 2, sizeof(cl_mem), &dC );
                if( status != CL_SUCCESS )
                        fprintf( stderr, "clSetKernelArg failed (3)\n" );

#ifdef MULT_ADD
                status = clSetKernelArg( kernel, 3, sizeof(cl_mem), &dD );
                if( status != CL_SUCCESS )
                        fprintf( stderr, "clSetKernelArg failed (3)\n" );
#endif

                // 11. enqueue the kernel object for execution:

                size_t globalWorkSize[3] = { NUM_ELEMENTS, 1, 1 };
                size_t localWorkSize[3]  = { LOCAL_SIZE,   1, 1 };

                Wait( cmdQueue );
                double time0 = omp_get_wtime( );

                time0 = omp_get_wtime( );

                status = clEnqueueNDRangeKernel( cmdQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL );
                if( status != CL_SUCCESS )
                        fprintf( stderr, "clEnqueueNDRangeKernel failed: %d\n", status );

                Wait( cmdQueue );
                double time1 = omp_get_wtime( );

                // 12. read the results buffer back from the device to the host:

                status = clEnqueueReadBuffer( cmdQueue, dC, CL_TRUE, 0, dataSize, hC, 0, NULL, NULL );
                if( status != CL_SUCCESS )
                        fprintf( stderr, "clEnqueueReadBuffer failed\n" );

                // did it work?
                double perfect = (double)NUM_ELEMENTS/(time1-time0)/1000000.0;
                if (perfect > max_perfect)
                        max_perfect = perfect;
                sum_perfect += perfect;
        }

        #if defined MULT
        // Write to file
        std::ofstream outputFile;
        outputFile.open("outputMULT.csv", std::ios_base::app);
        outputFile << NUM_ELEMENTS << "," << LOCAL_SIZE << "," << NUM_WORK_GROUPS << "," << sum_perfect / 10 << "," << max_perfect << std::endl;
        #elif defined MULT_ADD
        std::ofstream outputFile;
        outputFile.open("outputMULT_ADD.csv", std::ios_base::app);
        outputFile << NUM_ELEMENTS << "," << LOCAL_SIZE << "," << NUM_WORK_GROUPS << "," << sum_perfect / 10 << "," << max_perfect << std::endl;
        #endif

        fprintf(stderr, "\nNUM_ELEMENTS: %7d\tLOCAL_SIZE: %4d\tNUM_WORK_GROUPS: %10d\n", NUM_ELEMENTS, LOCAL_SIZE, NUM_WORK_GROUPS);
        fprintf(stderr, "Avg Perfect %10.3lf MegaMultsPerSec\nMax Perfect %10.3lf MegaMultsPerSec\n", sum_perfect / 10, max_perfect );

#ifdef WIN32
        Sleep( 2000 );
#endif


        // 13. clean everything up:

        clReleaseKernel(        kernel   );
        clReleaseProgram(       program  );
        clReleaseCommandQueue(  cmdQueue );
        clReleaseMemObject(     dA  );
        clReleaseMemObject(     dB  );
        clReleaseMemObject(     dC  );

        delete [] hA;
        delete [] hB;
        delete [] hC;

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
