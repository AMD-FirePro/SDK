#ifndef OVERLAP_H_
#define OVERLAP_H_

#include <CL/cl.h>
#include <CL/opencl.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <malloc.h>
#include <sys/time.h>
#endif

#ifdef _WIN32
typedef __int64 i64;
#else
typedef long long i64;
extern int clock_gettime( clockid_t clock_id, struct timespec * tp );
#endif

#define SDK_SUCCESS 0
#define SDK_FAILURE 1

#define NSIZES 15
#define NITERS 100
#define NSTAGES 3   // write data, kernel execution and read data
#define MAX_Q 3     // nbr of execution queues (has to be 2 or 3)
#define MAXCOMP 15  // computation load x=sin(sin(sin(...sin(x))))))

#if (MAX_Q == 3)
# define QID 2
#else
# define QID 0
#endif

#endif  /* #ifndef OVERLAP_H_ */
