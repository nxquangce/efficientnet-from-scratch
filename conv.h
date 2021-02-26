typedef int ts224x3[3][224][224];
typedef int ts112x32[32][112][112];
typedef int wts32x3[32][3][3][3];
typedef int vt32[32];

void createToeplitz(int **inputToeplitz, int **filterFlattened, int ***input, int ****filter, int stride);
void conv2d_4x2x2();
int *conv2d(int ***&output, int ***input, int inputShape[3], int ****weights, int weightsShape[4], int *biases, int stride = 2);