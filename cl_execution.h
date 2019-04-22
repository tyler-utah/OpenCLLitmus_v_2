#pragma once

#include <CL/cl.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <limits.h>
#include "functiondefs.h"
#include <string.h>

//From IWOCL tutorial (needs attribution)
#ifndef CL_DEVICE_BOARD_NAME_AMD
#define CL_DEVICE_BOARD_NAME_AMD 0x4038
#endif

#define check_ocl_error(e, f, l) if (e < 0) { \
  return_str << f << ":" << l << ": error (" << e << ")\n"; \
  ret_info = return_str.str(); \
  return -1; \
  }

#define check_ocl(err) check_ocl_error(err, __FILE__, __LINE__)

/*void check_ocl_error(const int e, const char *file, const int line) {
  if (e < 0) {
    printf("%s:%d: error (%d)\n", file, line, e);
    exit(1);
  }
}*/

class CL_Execution {
 public:
  cl_device_id exec_device;
  cl_context exec_context;
  cl_command_queue exec_queue;
  cl_program exec_program;
  std::map<std::string, cl_kernel> exec_kernels;
  cl_platform_id exec_platform;

public:
  
  //From IWOCL tutorial (needs attribution)
  static std::string getDeviceName(const cl_device_id& device, int &err) {
    CLW_CLASS_WRAPPER;
    char name[256];
    cl_device_info info = CL_DEVICE_NAME;
    
    // Special case for AMD
    err = CLWGetDeviceInfo(device, info, 256, name, NULL);
    if (err < 0){
      std::stringstream error_str;
      error_str << "[Error:getDeviceName] " << err;
      return error_str.str();
    }

    std::string ret(name);
#ifdef CL_DEVICE_BOARD_NAME_AMD
    if (strstr(ret.c_str(), "Advanced Micro Devices"))
      info = CL_DEVICE_BOARD_NAME_AMD;
#endif
    return ret;
  }

  static std::string getDriverVersion(const cl_device_id& device, int &err) {
    CLW_CLASS_WRAPPER;
    char version[256];
    cl_device_info info = CL_DRIVER_VERSION;
    err = CLWGetDeviceInfo(device, info, 256, version, NULL);
    if (err < 0) {
      return "";
    }
    return std::string(version);
  }
  
  std::string getExecDeviceName(int &err) {
    return getDeviceName(exec_device, err);
  }

  std::string getExecDriverVersion(int &err) {
    return getDriverVersion(exec_device, err);
  }
  
  int get_SMs() {
    CLW_CLASS_WRAPPER;
    cl_uint ret;
    cl_device_info info = CL_DEVICE_MAX_COMPUTE_UNITS;
    
    CLWGetDeviceInfo(exec_device, info, sizeof(cl_uint), &ret, NULL);
    return ret;
  }
  
  bool is_Nvidia(int &err) {
    CLW_CLASS_WRAPPER;
    char buffer[256];   
    cl_device_info info = CL_DEVICE_VENDOR;
    err = CLWGetDeviceInfo(exec_device, info, 256, buffer, NULL);
    if (err < 0) {
      return false;
    }
    std::string to_search = buffer;
    if (to_search.find("NVIDIA Corporation") == std::string::npos) {
      return false;
    }
    return true;
  }

  bool is_Intel(int &err) {
    CLW_CLASS_WRAPPER;
	  char buffer[256];
	  cl_device_info info = CL_DEVICE_VENDOR;
	  err = CLWGetDeviceInfo(exec_device, info, 256, buffer, NULL);
    if (err < 0)
      return false;
    std::string to_search = buffer;
	  if (to_search.find("Intel") == std::string::npos) {
		  return false;
	  }
	  return true;
  }

  bool is_AMD(int &err) {
    CLW_CLASS_WRAPPER;
	  char buffer[256];
	  cl_device_info info = CL_DEVICE_VENDOR;
	  err = CLWGetDeviceInfo(exec_device, info, 256, buffer, NULL);
    if (err < 0)
      return false;
    std::string to_search = buffer;
	  if (to_search.find("Advanced Micro Devices") == std::string::npos) {
		  return false;
	  }
	  return true;
  }

  bool is_ARM(int &err) {
    CLW_CLASS_WRAPPER;
    char buffer[256];
    cl_device_info info = CL_DEVICE_VENDOR;

    err = CLWGetDeviceInfo(exec_device, info, 256, buffer, NULL);
    if (err < 0)
      return false;
    std::string to_search = buffer;
    if (to_search.find("ARM") == std::string::npos) {
      return false;
    }
    return true;
  }
  
  bool is_ocl2(int &err) {
    CLW_CLASS_WRAPPER;
    char buffer[256];   
    cl_device_info info = CL_DEVICE_VERSION;
    err = CLWGetDeviceInfo(exec_device, info, 256, buffer, NULL);
    if (err < 0)
      return false;
    std::string to_search(buffer);
    if (to_search.find("OpenCL 2.") == std::string::npos) {
      return false;
    }
    return true;
  }
  
  std::string get_vendor_option(int &err) {
    if (is_Nvidia(err)) {
      return " -DNVIDIA ";
    }
    if (is_Intel(err)) {
      return " -DINTEL ";
    }
    if (is_AMD(err)) {
      return " -DAMD ";
    }
    if (is_ARM(err)) {
      return " -DARM ";
    }
    return "";
  }
  
  std::string check_ocl2x(int &err) {
    if (is_ocl2(err)) {
      return " -cl-std=CL2.0 ";
    }
    return "";
  }
  
 
  
  
  //roughly from IWOCL tutorial (needs attribution)
int compile_kernel(std::string source, const char * kernel_include, std::string kernel_defs, int &err, std::string &ret) {

    CLW_CLASS_WRAPPER;

    //exec_program = cl::Program(cl::Context(exec_context), loadProgram(kernel_file), ret);
    //std::string  source = (loadProgram(kernel_file, &len));
    const char * source_c_str = source.c_str();
    size_t len = source.size();


    exec_program = CLWCreateProgramWithSource(exec_context, 1, (const char **)& source_c_str, &len, &err);
    if (err < 0) {
      return err;
    }

    std::stringstream options;
    std::stringstream to_ret;
    options.setf(std::ios::fixed, std::ios::floatfield);
    
    //set compiler options here, example below 
    //options << " -cl-fast-relaxed-math";
    
    //Include the rt_device sources
    //options << "-I" << kernel_include << " ";

    options <<  kernel_defs;

        
    //Check to see if we're OpenCL 2.0
    options << check_ocl2x(err);

	  options << get_vendor_option(err);
    
    to_ret << "FLAGS: " << options.str() << std::endl;


    //build the program
    //ret = exec_program.build(options.str().c_str());
    err = CLWBuildProgram(exec_program, 1, &exec_device, options.str().c_str(), NULL, NULL);
    //check_ocl(ret);
    
    if (err != CL_SUCCESS) {
      char buffer[2048];
      cl_program_build_info b_info = CL_PROGRAM_BUILD_LOG;
      CLWGetProgramBuildInfo(exec_program, exec_device, b_info, 2048, buffer, NULL);
      std::string log(buffer);
      to_ret << log << std::endl;
      ret = to_ret.str();
      //if(false) dump_program_binary(exec_program);
    }

    ret = to_ret.str();
    return err;        
  }

  bool intel_subgroups(int &err) {
    CLW_CLASS_WRAPPER;
    char buffer[256];
    cl_device_info info = CL_DEVICE_EXTENSIONS;
    err = CLWGetDeviceInfo(exec_device, info, 256, buffer, NULL);
    if (err < 0)
      return false;
    //err = exec_device.getInfo(info, &buffer);
    std::string to_search(buffer);
    if (to_search.find("cl_intel_subgroups") == std::string::npos) {
      return false;
    }
    return true;
  }

  int get_compute_units(const char* kernel_name) {
    return get_SMs();
  }

};
