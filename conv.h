typedef int ts224x3[3][224][224];
typedef int ts112x32[32][112][112];
typedef int wts32x3[32][3][3][3];
typedef int vt32[32];

void createToeplitz(int8_t **inputToeplitz, int8_t **filterFlattened, int8_t ***input, int8_t ****filter, int stride);
void conv2d_4x2x2();
int *conv2d(
    int8_t ***&output, int8_t ***input, int inputShape[3], int8_t ****weights, int weightsShape[4], int8_t *biases, int stride = 2, int8_t (*actFn)(int8_t) = [](int8_t x) { return x; });
int *conv2d_depthwise(
    int8_t ***&output, int8_t ***input, int inputShape[3], int8_t ****weights, int weightsShape[4], int8_t *biases, int stride = 2, int8_t (*actFn)(int8_t) = [](int8_t x) { return x; });
void matMul(
    int8_t **output, int8_t **matrixA, int matrixASize[2], int8_t **matrixB, int matrixBSize[2], int8_t *biases, int8_t (*actFn)(int8_t) = [](int8_t x) { return x; });
int *matAdd(int8_t ***&output, int8_t ***matrixA, int8_t ***matrixB, int shape[3]);