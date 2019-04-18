#pragma once

#ifdef ANDROID
#define CLW_CLASS_WRAPPER DroidCL droidCL;
#define CLWGetPlatformIDs droidCL.clGetplatformIDs
#define CLWGetDeviceIDs droidCL.clGetDeviceIDs
#define CLWGetDeviceInfo droidCL.clGetDeviceInfo
#define CLWCreateProgramWithSource droidCL.clCreateProgramWithSource
#define CLWBuildProgram droidCL.clBuildProgram
#define CLWGetProgramBuildInfo droidCL.clGetProgramBuildInfo
#define CLWCreateCommandQueue droidCL.clCreateCommandQueue
#define CLWCreateKernel droidCL.clCreateKernel
#define CLWCreateBuffer droidCL.clCreateBuffer
#define CLWSetKernelArg droidCL.clSetKernelArg
#define CLWEnqueueWriteBuffer droidCL.clEnqueueWriteBuffer
#define CLWFinish droidCL.clFinish
#define CLWEnqueueNDRangeKernel droidCL.clEnqueueNDRangeKernel
#define CLWEnqueueReadBuffer droidCL.clEnqueueReadBuffer


#else
#define CLW_CLASS_WRAPPER  ;
#define CLWGetPlatformIDs  clGetPlatformIDs
#define CLWGetDeviceIDs clGetDeviceIDs
#define CLWGetDeviceInfo clGetDeviceInfo
#define CLWCreateProgramWithSource clCreateProgramWithSource
#define CLWBuildProgram clBuildProgram
#define CLWGetProgramBuildInfo clGetProgramBuildInfo
#define CLWCreateCommandQueue clCreateCommandQueue
#define CLWCreateKernel clCreateKernel
#define CLWCreateBuffer clCreateBuffer
#define CLWSetKernelArg clSetKernelArg
#define CLWEnqueueWriteBuffer clEnqueueWriteBuffer
#define CLWFinish clFinish
#define CLWEnqueueNDRangeKernel clEnqueueNDRangeKernel
#define CLWEnqueueReadBuffer clEnqueueReadBuffer


#endif 