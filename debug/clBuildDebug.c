#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

int main() {
    
    cl_platform_id platform; cl_device_id device; cl_context context;
    cl_program program; cl_int error; cl_build_status status;
    
    FILE* programHandle;
    char *programBuffer; char *programLog;
    size_t programSize; size_t logSize;
    
    // get first available platform and gpu and create context
    clGetPlatformIDs(1, &platform, NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    
    
   
    // get size of kernel source
    programHandle = fopen("../kernel.c", "r");
    fseek(programHandle, 0, SEEK_END);
    programSize = ftell(programHandle);
    rewind(programHandle);

    // read kernel source into buffer
    programBuffer = (char*) malloc(programSize + 1);
    programBuffer[programSize] = '\0';
    fread(programBuffer, sizeof(char), programSize, programHandle);
    fclose(programHandle);
    
    // create program from buffer
    program = clCreateProgramWithSource(context, 1,
                                        (const char**) &programBuffer, &programSize, NULL);
    free(programBuffer);
    
    // build program
    const char options[] = "-Werror -cl-std=CL1.1";
    error = clBuildProgram(program, 1, &device, options, NULL, NULL);
    
    // build failed
    if (error != CL_SUCCESS) {
        
        // check build error and build status first
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                              sizeof(cl_build_status), &status, NULL);
        
        // check build log
        clGetProgramBuildInfo(program, device,
                              CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        programLog = (char*) calloc (logSize+1, sizeof(char));
        clGetProgramBuildInfo(program, device,
                              CL_PROGRAM_BUILD_LOG, logSize+1, programLog, NULL);
        printf("Build failed; error=%d, status=%d, programLog:nn%s",
               error, status, programLog);
        free(programLog);
        
    }
    
    clReleaseContext(context);
    return 0;
    
}
