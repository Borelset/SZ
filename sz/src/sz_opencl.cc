/* no opencl

#include "sz_opencl.h"
#include "sz.h"
#include <algorithm>
#include <iterator>
#include <numeric>
#include <random>
#include <utility>
#include <vector>


#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

struct sz_opencl_state
{
  struct error_t
  {
    cl_int code = 0;
    const char* str = nullptr;
  } error;
  cl::Platform platform;
  cl::Device device;
  cl::Context context;
  cl::CommandQueue queue;
};

extern "C"
{
  int sz_opencl_init(struct sz_opencl_state** state)
  {
    try {
      *state = new sz_opencl_state;

      std::vector<cl::Platform> platforms;
      cl::Platform::get(&platforms);

      auto valid_platform =
        std::find_if(std::begin(platforms), std::end(platforms),
                     [state](cl::Platform const& platform) {
                       try {
                         std::vector<cl::Device> devices;
                         platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
                         (*state)->device = devices.front();
                         (*state)->platform = platform;
                         return true;
                       } catch (cl::Error const& error) {
                         if (error.err() != CL_DEVICE_NOT_FOUND)
                           throw;
                       }
                       return false;
                     });
      if (valid_platform == std::end(platforms))
        throw cl::Error(CL_DEVICE_NOT_FOUND, "Failed to find a GPU");

      (*state)->context = cl::Context({ (*state)->device });
      (*state)->queue = cl::CommandQueue((*state)->context, (*state)->device);

      return SZ_SCES;
    } catch (cl::Error const& cl_error) {
      (*state)->error.code = cl_error.err();
      (*state)->error.str = cl_error.what();
      return SZ_NSCS;

    } catch (...) {
      delete *state;
      *state = nullptr;
      return SZ_NSCS;
    }
  }

  int sz_opencl_release(struct sz_opencl_state** state)
  {
    delete *state;

    return SZ_SCES;
  }

  const char* sz_opencl_error_msg(struct sz_opencl_state* state)
  {
    if (state == nullptr) {
      return "sz opencl allocation failed";
    }

    return state->error.str;
  }

  int sz_opencl_error_code(struct sz_opencl_state* state)
  {
    if (state == nullptr) {
      return -1;
    }

    return state->error.code;
  }

  int sz_opencl_check(struct sz_opencl_state* state)
  {
    try {
      std::string vec_add(
        R"(
				kernel void add(__global float* a, __global float* b, __global float* c)
				{
					int id = get_global_id(0);
					c[id] = a[id] + b[id];
				}
				)");
      cl::Program::Sources sources(
        1, std::make_pair(vec_add.c_str(), vec_add.size() + 1));

      cl::Program program(state->context, sources);
      program.build({ state->device });
      cl::Kernel kernel(program, "add");
      const int size = 1024;
      std::vector<float> h_a(size);
      std::vector<float> h_b(size);
      std::vector<float> h_c(size);
      std::vector<float> verify(size);
      cl::Buffer d_a(state->context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                     sizeof(cl_float) * size);
      cl::Buffer d_b(state->context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                     sizeof(cl_float) * size);
      cl::Buffer d_c(state->context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                     sizeof(cl_float) * size);

      auto random_fill = [](std::vector<float>& vec, int seed) {
        std::seed_seq seed_seq{ seed };
        std::mt19937 gen(seed_seq);
        std::uniform_real_distribution<float> dist;

        std::generate(std::begin(vec), std::end(vec),
                      [&dist, &gen]() { return dist(gen); });
      };
      random_fill(h_a, 0);
      random_fill(h_b, 1);
      random_fill(h_c, 2);
      std::transform(std::begin(h_a), std::end(h_a), std::begin(h_b),
                     std::begin(verify),
                     [](float a, float b) { return a + b; });

      kernel.setArg(0, d_a);
      kernel.setArg(1, d_b);
      kernel.setArg(2, d_c);
*/
//      state->queue.enqueueWriteBuffer(d_a, /*blocking*/ CL_TRUE, /*offset*/ 0,
//                                      /*size*/ sizeof(cl_float) * size,
//                                      h_a.data());
//      state->queue.enqueueWriteBuffer(d_b, /*blocking*/ CL_TRUE, /*offset*/ 0,
//                                      /*size*/ sizeof(cl_float) * size,
//                                      h_b.data());
//      state->queue.enqueueNDRangeKernel(kernel, /*offset*/ cl::NullRange,
//                                        /*global*/ cl::NDRange(size),
//                                        cl::NullRange);
//      state->queue.finish();
//      state->queue.enqueueReadBuffer(d_c, /*blocking*/ CL_TRUE, /*offset*/ 0,
//                                     /*size*/ sizeof(cl_float) * size,
 //                                    h_c.data());
/*
      if (std::equal(std::begin(h_c), std::end(h_c), std::begin(verify),
                     std::end(verify))) {
        return SZ_SCES;
      } else {
        return SZ_NSCS;
      }
    } catch (cl::Error const& error) {
      state->error.code = error.err();
      state->error.str = error.what();
      return SZ_NSCS;
    }
  }

unsigned char * sz_compress_float3d_opencl(float *oriData, size_t r1, size_t r2, size_t r3, double realPrecision, size_t * comp_size){
  return SZ_compress_float_3D_MDQ_decompression_random_access_with_blocked_regression(oriData, r1, r2, r3, realPrecision, comp_size);
}

}
*/