typedef int ts224x3[3][224][224];
typedef int ts112x32[32][112][112];
typedef int wts32x3[32][3][3][3];
typedef int vt32[32];

void createToeplitz(int **inputToeplitz, int **filterFlattened, int ***input, int ****filter, int stride);
void conv2d_4x2x2();
int *conv2d(
    int ***&output, int ***input, int inputShape[3], int ****weights, int weightsShape[4], int *biases, int stride = 2, int (*actFn)(int) = [](int x) { return x; });
int *conv2d_depthwise(
    int ***&output, int ***input, int inputShape[3], int ****weights, int weightsShape[4], int *biases, int stride = 2, int (*actFn)(int) = [](int x) { return x; });
void matMul(
    int **output, int **matrixA, int matrixASize[2], int **matrixB, int matrixBSize[2], int *biases, int (*actFn)(int) = [](int x) { return x; });
int *matAdd(int ***&output, int ***matrixA, int ***matrixB, int shape[3]);