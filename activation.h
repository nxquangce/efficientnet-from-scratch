int8_t relu6(int8_t value);
double *softmax(int8_t *input, int size);
int *pool2dAverage(int8_t ***&output, int8_t ***input, int inputShape[3], int filterShape[2], int stride);