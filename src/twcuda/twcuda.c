#include "twcuda_private.h"

TwParentDetails twCreateParentDetails(int winWidth, int winHeight, int texWidth, int texHeight){
    TwParentDetails parentDetails = malloc(sizeof(*parentDetails));
    memset(parentDetails, 0, sizeof(*parentDetails));

    parentDetails->winWidth = winWidth;
    parentDetails->winHeight = winHeight;
    parentDetails->texWidth = texWidth;
    parentDetails->texHeight = texHeight;

    return parentDetails;
}

void twDestroyParentDetails(TwParentDetails parentDetails){
    free(parentDetails);
}

void twCopy(
    TwParentDetails parentDetails,
    TwReceivedChildDetails childDetails,
    float *tensorDataptr,
    uint16_t dimA,
    uint16_t dimB,
    uint16_t dimC
){
    cudaError_t cdResult;
    struct cudaExternalSemaphoreWaitParams waitParams = {0};
    struct cudaExternalSemaphoreSignalParams signalParams = {0};

    cudaWaitExternalSemaphoresAsync(&childDetails->updateConsumed, &waitParams, 1, 0);

    transposeCopy(
        tensorDataptr, childDetails->textureMemoryBuffer,
        dimA, dimB, dimC,
        parentDetails->texHeight, parentDetails->texWidth, transposeCopyXYZ
    );
    
    cudaSignalExternalSemaphoresAsync(&childDetails->textureUpdated, &signalParams, 1, 0);
}