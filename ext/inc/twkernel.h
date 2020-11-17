#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    transposeCopyXYZ = 0,
    transposeCopyXZY = 1,
    transposeCopyYXZ = 2,
    transposeCopyYZX = 3,
    transposeCopyZXY = 4,
    transposeCopyZYX = 5
} TransposeCopyType;

void transposeCopy(
    const float *input,
    float *output,
    int inDimA,
    int inDimB,
    int inDimC,
    int outDimA,
    int outDimB,
    TransposeCopyType type
);

void transposeCopyStrided(
    float *input,
    float *output,
    int inDimA,
    int inDimB,
    int inDimC,
    int outDimA,
    int outDimB,
    int outDimC,
    int strideA,
    int strideB,
    int strideC,
    TransposeCopyType type
);

#ifdef __cplusplus
} /* extern "C" */
#endif