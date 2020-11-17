#pragma once

#ifndef TW_CUDA_H
#define TW_CUDA_H

#include <cuda_runtime_api.h>

typedef struct TwParentDetails_T *TwParentDetails;

typedef struct TwReceivedChildDetails_T *TwReceivedChildDetails;

struct TwParentDetails_T{
    uint16_t winWidth;
    uint16_t winHeight;
    uint16_t texWidth;
    uint16_t texHeight;
};

struct TwReceivedChildDetails_T{
    void *childProcess;
    float *textureMemoryBuffer;
    size_t bufferSize;
    cudaExternalSemaphore_t textureUpdated;
    cudaExternalSemaphore_t updateConsumed;
};

TwParentDetails twCreateParentDetails(int winWidth, int winHeight, int texWidth, int texHeight);

void twDestroyParentDetails(TwParentDetails parentDetails);

void twCreateWindowProcess(
    TwParentDetails parentDetails,
    TwReceivedChildDetails childDetails,
    const char *name,
    const char *dir
);

void twCopy(
    TwParentDetails parentDetails,
    TwReceivedChildDetails childDetails,
    float *tensorDataptr,
    uint16_t dimA,
    uint16_t dimB,
    uint16_t dimC
);


#endif /* TW_CUDA_H */