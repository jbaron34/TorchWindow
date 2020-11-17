#include "twkernel.h"
#include <stdio.h>

#define BLOCK_SIZE 8
#define BLOCK_WIDTH BLOCK_SIZE
#define BLOCK_HEIGHT BLOCK_SIZE
#define NUM_OUT_CHANNELS 4

__global__
void dev_copy( const float  *in,
               float        *out,
               int           inDimA,
               int           inDimB,
               int           inDimC,
               int           outDimA,
               int           outDimB)
{
	int x, y, z,
        outIdx;
    float outValue;

	x = threadIdx.x + BLOCK_SIZE * blockIdx.x;
	y = threadIdx.y + BLOCK_SIZE * blockIdx.y;
	z = threadIdx.z;

    if( x >= inDimA  || y >= inDimB ||
        x >= outDimA || y >= outDimB
    ){
        return;
    }

    if (z >= inDimC ){
        outValue = 1.0f;
    }else{
        int inIdx;
        inIdx = x * inDimB * inDimC + y * inDimC + z;
        outValue = in[inIdx];
    }

    outIdx = x * outDimB * NUM_OUT_CHANNELS + y * NUM_OUT_CHANNELS + z;
    out[outIdx] = outValue;
}

void transposeCopy(
    const float *input,
    float *output,
    int inDimA,
    int inDimB,
    int inDimC,
    int outDimA,
    int outDimB,
    TransposeCopyType type
){
    switch(type){
        case transposeCopyXYZ:
        dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE, NUM_OUT_CHANNELS);
        dim3 dimGrid(
            min(inDimA, outDimA) / BLOCK_SIZE + 1,
            min(inDimB, outDimB) / BLOCK_SIZE + 1
        );

        dev_copy<<<dimGrid, dimBlock>>>(
            input, output,
            inDimA, inDimB, inDimC,
            outDimA, outDimB
        );
    }

}