#include "main.hpp"

#define OVERLAP
#define CHECK
//#define DEBUG

//Initialize opencl
cl_device_id _deviceId;
cl_context _deviceContext;              // OpenCL context
cl_command_queue _commandQueue[MAX_Q];  // OpenCL command queues
cl_platform_id _platformId;
cl_program _Program;
cl_kernel _Kernel;
double* validMem;

double* hostSrc[NSTAGES];
double* hostDst[NSTAGES];
cl_mem pinnedBufferSrc[NSTAGES];
cl_mem pinnedBufferDst[NSTAGES];
cl_mem devSrc[NSTAGES];
cl_mem devDst[NSTAGES];

size_t memSize[NSIZES] = {      1024,     2048,      4096,      8192,     16384,\
//                                1K        2K         4K         8K        16K
                               32768,    65536,    131072,    262144,    524288,\
//                               32K       64K       128K       256K       512K
                             1048576,  2097152,   4194304,   8388608,  16777216};
//                                1M        2M         4M         8M        16M
    i64 freq, start, iclock;

void TimerStart(void)
{
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER *)&start);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
#else
    struct timespec s;
    assert(clock_gettime(CLOCK_REALTIME, &s ) == SDK_SUCCESS);
    start = (i64)s.tv_sec * 1e9 + (i64)s.tv_nsec;
    freq = 1000000000;
#endif
}

void TimerStop(void)
{
    i64 n;
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER *)&n);
#else
    struct timespec s;
    assert(clock_gettime(CLOCK_REALTIME, &s ) == SDK_SUCCESS);
    n = (i64)s.tv_sec * 1e9 + (i64)s.tv_nsec;
#endif
    n -= start;
    start = 0;
    iclock += n;
}

void TimerReset(void)
{
    iclock = 0;
}

double GetElapsedTime(void)
{
#ifdef _WIN32
    return (double)iclock / (double) freq;
#else
    return (double)iclock / (double) 1e9;
#endif
}

char* oclGetOpenCLErrorCodeStr(cl_int input)
{
    int errorCode = (int)input;
    switch(errorCode)
    {
        case CL_SUCCESS:
            return (char*) "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND:
            return (char*) "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:
            return (char*) "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE:
            return (char*) "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return (char*) "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES:
            return (char*) "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY:
            return (char*) "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return (char*) "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP:
            return (char*) "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH:
            return (char*) "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return (char*) "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE:
            return (char*) "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE:
            return (char*) "CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return (char*) "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return (char*) "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_COMPILE_PROGRAM_FAILURE:
            return (char*) "CL_COMPILE_PROGRAM_FAILURE";
        case CL_LINKER_NOT_AVAILABLE:
            return (char*) "CL_LINKER_NOT_AVAILABLE";
        case CL_LINK_PROGRAM_FAILURE:
            return (char*) "CL_LINK_PROGRAM_FAILURE";
        case CL_DEVICE_PARTITION_FAILED:
            return (char*) "CL_DEVICE_PARTITION_FAILED";
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
            return (char*) "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
        case CL_INVALID_VALUE:
            return (char*) "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE:
            return (char*) "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM:
            return (char*) "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE:
            return (char*) "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT:
            return (char*) "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES:
            return (char*) "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE:
            return (char*) "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR:
            return (char*) "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT:
            return (char*) "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return (char*) "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE:
            return (char*) "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER:
            return (char*) "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY:
            return (char*) "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS:
            return (char*) "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM:
            return (char*) "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return (char*) "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME:
            return (char*) "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION:
            return (char*) "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL:
            return (char*) "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX:
            return (char*) "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE:
            return (char*) "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE:
            return (char*) "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS:
            return (char*) "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION:
            return (char*) "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE:
            return (char*) "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE:
            return (char*) "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET:
            return (char*) "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST:
            return (char*) "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT:
            return (char*) "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION:
            return (char*) "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT:
            return (char*) "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE:
            return (char*) "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL:
            return (char*) "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return (char*) "CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_PROPERTY:
            return (char*) "CL_INVALID_PROPERTY";
        case CL_INVALID_IMAGE_DESCRIPTOR:
            return (char*) "CL_INVALID_IMAGE_DESCRIPTOR";
        case CL_INVALID_COMPILER_OPTIONS:
            return (char*) "CL_INVALID_COMPILER_OPTIONS";
        case CL_INVALID_LINKER_OPTIONS:
            return (char*) "CL_INVALID_LINKER_OPTIONS";
        case CL_INVALID_DEVICE_PARTITION_COUNT:
            return (char*) "CL_INVALID_DEVICE_PARTITION_COUNT";
        default:
            return (char*) "unknown error code";
  }

    return (char*) "unknown error code";
}

int main(const int argc, const char** argv) 
{
    //Lock this onto a single processor to make timing easier.
#ifdef _WIN32
    HANDLE hProcess = GetCurrentProcess();
    SetProcessAffinityMask( hProcess, 2L );
#endif
#ifdef __linux__
    cpu_set_t cpu_mask;
    unsigned int len = sizeof( cpu_mask );
    pid_t pid;
    CPU_ZERO( &cpu_mask );
    cpu_mask.__bits[0] = 0x2L;
    if( sched_setaffinity(0, len, &cpu_mask) )
    {
      perror( "sched_setaffinity" );
      return -1;
    }
#endif

    //Initialize opencl
    cl_int err;
    cl_device_type type = CL_DEVICE_TYPE_GPU;
    if(((err = clGetPlatformIDs(1, &_platformId, NULL) == SDK_SUCCESS)) &&
        ((err = clGetDeviceIDs(_platformId, type, 1, &_deviceId, NULL)) == SDK_SUCCESS))
    {
        //Create the context
        _deviceContext = clCreateContext(0, 1, &_deviceId, NULL, NULL, &err);
        //And 3 command queues
        if (err == SDK_SUCCESS)
        {
            for (int i=0; i<MAX_Q; i++)
            {
                _commandQueue[i] = clCreateCommandQueue(_deviceContext, _deviceId, 0, &err);
                if (err != SDK_SUCCESS) {
                   printf("clCreateCommandQueue() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
            }
        }
    } else {
        exit (-10);
    }

    char *compilerOptions = (char*) malloc(240*sizeof(char));
    err = sprintf(compilerOptions," -DMAXCOMP=%d",MAXCOMP);
    printf("OpenCL compilation options: %s\n",compilerOptions);

    const char * kernelSrc = "\
//#if (__OPENCL_VERSION__ < CL_VERSION_1_2) \n \
#pragma OPENCL EXTENSION cl_khr_fp64 : enable \n \
//#endif \n \
 \n \
__kernel void Compute_Kernel(__global double *input, \n \
                             __global double *output, \n \
                             const int n) \n \
{ \n \
   int tid=get_global_id(0); \n \
 \n \
   double tmp = input[tid]; \n \
#pragma unroll \n \
   for (int ic=0; ic < MAXCOMP; ic++) \n \
      tmp = sin(tmp); \n \
   output[tid] = tmp; \n \
} \n \
";

    size_t SrcSize[] = {strlen(kernelSrc)};
    _Program = clCreateProgramWithSource(_deviceContext, 1, &kernelSrc, SrcSize, &err);
    if (err != SDK_SUCCESS) {
       printf("clCreateProgramWithSource() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
       printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
       exit(err);
    }

    // create a cl program executable for all the devices specified
    err = clBuildProgram(_Program, 1, &_deviceId, compilerOptions, NULL, NULL);
    char* build_log;
    size_t log_size;
    // First call to know the proper size
    printf(" -- Compilation log --\n\n");
    clGetProgramBuildInfo(_Program, _deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    build_log = (char*) malloc((log_size+1)*sizeof(char));
    memset(build_log, 0, log_size);
    // Second call to get the log
    clGetProgramBuildInfo(_Program, _deviceId, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
    build_log[log_size] = '\0';
    printf("%s\n",build_log);
    free(build_log);
    free(compilerOptions);
    printf(" -- End of log --\n\n");
    if(err != SDK_SUCCESS) {
        printf("can't build program %d, abort! \n",err);
        exit(1);
    }

    // get a kernel object handle for a kernel with the given name
    _Kernel = clCreateKernel(_Program, "Compute_Kernel", &err);
    if (err != SDK_SUCCESS) {
       printf("clCreateKernel() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
       printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
       exit(err);
    }

    // Conversion factor bytes to MB, iteration count
    double KB = 1024.;
    double MB = KB*KB; 
    cl_mem devPinnedBuffer = NULL;

    // Init random generator
    srand((unsigned int)time(NULL));
    double value = 0, value_old = 0;
    double zero = 0;
#ifdef _WIN32
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    int pageSize = system_info.dwPageSize;
#endif
#ifdef __linux__
    int pageSize = getpagesize();
#endif
#ifndef OVERLAP
    cl_int param_value;
    size_t param_value_size_ret;
#endif
    printf("System page size=%d Bytes\n",pageSize);
#ifdef OVERLAP
    printf("Overlapping data transfers with computation\n");
#endif

    printf("\nTest started:\n-------------\n");

    //Allocate device memory 
    for (int sizeCount=0; sizeCount < NSIZES; sizeCount++)
    {
        printf("\nBuffer size: %d Bytes (%.0f MBytes)\n",memSize[sizeCount],memSize[sizeCount]/MB);
#ifdef _WIN32
        validMem = (double*) _aligned_malloc(memSize[sizeCount],pageSize);
#else
        validMem = (double*) memalign(pageSize,memSize[sizeCount]);
#endif
        for (int ns=0; ns < NSTAGES; ns++)
        {

            pinnedBufferSrc[ns] = clCreateBuffer(_deviceContext,
                                                 CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                                 memSize[sizeCount],
                                                 NULL,
                                                 &err);
            if (err != SDK_SUCCESS) {
               printf("clCreateBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }

#ifdef DEBUG
            printf("DEBUG: for hostSrc ns=%d memSize[sizeCount]=%d\n",ns,memSize[sizeCount]);
#endif
            hostSrc[ns] = (double*) clEnqueueMapBuffer(_commandQueue[0], // Queue for transfers
                                                       pinnedBufferSrc[ns],
                                                       CL_TRUE,
                                                       CL_MAP_READ | CL_MAP_WRITE,
                                                       0,
                                                       memSize[sizeCount],
                                                       0, NULL,
                                                       NULL,
                                                       &err);
            if (err != SDK_SUCCESS) {
               printf("clEnqueueMapBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }

            pinnedBufferDst[ns] = clCreateBuffer(_deviceContext,
                                                 CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                                 memSize[sizeCount],
                                                 NULL,
                                                 &err);
            if (err != SDK_SUCCESS) {
               printf("clCreateBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }

            hostDst[ns] = (double*) clEnqueueMapBuffer(_commandQueue[0], // Queue for transfers
                                                       pinnedBufferDst[ns],
                                                       CL_TRUE,
                                                       CL_MAP_READ | CL_MAP_WRITE,
                                                       0,
                                                       memSize[sizeCount],
                                                       0, NULL,
                                                       NULL,
                                                       &err);
            if (err != SDK_SUCCESS) {
               printf("clEnqueueMapBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }

            // Create a buffer on the GPU for writing kernel input data in
#ifdef DEBUG
            printf("DEBUG: for devSrc ns=%d memSize[sizeCount]=%d\n",ns,memSize[sizeCount]);
#endif
            devSrc[ns] = clCreateBuffer(_deviceContext,
                                        CL_MEM_READ_ONLY,
                                        memSize[sizeCount],
                                        NULL,
                                        &err);
            if (err != SDK_SUCCESS) {
               printf("clCreateBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }

            // Create a buffer on the GPU for reading kernel output data from
            devDst[ns] = clCreateBuffer(_deviceContext,
                                        CL_MEM_WRITE_ONLY,
                                        memSize[sizeCount],
                                        NULL,
                                        &err);
            if (err != SDK_SUCCESS) {
               printf("clCreateBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
        }

        // Generate a random value, but different from the previous one
        do
        {
           value_old = value;
           value = (double) rand()/(double)RAND_MAX;
        }
        while (abs(value_old - value) > 1e-9);

        // Initialize arrays in host space with new values
        int nDim = (int) (memSize[sizeCount] / sizeof(double));
        cl_uint wkDim = 1;
        size_t gOffset = 0;
        size_t gWorkSize = nDim;

        for (int i=0; i<nDim; i++)
            validMem[i] = value;
        for (int ns=0; ns < NSTAGES; ns++)
            for (int i=0; i<nDim; i++)
            {
                hostSrc[ns][i] = value;
                hostDst[ns][i] = zero;
            }

        TimerReset();
        TimerStart();

        int buffNum = 0;

        cl_int numWaitEventWrite = 0;
        cl_event* eventWaitPtr = NULL;

        int writeStageIndex;
        int kernelStageIndex;
        int readStageIndex;

        cl_event lastWriteEvent[NSTAGES] = { 0 };
        cl_event lastKernelEvent[NSTAGES] = { 0 };
        cl_event lastReadEvent[NSTAGES] = { 0 };

        int nl = NITERS+2;
        while( nl-- )
        {
#ifdef DEBUG
            printf("\nDEBUG: ===> nl=%d buffNum=%d\n",nl,buffNum);
#endif
            // 0, 2, 1 to load pipeline correctly
            writeStageIndex =  (buffNum + 0) % NSTAGES;
            kernelStageIndex = (buffNum + 2) % NSTAGES;
            readStageIndex =   (buffNum + 1) % NSTAGES;
#ifdef DEBUG
            printf("DEBUG: writeStageIndex=%d kernelStageIndex=%d readStageIndex=%d\n",writeStageIndex,kernelStageIndex,readStageIndex);
#endif

            if(nl >= 2)
            {
                if(lastKernelEvent[writeStageIndex] != 0)
                {
                    numWaitEventWrite = 1;
                    eventWaitPtr = &lastKernelEvent[writeStageIndex];
                } else {
                    numWaitEventWrite = 0;
                    eventWaitPtr = NULL;
                }

#ifdef DEBUG
                printf("DEBUG: Before clEnqueueWriteBuffer writeStageIndex=%d\n",writeStageIndex);
#endif
                err = clEnqueueWriteBuffer(_commandQueue[0],                    // Transfer queue
                                           devSrc[writeStageIndex],
                                           CL_FALSE,
                                           0,
                                           memSize[sizeCount],
                                           hostSrc[writeStageIndex],
                                           numWaitEventWrite,
                                           eventWaitPtr,
                                           &lastWriteEvent[writeStageIndex]);
                if (err != SDK_SUCCESS) {
                   printf("clEnqueueWriteBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#ifdef DEBUG
                printf("DEBUG: After clEnqueueWriteBuffer\n");
#endif

                err = clFlush(_commandQueue[0]);                                // Transfer queue
                if (err != SDK_SUCCESS) {
                   printf("clFlush() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#ifndef OVERLAP
                param_value = CL_QUEUED;
                while(param_value != CL_COMPLETE)
                {
                    err = clGetEventInfo(lastWriteEvent[writeStageIndex],
                                         CL_EVENT_COMMAND_EXECUTION_STATUS,
                                         sizeof(cl_int),
                                         &param_value,
                                         &param_value_size_ret);
                }
                err = clReleaseEvent(lastWriteEvent[writeStageIndex]);
                if (err != SDK_SUCCESS) {
                   printf("clReleaseEvent() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#endif
            }

            if(lastWriteEvent[kernelStageIndex] != 0 && nl >= 1)
            {
                err  = clSetKernelArg(_Kernel, 0, sizeof(void *),  (void *) &devSrc[kernelStageIndex]);
                err |= clSetKernelArg(_Kernel, 1, sizeof(void *),  (void *) &devDst[kernelStageIndex]);
                err |= clSetKernelArg(_Kernel, 2, sizeof(cl_uint), (void *) &nDim);
                if (err != SDK_SUCCESS) {
                   printf("clSetKernelArg() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }

#ifdef DEBUG
                printf("DEBUG: Before clEnqueueNDRangeKernel kernelStageIndex=%d\n",kernelStageIndex);
#endif
                err = clEnqueueNDRangeKernel(_commandQueue[1],    // Kernel execution queue
                                             _Kernel,
                                             wkDim,
                                             &gOffset,
                                             &gWorkSize,
                                             NULL,
                                             1,
                                             &lastWriteEvent[kernelStageIndex],
                                             &lastKernelEvent[kernelStageIndex]);
                if (err != SDK_SUCCESS) {
                   printf("clEnqueueNDRangeKernel() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#ifdef DEBUG
                printf("DEBUG: After clEnqueueNDRangeKernel\n");
#endif

                err = clFlush(_commandQueue[1]);                                  // Kernel execution queue
                if (err != SDK_SUCCESS) {
                   printf("clFlush() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#ifndef OVERLAP
                param_value = CL_QUEUED;
                while(param_value != CL_COMPLETE)
                {
                    err = clGetEventInfo(lastKernelEvent[kernelStageIndex],
                                         CL_EVENT_COMMAND_EXECUTION_STATUS,
                                         sizeof(cl_int),
                                         &param_value,
                                         &param_value_size_ret);
                    if (err != SDK_SUCCESS) {
                       printf("clGetEventInfo() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                       printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                       exit(err);
                    }
                }
                err = clReleaseEvent(lastKernelEvent[kernelStageIndex]);
                if (err != SDK_SUCCESS) {
                   printf("clReleaseEvent() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#endif
            }

            if(lastKernelEvent[readStageIndex] != 0)
            {
#ifdef DEBUG
                printf("DEBUG: Before clEnqueueReadBuffer readStageIndex=%d\n",readStageIndex);
#endif
                err = clEnqueueReadBuffer(_commandQueue[QID],                   // Transfer queue
                                          devDst[readStageIndex],
                                          CL_FALSE,
                                          0,
                                          memSize[sizeCount],
                                          hostDst[readStageIndex],
                                          1,
                                          &lastKernelEvent[readStageIndex],
                                          &lastReadEvent[readStageIndex]);
                if (err != SDK_SUCCESS) {
                   printf("clEnqueueReadBuffer() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#ifdef DEBUG
                printf("DEBUG: After clEnqueueReadBuffer\n");
#endif

                err = clFlush(_commandQueue[QID]);                              // Transfer queue
                if (err != SDK_SUCCESS) {
                   printf("clFlush() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#ifndef OVERLAP
                param_value = CL_QUEUED;
                while(param_value != CL_COMPLETE)
                {
                    err = clGetEventInfo(lastReadEvent[readStageIndex],
                                         CL_EVENT_COMMAND_EXECUTION_STATUS,
                                         sizeof(cl_int),
                                         &param_value,
                                         &param_value_size_ret);
                    if (err != SDK_SUCCESS) {
                       printf("clGetEventInfo() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                       printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                       exit(err);
                    }
                }
                err = clReleaseEvent(lastReadEvent[readStageIndex]);
                if (err != SDK_SUCCESS) {
                   printf("clReleaseEvent() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
                   printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
                   exit(err);
                }
#endif
            }
            buffNum++;
        }
#ifdef DEBUG
        printf("\nDEBUG: The pipeline is empty\n");
#endif
        // Wait for dust to settle
        for(int i=0; i<MAX_Q; i++)
        {
            err = clFinish(_commandQueue[i]);
            if (err != SDK_SUCCESS) {
               printf("clFinish() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
        }
        TimerStop();
        double Time = GetElapsedTime();
        double rate = (double(2.*memSize[sizeCount]/MB) * double(NITERS)) / Time;
        if(memSize[sizeCount] >= MB)
        {
            printf("%d iteration(s) of %d MBytes moved forth and back with computation in %.1fs => %.0f MB/s\n",NITERS,(int) (memSize[sizeCount]/MB),Time,rate);
        } else {
            printf("%d iteration(s) of %d KBytes moved forth and back with computation in %.1fs => %.0f MB/s\n",NITERS,(int) (memSize[sizeCount]/KB),Time,rate);
        }

#ifdef CHECK
        //Check if the operations went OK
        err = SDK_SUCCESS;
        double check = value;
        for (int ic=0; ic < MAXCOMP; ic++)
             check = sin(check);

        for (int rStgIndex=0; rStgIndex < NSTAGES; rStgIndex++)
        {
            printf("Checking stage %d\n",rStgIndex);
            for (int i=0; i < nDim; i++)
            {
                if(abs(hostDst[rStgIndex][i] - check) > 1.e-9)
                {
                   err = i;
#ifdef DEBUG
                   printf("DEBUG: Stage:%d hostDst:%f check:%f\n",hostDst[rStgIndex][i],check);
#endif
                }
            }
        }

        if(err != SDK_SUCCESS)
          printf("Warning: the operations are not valid\n");
        else
          printf("SUCCESS: the operations are valid\n");

#ifdef DEBUG
        printf("DEBUG: Here-1\n");
#endif
#ifdef _WIN32
        _aligned_free(validMem);
#else
        free(validMem);
#endif
#endif
#ifdef DEBUG
        printf("DEBUG: Here-2\n");
#endif
        for (int ns=0; ns < NSTAGES; ns++)
        {
            err = clEnqueueUnmapMemObject(_commandQueue[0], pinnedBufferSrc[ns], hostSrc[ns], 0, NULL, NULL);
            if (err != SDK_SUCCESS) {
               printf("clEnqueueUnmapMemObject() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
            err = clEnqueueUnmapMemObject(_commandQueue[0], pinnedBufferDst[ns], hostDst[ns], 0, NULL, NULL);
            if (err != SDK_SUCCESS) {
               printf("clEnqueueUnmapMemObject() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
        }
        err = clFinish(_commandQueue[0]);
        if (err != SDK_SUCCESS) {
           printf("clFinish() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
           printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
           exit(err);
        }

        for (int ns=0; ns < NSTAGES; ns++)
        {
            err = clReleaseMemObject(pinnedBufferSrc[ns]);
            if (err != SDK_SUCCESS) {
               printf("clReleaseMemObject() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
            err = clReleaseMemObject(pinnedBufferDst[ns]);
            if (err != SDK_SUCCESS) {
               printf("clReleaseMemObject() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
            err = clReleaseMemObject(devSrc[ns]);
            if (err != SDK_SUCCESS) {
               printf("clReleaseMemObject() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
            err = clReleaseMemObject(devDst[ns]);
            if (err != SDK_SUCCESS) {
               printf("clReleaseMemObject() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
               printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
               exit(err);
            }
        }
    }
   
    printf("\nTest completed:\n---------------\n");
#ifdef DEBUG
    printf("DEBUG: Here-3\n");
#endif
    //Clean up 
    err = clReleaseKernel(_Kernel);
    if (err != SDK_SUCCESS) {
       printf("clReleaseKernel() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
       printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
       exit(err);
    }
    err = clReleaseProgram(_Program);
    if (err != SDK_SUCCESS) {
       printf("clReleaseProgram() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
       printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
       exit(err);
    }
    for(int i=MAX_Q-1; i>=0; i--)
    {
        err = clReleaseCommandQueue(_commandQueue[i]);
        if (err != SDK_SUCCESS) {
           printf("clReleaseCommandQueue() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
           printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
           exit(err);
        }
    }
    err = clReleaseContext(_deviceContext);
    if (err != SDK_SUCCESS) {
       printf("clReleaseContext() failed with %d (%s)\n",err,oclGetOpenCLErrorCodeStr(err));
       printf("Error at line %u in file %s !!!\n\n", __LINE__, __FILE__);
       exit(err);
    }
#ifdef DEBUG
    printf("DEBUG: Here-4\n");
#endif

    return 0;
}
