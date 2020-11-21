#include <twcommon.h>
#include <twcuda.h>

typedef struct TwWindowHandle_T *TwWindowHandle;

struct TwWindowHandle_T{
    TwParentDetails parentDetails;
    TwReceivedChildDetails childDetails;
    float *dataptr;
};

void* createWindow(int winWidth, int winHeight, int texWidth, int texHeight, const char *name, const char *dir);

void destroyWindow(TwWindowHandle window);

void draw(
    TwWindowHandle windowHandle,
    void *dataptr,
    int dimA, int dimB, int dimC
);