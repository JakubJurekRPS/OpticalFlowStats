#include <cuda_runtime.h>
#include "helper_cuda.h"
#include "stats_definitions.hpp"
#include <cmath>
#include <chrono>

#define WARP_SIZE 32

// TODO: check possibiities for kernels optimisation
// TODO: check if it is possible to avoid bank conflicts
namespace ofs_detail
{

namespace
{ 
  using cv::cuda::PtrStepSz;
  // using cv::cuda::GpuMat;

__host__ __device__  int nearestGreaterPow2(int n)
{
  --n;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return ++n;
} 

__device__ float calc_angle(const float x, const float y)
{
    float angle = atan2f(y, x);
    if (angle < 0)
        angle += 2*CV_PI;
    return angle;
  }

  
__global__ void k_calc_and_part_reduce_HoA(const PtrStepSz<float2> flow, unsigned* hist, const unsigned binsNum)
{
  const int i = blockIdx.y * blockDim.y + threadIdx.y;
  const int j = blockIdx.x * blockDim.x + threadIdx.x;
  extern __shared__ unsigned hist_shared[];

  int numOfcells = binsNum * blockDim.y;
  int numOfThreads = blockDim.y * blockDim.x;
  int numOfIters = numOfcells / numOfThreads;
  int thID = threadIdx.y * blockDim.x + threadIdx.x;

  auto boundaryCheck = (i < flow.rows && j < flow.cols);
// get an address for a given row in thread block
  auto getShMemIndex_y = [](const unsigned index)
  {
      return threadIdx.y + index * blockDim.y;
  };

  for (int i = 0 ; i <= numOfIters; i++) 
  {
    int linearID = i * numOfThreads + thID;
    if (linearID < numOfcells)
    {
      hist_shared[linearID] = 0;
    }
  }
  __syncthreads();

  if (boundaryCheck)
  {
    const float2 fxy = flow(i, j);
    auto indexInHistogram = static_cast<unsigned>(calc_angle(fxy.x, fxy.y)/(2*CV_PI)*binsNum);
    atomicAdd(&hist_shared[getShMemIndex_y(indexInHistogram)], 1);
  }
  __syncthreads();

// Reduce hist_shared 
  const unsigned xstride = binsNum / blockDim.x;
  for (unsigned stride = nearestGreaterPow2(blockDim.y)/2; stride > 0; stride >>= 1)
  {
    if ( (threadIdx.y < stride) && ((threadIdx.y + stride) < blockDim.y) && boundaryCheck)
    {
      for(unsigned it = 0; it <= xstride; it++)
      {
        auto bin = it * blockDim.x + threadIdx.x;
        if(bin < binsNum)
        {
          atomicAdd(&hist_shared[getShMemIndex_y(bin)], hist_shared[threadIdx.y + stride + bin * blockDim.y]);
        }  
      }
    }
    __syncthreads();
  }

// save to global memory (hist)
  if (boundaryCheck)
  {
    for (int i = 0 ; i <= binsNum / numOfThreads; i++) 
    {
      int index = i * numOfThreads + thID;
      if (index < binsNum)
      {
        hist[(blockIdx.x + blockIdx.y * gridDim.x) * binsNum + index] = hist_shared[index * blockDim.y];
      }
    }
  }
}


__global__ void reduce_hist(unsigned* hist, const unsigned binsNum)
{
// TODO: use shared memory and warp shuffle
//  reduce columns
  for (unsigned stride = nearestGreaterPow2(blockDim.x)/2; stride > 0; stride >>= 1)
  {
    if (threadIdx.x < stride && (threadIdx.x + stride) < blockDim.x)
    {
      for (unsigned id = 0; id < binsNum; id++)
      {
        hist[(threadIdx.x + threadIdx.y * blockDim.x) * binsNum + id] += hist[((threadIdx.x + stride) + threadIdx.y * blockDim.x) * binsNum + id];
      }
    }
    __syncthreads();
  }

// reduce rows

  for(unsigned stride = nearestGreaterPow2(blockDim.y)/2; stride > 0; stride >>= 1)
  {
    if(threadIdx.x == 0 && threadIdx.y < stride && threadIdx.y + stride < blockDim.y)  
    {
      for(unsigned id = 0; id < binsNum; id++)
      {
        hist[(threadIdx.y * blockDim.x) * binsNum + id] += hist[((stride + threadIdx.y) * blockDim.x) * binsNum + id];
      }
    }
    __syncthreads();
  }
}

__device__ __inline_hint__ float calc_divergence(auto & flow)
{
  return (flow(threadIdx.y, threadIdx.x+1).x - flow(threadIdx.y, threadIdx.x).x) 
        + (flow(threadIdx.y+1, threadIdx.x).y - flow(threadIdx.y, threadIdx.x).y);
}

// blockDim.x is supposed to be 32
__global__ void k_abs_div_calc_reduce_part(const PtrStepSz<float2> flow, float * divMat)
{
  extern __shared__ float2 shMem[];

  auto shFlow = [] (int i, int j) -> float2& { return shMem[i * (blockDim.x + 1) + j]; };
  auto gFlow = [&flow] (int i, int j) ->const float2& { return flow(i + blockIdx.y * blockDim.y, j + blockIdx.x * blockDim.x); };           
  auto shDiv = [] (int i) -> float& { return ((float*)&shMem[(blockDim.y-1) * blockDim.x + blockDim.x])[i]; };

  // load data to shared memory
  auto y = blockIdx.y * blockDim.y + threadIdx.y;
  auto x = blockIdx.x * blockDim.x + threadIdx.x;

  if (y < flow.rows && x < flow.cols)
  {
    shFlow(threadIdx.y, threadIdx.x) = gFlow(threadIdx.y, threadIdx.x);
  }

  // this is supposed to be the last warp in a block, so threads should not be highly divergent accross warps
  if( threadIdx.y == blockDim.y - 1) 
  {
    if ((blockIdx.x * blockDim.x + blockDim.x) < flow.cols 
      && (blockIdx.y * blockDim.y + threadIdx.x) < flow.rows
      && threadIdx.x < blockDim.y)
      shFlow(threadIdx.x, blockDim.x) = gFlow(threadIdx.x, blockDim.x);
    if((blockIdx.y * blockDim.y + threadIdx.y) < (flow.rows - 1))
      shFlow(threadIdx.y+1, threadIdx.x) = gFlow(threadIdx.y+1, threadIdx.x);
  }
  __syncthreads();


  if(y < (flow.rows-1) && x < (flow.cols-1))
  {
    // calculate divergence
    auto div = abs(calc_divergence(shFlow));
    // reduce warp (horizontally)
    for (unsigned stride = WARP_SIZE/2; stride > 0; stride >>= 1)
    {
        div += __shfl_xor_sync(__activemask(), div, stride); // default width is warpSize
    }

    // save to shared memory
    if(threadIdx.x == 0)
    {
      shDiv(threadIdx.y) = div;
    }
  }
  __syncthreads();

  // reduce vertically (but using only one warp)
  for (unsigned stride = blockDim.y/2; stride > 0; stride >>= 1)
  {
    if (threadIdx.y == 0 && (blockDim.y * blockIdx.y + threadIdx.x + stride) < (flow.rows-1) && threadIdx.x < stride && (threadIdx.x + stride) < blockDim.y)
    {
      shDiv(threadIdx.x) += shDiv(threadIdx.x + stride);
    }
    __syncthreads();
  }
  
  // // save to global memory
  if(threadIdx.x == 0 && threadIdx.y == 0 && y < (flow.rows-1) && x < (flow.cols-1))
  {
    divMat[blockIdx.y * gridDim.x + blockIdx.x] = shDiv(0);
  }

}

// template <typename T>
__global__ void k_abs_divergence_reduce(float * divMat)//, const unsigned d_mem[][], const int2 size)
{
  extern __shared__ float shMem_r[];

  auto shDiv = [] (int i, int j) -> float& { return shMem_r[i * blockDim.x + j]; };
  auto gDiv = [divMat] (int i, int j) -> float& { return divMat[i * blockDim.x + j]; };
  shDiv(threadIdx.y, threadIdx.x) = gDiv(threadIdx.y, threadIdx.x);

  __syncthreads();

// reduce horizontally
  for (unsigned stride = nearestGreaterPow2(blockDim.x)/2; stride > 0; stride >>= 1)
  {
    if (threadIdx.x < stride && (threadIdx.x + stride) < blockDim.x)
    {
      shDiv(threadIdx.y, threadIdx.x) += shDiv(threadIdx.y, threadIdx.x + stride);
    }
    __syncthreads();
  }

// reduce vertically
  for (unsigned stride = nearestGreaterPow2(blockDim.y)/2; stride > 0; stride >>= 1)
  {
    if (threadIdx.y == 0 && threadIdx.x < stride && threadIdx.x + stride < blockDim.y)
    {
      shDiv(threadIdx.x, threadIdx.y) += shDiv(threadIdx.x + stride, threadIdx.y);
    }
    __syncthreads();
  }

  if(threadIdx.x == 0 && threadIdx.y == 0)
  {
    divMat[0] = shDiv(0, 0);
  }
}

} // namespace

HoA calc_hist_of_angles(const Mat & flow,const unsigned binsNum)
{
  assert( 4 == sizeof(unsigned) );

  // Define block and grid dimensions
  dim3 blockDim(WARP_SIZE, 512/WARP_SIZE);
  dim3 gridDim(std::ceil((float)flow.cols/ blockDim.x), std::ceil((float)flow.rows/ blockDim.y));

  // Allocate device memory for hist (space needed for reduction also)
  auto numBlocks = gridDim.x * gridDim.y;
  unsigned * d_hist = nullptr;

  checkCudaErrors(cudaMalloc((void**)&d_hist, numBlocks * binsNum * sizeof(unsigned)));
  checkCudaErrors(cudaMemset(d_hist, 0, numBlocks *binsNum * sizeof(unsigned)));

  auto singleHistSize = binsNum * sizeof(unsigned);

  // Launch kernel v2
  auto sharedMemSize = blockDim.y * singleHistSize;
  k_calc_and_part_reduce_HoA<<<gridDim, blockDim, sharedMemSize>>>(flow, d_hist, binsNum);
  checkCudaErrors(cudaDeviceSynchronize());

  dim3 blockDim_reduce = gridDim;
  reduce_hist<<<1, blockDim_reduce>>>(d_hist, binsNum);
  checkCudaErrors(cudaGetLastError());
  // Copy result back to host
  HoA hist(binsNum);
  checkCudaErrors(cudaMemcpy(&hist[0], d_hist, binsNum * sizeof(unsigned), cudaMemcpyDeviceToHost));

  // Cleanup
  checkCudaErrors(cudaFree(d_hist));
  return hist;
}

MADiv calc_mean_abs_divergence(const Mat & flow)
{
  auto start = std::chrono::high_resolution_clock::now();

  dim3 childBlockDim(WARP_SIZE, WARP_SIZE);
  dim3 blockDim;
  blockDim.x = std::ceil((float)flow.cols/childBlockDim.x); 
  blockDim.y = std::ceil((float)flow.rows/childBlockDim.y);

  float * d_divMat = nullptr;
  checkCudaErrors(cudaMalloc((void**)&d_divMat, blockDim.x * blockDim.y * sizeof(float)));

  auto shMem4flow = (childBlockDim.x + 1) * childBlockDim.y + childBlockDim.x;
  auto shMem4div = childBlockDim.y;
  auto shMemSize = shMem4flow * sizeof(float2) + shMem4div * sizeof(float);

  k_abs_div_calc_reduce_part<<<blockDim, childBlockDim, shMemSize>>>(flow, d_divMat);
  checkCudaErrors(cudaDeviceSynchronize());

  // TODO: size of shared memory can be lowered at least by a half
  //  using warp shuffle and performing first step of reduction and reading global memoru in the same thread 
  k_abs_divergence_reduce<<<1, blockDim, blockDim.x * blockDim.y * sizeof(float)>>>(d_divMat);
  checkCudaErrors(cudaGetLastError());

  float div = 0.0;
  checkCudaErrors(cudaMemcpy(&div, d_divMat, sizeof(float), cudaMemcpyDeviceToHost));

  float mean = div / ((flow.rows - 1) * (flow.cols - 1));

  return MADiv(mean);
}

} // namespace ofs_detail
