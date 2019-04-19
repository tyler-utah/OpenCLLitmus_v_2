
#include "stdio.h"
#include <vector>
#include <iostream>
#ifdef _WIN32
#include "getopt_win.h"
#else
#include "getopt.h"
#endif
#include "cl_execution.h"
#include <chrono>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include "functiondefs.h"

#define SIZE 1024
#define NANOSEC 1000000000LL

std::string INPUT_FILE;
std::string kernel_include = "C:\\Users\\Tyler\\Documents\\GPUMemTesting2\\OpenCLLitmus\\tests";
//std::string kernel_include = "/home/quanto/IW/OpenCLLitmus/tests"; 
int LIST = 0;
int PLATFORM_ID = 0;
int DEVICE_ID = 0;
int QUIET = 0;
int ITERATIONS = 1000;
int USE_CHIP_CONFIG = 1;

struct TestConfig
{
  int hist_size;
  std::vector<std::string> hist_strings;
  int output_size;
};

struct ChipConfig
{
  int max_local_size;
  int min_local_size;
  int occupancy_est;
  int warp_size;
};

std::map<std::string, ChipConfig> ChipConfigMaps;

void populate_ChipConfigMaps()
{
  ChipConfig defaultChipConfig = { 256, 128, 32, 1 };
  ChipConfig Nvidia960M = { 1024, 256, 20, 32 };
  ChipConfig Nvidia940M = { 1024, 1, 16, 32 };
  ChipConfig IntelHD5500 = { 256, 1, 16, 8 };
  ChipConfig Inteli75600u = { 4, 1, 4, 1 };
  ChipConfig TeslaK20m = { 1024, 512, 32, 32 };
  ChipConfig TeslaK40c = { 1024, 512, 32, 32 };

  ChipConfigMaps["default"] = defaultChipConfig;
  ChipConfigMaps["Intel(R) Core(TM) i7-5600U CPU @ 2.60GHz"] = Inteli75600u;
  ChipConfigMaps["Intel(R) HD Graphics 5500"] = IntelHD5500;
  ChipConfigMaps["Tesla K20m"] = TeslaK20m;
  ChipConfigMaps["GeForce 940M"] = Nvidia940M;
  ChipConfigMaps["GeForce GTX 960M"] = Nvidia960M;
  ChipConfigMaps["Tesla K40c"] = TeslaK40c;
}

//From IWOCL tutorial (needs attribution)
cl_platform_id *platforms;
unsigned getDeviceList(std::vector<std::vector<cl_device_id>> &devices, int &err) {
  CLW_CLASS_WRAPPER;
  // Get list of platforms
  cl_uint num_plats = 0;
  err = CLWGetPlatformIDs(0, NULL, &num_plats);
  platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id)*num_plats);
  CLWGetPlatformIDs(num_plats, platforms, NULL);
  if (err < 0) {
    return err;
  }

  // Enumerate devices
  for (unsigned int i = 0; i < num_plats; i++) {
    //std::vector<cl::Device> plat_devices;
    cl_uint num_devices = 0;
    err = CLWGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    if (err < 0) {
      return err;
    }
    cl_device_id * plat_devices = (cl_device_id *)malloc(sizeof(cl_device_id)*num_devices);
    CLWGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, plat_devices, NULL);


    //platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &plat_devices);
    std::vector<cl_device_id> to_push;
    for (unsigned int j = 0; j < num_devices; j++) {
      to_push.push_back(plat_devices[j]);
    }
    devices.push_back(to_push);
    //devices.insert(devices.end(), plat_devices.begin(), plat_devices.end());
  }
  return devices.size();
}

#if !defined(ANDROID)
//From IWOCL tutorial (needs attribution)
void list_devices(int &err)
{
  std::vector<std::vector<cl_device_id>> devices;
  getDeviceList(devices, err);

  // Print device names
  if (devices.size() == 0)
  {
    std::cout << "No devices found." << std::endl;
  }
  else
  {
    std::cout << std::endl;
    std::cout << "Platform,Devices:" << std::endl;
    for (unsigned j = 0; j < devices.size(); j++)
    {
      for (unsigned i = 0; i < devices[j].size(); i++)
      {
        std::cout << j << ", " << i << ": " << CL_Execution::getDeviceName(devices[j][i], err) << std::endl;
      }
    }
  }
}

void usage(int argc, char *argv[])
{
  fprintf(stderr, "usage: %s [-l] [-q] [-p platform_id] [-d device_id] [-o output-file] litmus-test-directory\n", argv[0]);
}

std::string parse_args(int argc, char *argv[])
{
  int c;
  const char *skel_opts = "p:d:D:i:clqo:";
  char *opts;
  int len = 0;
  char *end;
  // LEFT OFF HERE, FIGURE OUT T
  std::stringstream kernel_defs;

  len = strlen(skel_opts) + 1;
  opts = (char *)calloc(len, sizeof(char));
  if (!opts)
  {
    fprintf(stderr, "Unable to allocate memory\n");
    exit(EXIT_FAILURE);
  }

  strcat(opts, skel_opts);

  while ((c = getopt(argc, argv, opts)) != -1)
  {
    switch (c)
    {
    case 'D':
      kernel_defs << "-D " << optarg << " ";
      break;
    case 'q':
      QUIET = 1;
      break;
    case 'c':
      USE_CHIP_CONFIG = 1;
      break;
    case 'i':
      errno = 0;
      ITERATIONS = strtol(optarg, &end, 10);
      if (errno != 0 || *end != '\0')
      {
        fprintf(stderr, "Invalid iterations '%s'. An integer must be specified.\n", optarg);
        exit(EXIT_FAILURE);
      }
      break;
    case 'l':
      LIST = 1; //TODO: copy?
      break;
    case 'p':
      errno = 0;
      PLATFORM_ID = strtol(optarg, &end, 10);
      if (errno != 0 || *end != '\0')
      {
        fprintf(stderr, "Invalid platform id device '%s'. An integer must be specified.\n", optarg);
        exit(EXIT_FAILURE);
      }
      break;
    case 'd':
      errno = 0;
      DEVICE_ID = strtol(optarg, &end, 10);
      if (errno != 0 || *end != '\0')
      {
        fprintf(stderr, "Invalid device id device '%s'. An integer must be specified.\n", optarg);
        exit(EXIT_FAILURE);
      }
      break;
    case '?':
      usage(argc, argv);
      exit(EXIT_FAILURE);
      break;
    }
  }

  if (LIST != 1)
  {
    if (optind < argc)
    {
      INPUT_FILE = argv[optind];
    }
    else
    {
      usage(argc, argv);
      exit(EXIT_FAILURE);
    }
  }
  free(opts);
  return kernel_defs.str();
}
#endif

cl_int get_kernels(CL_Execution &exec, int &err)
{
  CLW_CLASS_WRAPPER;


  exec.exec_kernels["litmus_test"] = CLWCreateKernel(exec.exec_program, "litmus_test", &err);

  //Consider doing this for robustness
  //exec.check_kernel_wg_sizes(exec.exec_kernels["bfs_init"], "bfs_init", TB_SIZE);

  exec.exec_kernels["check_outputs"] = CLWCreateKernel(exec.exec_program, "check_outputs", &err);

  //Consider doing this for robustness
  //exec.check_kernel_wg_sizes(exec.exec_kernels["bfs_kernel"], "bfs_kernel", TB_SIZE);
  return err;
}

TestConfig parse_config(const std::string &config_str) {
  std::stringstream config_stream(config_str);
  TestConfig ret;
  std::string title, ignore;

  std::getline(config_stream, title);

  std::getline(config_stream, ignore);
  std::getline(config_stream, ignore);
  ret.hist_size = std::stoi(ignore);

  //infile >> ret.hist_size;
  for (int i = 0; i < ret.hist_size; i++) {
    std::string out_desc;
    std::getline(config_stream, out_desc);
    ret.hist_strings.push_back(out_desc);
  }
  ret.hist_strings.push_back("errors: ");
  ret.hist_size++;
  std::getline(config_stream, ignore); // "num outputs"
  std::getline(config_stream, ignore);
  ret.output_size = std::stoi(ignore);
  return ret;
}

// test is kernel string with the testing_common.h as part of the string
int run_test(std::string test, std::string test_config, int iterations, int platform_id, int device_id, std::string options, std::string &ret_info) {

  CLW_CLASS_WRAPPER;
  CL_Execution exec;
  std::stringstream return_str;
  ChipConfig cConfig;
  int err = 0;

  populate_ChipConfigMaps();
  std::vector<std::vector<cl_device_id>> devices;
  getDeviceList(devices, err);
  check_ocl(err);


  if (platform_id >= devices.size())
  {
    return_str << "ERROR: Invalid platform id (" << platform_id << ") \n";
    ret_info = return_str.str();
    return -1;
  }

  if (device_id >= devices[platform_id].size())
  {
    return_str << "ERROR: Invalid device id (" << device_id << ") \n";
    ret_info = return_str.str();
    return -1;
  }

  populate_ChipConfigMaps();
  exec.exec_device = devices[PLATFORM_ID][DEVICE_ID];
  exec.exec_platform = platforms[PLATFORM_ID];

  return_str << "Using Device: " << exec.getExecDeviceName(err) << "\n";
  check_ocl(err);

  return_str << "Driver Version: " << exec.getExecDriverVersion(err) << "\n";
  check_ocl(err);


  if (ChipConfigMaps.find(exec.getExecDeviceName(err).c_str()) == ChipConfigMaps.end())
  {
    return_str << "using chip config: default\n";
    cConfig = ChipConfigMaps["default"];
  }
  else
  {
    return_str << "using chip config: " << exec.getExecDeviceName(err) << "\n";
    cConfig = ChipConfigMaps[exec.getExecDeviceName(err).c_str()];
    check_ocl(err);

  }

  cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)exec.exec_platform, 0 };
  exec.exec_context = CLWCreateContext(props, 1, &(exec.exec_device), NULL, NULL, &err);
  check_ocl(err);
  //cl::CommandQueue queue(cl::Context(exec.exec_context), cl::Device(exec.exec_device));
  exec.exec_queue = CLWCreateCommandQueue(exec.exec_context, exec.exec_device, NULL, &err);
  check_ocl(err);

  std::string tmp_str;
  err = exec.compile_kernel(test.c_str(), "", options, err, tmp_str);
  return_str << tmp_str;
  check_ocl(err);


  int occupancy = cConfig.occupancy_est;
  int max_local_size = cConfig.max_local_size;

  int max_global_size = max_local_size * occupancy;
  err = get_kernels(exec, err);
  check_ocl(err);

  TestConfig cfg = parse_config(test_config);

  // Actual real stuff starts

  cl_mem dga = CLWCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (SIZE), NULL, &err);
  check_ocl(err);
  cl_mem dgna = CLWCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (SIZE), NULL, &err);
  check_ocl(err);
  cl_mem doutput = CLWCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * ((cfg.output_size)), NULL, &err);
  check_ocl(err);
  cl_mem dresult = CLWCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (1), NULL, &err);
  check_ocl(err);
  cl_mem dshuffled_ids = CLWCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (max_global_size), NULL, &err);
  check_ocl(err);
  cl_mem dscratchpad = CLWCreateBuffer(exec.exec_context, CL_MEM_READ_WRITE, sizeof(cl_int) * (512), NULL, &err);
  check_ocl(err);
  cl_int dwarp_size = cConfig.warp_size;

  cl_int hga[SIZE], hgna[SIZE], houtput[SIZE];
  cl_int hresult;
  cl_int *hshuffled_ids = (cl_int *)malloc(sizeof(cl_int) * max_global_size);

  for (int i = 0; i < max_global_size; i++)
  {
    hshuffled_ids[i] = i;
  }

  for (int i = 0; i < SIZE; i++)
  {
    hga[i] = hgna[i] = houtput[i] = 0;
  }

  check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 0, sizeof(cl_mem), &dga));
  check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 1, sizeof(cl_mem), &dgna));
  check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 2, sizeof(cl_mem), &doutput));
  check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 3, sizeof(cl_mem), &dshuffled_ids));
  check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 4, sizeof(cl_mem), &dscratchpad));
  check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 7, sizeof(cl_int), &dwarp_size));

  check_ocl(CLWSetKernelArg(exec.exec_kernels["check_outputs"], 0, sizeof(cl_mem), &doutput));
  check_ocl(CLWSetKernelArg(exec.exec_kernels["check_outputs"], 1, sizeof(cl_mem), &dresult));

  auto now = std::chrono::high_resolution_clock::now();
  unsigned long long begin_time, end_time, time;
  begin_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
  int *shuffled_wg_order = (int *)malloc(occupancy * sizeof(int));
  int *temp_id_ordering = (int *)malloc(max_global_size*sizeof(int));
  int *shuffled_warp_order = (int *)malloc((max_local_size / dwarp_size) * sizeof(int));


  cl_int dist = 128;
  cl_int location = 64;
  cl_int x_y_distance = dist;

  cl_int scratch_location = location;
  cl_int offset = 3;

  int *histogram = (int *)malloc(sizeof(int) * cfg.hist_size);
  for (int i = 0; i < cfg.hist_size; i++)
  {
    histogram[i] = 0;
  }

  for (int i = 0; i < ITERATIONS; i++)
  {

    check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 5, sizeof(cl_int), &scratch_location));
    cl_int to_pass = x_y_distance + offset;
    check_ocl(CLWSetKernelArg(exec.exec_kernels["litmus_test"], 6, sizeof(cl_int), &to_pass));

    // set up ids
    // mod by zero, if max local size is same and min
    int local_size = (rand() % (max_local_size - cConfig.min_local_size)) + cConfig.min_local_size;
    int global_size = occupancy * local_size;
    int wg_count = global_size / local_size;
    int warp_size = dwarp_size; // add to chip config before amd testing
    int warps_per_wg = local_size / warp_size;
    int remainder_threads = local_size % warp_size;

    // id shuffle
    for (int j = 0; j < global_size; j++)
    {
      hshuffled_ids[j] = i;
    }
    for (int i = 0; i < wg_count; i++)
    {
      shuffled_wg_order[i] = i;
    }
    std::random_shuffle(shuffled_wg_order, &shuffled_wg_order[wg_count]); // random_shuffle not end inclusive

    for (int i = 0; i < global_size; i++)
    {
      temp_id_ordering[i] = i;
    }

    for (int i = 0; i < wg_count; i++)
    {
      for (int j = 0; j < warps_per_wg; j++)
      {
        std::random_shuffle(&temp_id_ordering[i * local_size + j * warp_size], &temp_id_ordering[i * local_size + (j + 1) * warp_size]);
      }
    }
    for (int i = 0; i < wg_count; i++)
    {

      for (int x = 0; x < warps_per_wg; x++)
      {
        shuffled_warp_order[x] = x;
      }
      std::random_shuffle(shuffled_warp_order, &shuffled_warp_order[warps_per_wg]);

      for (int j = 0; j < warps_per_wg; j++)
      {
        for (int k = 0; k < warp_size; k++)
        {
          hshuffled_ids[i * local_size + j * warp_size + k] = temp_id_ordering[shuffled_wg_order[i] * local_size + shuffled_warp_order[j] * warp_size + k];
        }
      }

      if (remainder_threads != 0)
      {
        for (int y = 0; y < remainder_threads; y++)
        {
          hshuffled_ids[i * local_size + (warps_per_wg + 1) * warp_size + y] = temp_id_ordering[shuffled_wg_order[i] * local_size + (warps_per_wg + 1) * warp_size + y];
        }
      }
    }
    err = CLWEnqueueWriteBuffer(exec.exec_queue, dshuffled_ids, CL_TRUE, 0, sizeof(cl_int) * (global_size), hshuffled_ids, 0, NULL, NULL);
    check_ocl(err);

    err = CLWEnqueueWriteBuffer(exec.exec_queue, dga, CL_TRUE, 0, sizeof(cl_int) * (SIZE), hga, 0, NULL, NULL);
    check_ocl(err);

    err = CLWEnqueueWriteBuffer(exec.exec_queue, dgna, CL_TRUE, 0, sizeof(cl_int) * (SIZE), hgna, 0, NULL, NULL);
    check_ocl(err);

    err = CLWEnqueueWriteBuffer(exec.exec_queue, doutput, CL_TRUE, 0, sizeof(cl_int) * (cfg.output_size), houtput, 0, NULL, NULL);
    check_ocl(err);

    err = CLWFinish(exec.exec_queue);
    check_ocl(err);

    const size_t global_size_t = global_size;
    const size_t local_size_t = local_size;

    err = CLWEnqueueNDRangeKernel(exec.exec_queue, exec.exec_kernels["litmus_test"], 1, NULL, &global_size_t, &local_size_t, 0, NULL, NULL);
    check_ocl(err);

    err = CLWFinish(exec.exec_queue);

    check_ocl(err);

    const size_t global_size_t2 = 1;
    const size_t local_size_t2 = 1;
    err = CLWEnqueueNDRangeKernel(exec.exec_queue, exec.exec_kernels["check_outputs"], 1, NULL, &global_size_t2, &local_size_t2, 0, NULL, NULL);
    check_ocl(err);

    err = CLWEnqueueReadBuffer(exec.exec_queue, dresult, CL_TRUE, 0, sizeof(cl_int) * 1, &hresult, 0, NULL, NULL);
    check_ocl(err);

    histogram[hresult]++;
  }

  now = std::chrono::high_resolution_clock::now();
  end_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
  time = end_time - begin_time;
  float time_float = static_cast<float>(time) / static_cast<float>(NANOSEC);

  return_str << std::endl << "RESULTS: " << std::endl;
  return_str << "-------------------" << std::endl;
  for (int i = 0; i < cfg.hist_size; i++) {
    return_str << cfg.hist_strings[i] << histogram[i] << std::endl;

  }
  return_str << std::endl;
  return_str << "RATES" << std::endl;
  return_str << "-------------------" << std::endl;
  return_str << "tests          : " << ITERATIONS << std::endl;
  return_str << "time (seconds) : " << time_float << std::endl;
  return_str << "tests/sec      : " << static_cast<float>(ITERATIONS) / time_float << std::endl;
  return_str << std::endl;


  free(shuffled_wg_order);
  free(temp_id_ordering);
  free(shuffled_warp_order);
  free(platforms);
  ret_info = return_str.str();
  return 1;
}

//From the IWOCL tutorial (needs attribution)
std::string loadFile(const char* input, size_t *len) {
  std::ifstream stream(input);
  if (!stream.is_open()) {
    std::cout << "Cannot open file: " << input << std::endl;
#if defined(_WIN32) && !defined(__MINGW32__)
    system("pause");
#endif
    exit(1);
  }

  std::string ret = std::string(
    std::istreambuf_iterator<char>(stream),
    (std::istreambuf_iterator<char>()));
  *len = ret.size();
  return ret;
}

#if !defined(ANDROID)
int main(int argc, char *argv[])
{

  int err = 0;
  std::string opts;

  if (argc == 1)
  {
    usage(argc, argv);
    exit(1);
  }

  opts = parse_args(argc, argv);

  if (LIST == 1)
  {
    list_devices(err);
    exit(0);
  }

  // TYLER TODO: create strings out of these files
  std::string kernel_file = INPUT_FILE + "/kernel.cl";
  std::string test_config_file = INPUT_FILE + "/config.txt";

  std::string to_print;
  size_t f_len;
  std::string tconfig = loadFile(test_config_file.c_str(), &f_len);
  std::string src0 = loadFile(kernel_file.c_str(), &f_len);
  std::string t_common = kernel_include + "\\testing_common.h";
  std::string src1 = loadFile(t_common.c_str(), &f_len);
  std::string nvidia_atomics = kernel_include + "\\nvidia_atomics.h";
  std::string src2 = loadFile(nvidia_atomics.c_str(), &f_len);
  //run_test("", "", 1, 0, 0, "", to_print);
  std::string final_source = src2 + src1 + src0;

  //std::cout << final_source << std::endl;

  run_test(final_source, tconfig, ITERATIONS, PLATFORM_ID, DEVICE_ID, opts, to_print);
  std::cout << to_print;
  return 1;
}
#endif