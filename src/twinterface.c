#include "twinterface.h"

void* createWindow(int winWidth, int winHeight, int texWidth, int texHeight, const char *name, const char *dir){
    TwWindowHandle windowHandle;

    windowHandle = malloc(sizeof(*windowHandle));
    memset(windowHandle, 0, sizeof(*windowHandle));
    windowHandle->childDetails = malloc(sizeof(*windowHandle->childDetails));
    memset(windowHandle->childDetails, 0, sizeof(*windowHandle->childDetails));

    windowHandle->parentDetails = twCreateParentDetails(winWidth, winHeight, texWidth, texHeight);

    twCreateWindowProcess(windowHandle->parentDetails, windowHandle->childDetails, name, dir);

    return windowHandle;
}

int isOpen(TwWindowHandle windowHandle){
    return 1;//twIsProcessStillRunning(windowHandle->childDetails->childProcess);
}

void draw(
    TwWindowHandle windowHandle,
    void *dataptr,
    int dimA, int dimB, int dimC
){
    twCopy(
        windowHandle->parentDetails,
        windowHandle->childDetails,
        dataptr,
        dimA,
        dimB,
        dimC
    );
}

void destroyWindow(TwWindowHandle windowHandle){
    twDestroyParentDetails(windowHandle->parentDetails);
    free(windowHandle);
}