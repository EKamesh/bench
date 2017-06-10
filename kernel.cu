
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE  16        // submatrix size
#define N           1024       // matrix size is N*N
cudaError_t addWithCuda(int *a, int *b, int *c, int size);

__global__ void addKernel(int *a, int *b, int *c)
{
		int bx = blockIdx.x;
		int by = blockIdx.y;
		int tx = threadIdx.x;
		int ty = threadIdx.y;
		int sum = 0;            // computed subelement
		int   ia = N * BLOCK_SIZE * by + N * ty;   // a [i][0]
		int   ib = BLOCK_SIZE * bx + tx;

		for (int k = 0; k < N; k++)
			sum += a[ia + k] * b[ib + k*N];
}

int main()
{
	int Sizebytes = N*N*sizeof(int);
	int *a, *b, *c;

	cudaError_t cudaStatus;
	a = (int*)malloc(N*N*sizeof(int));
	b = (int*)malloc(N*N*sizeof(int));
	c = (int*)malloc(N*N*sizeof(int));

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			int	k = N*i + j;

			a[k] = rand() % 100 + 1;
			b[k] = rand() % 100 + 1;
		}
	for (int i = 0; i < 20; i++){
		cudaStatus = addWithCuda(a, b, c, Sizebytes);
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "addWithCuda failed!");
			return 1;
		}
		cudaStatus = cudaDeviceReset();
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cudaDeviceReset failed!");
			return 1;
		}
	}
	free(a);
	free(b);
	free(c);
    return 0;
}

//функция для добавления массивов в gpu 
cudaError_t addWithCuda(int *a, int *b, int *c, int size)
{
	int *dev_a, *dev_b, *dev_c;
    cudaError_t cudaStatus;
	dim3 threads(16, 16);
	dim3 blocks(N / threads.x, N / threads.y);

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size, cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size, cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    addKernel<<<blocks, threads>>>(dev_a, dev_b, dev_c);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size, cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);
    
    return cudaStatus;
}
