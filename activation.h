int relu6(int value);
double *softmax(int *input, int size);
int *pool2dAverage(int ***&output, int ***input, int inputShape[3], int filterShape[2], int stride);