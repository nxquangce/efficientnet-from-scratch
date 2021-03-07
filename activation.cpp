#include <math.h>

#include "utils.h"
using namespace std;

int relu6(int value) {
    if (value < 0) return 0;
    if (value > 6) return 6;
    return value;
}

double *softmax(int *input, int size) {
    double *expInput = new double[size];
    double *output = new double[size];
    double expSum = 0.0;

    // Prevent overflow
    int maxInput = input[0];
    for (int idx = 0; idx < size; idx += 1) {
        if (input[idx] > maxInput) maxInput = input[idx];
    }

    for (int idx = 0; idx < size; idx += 1) {
        expInput[idx] = exp((double)(input[idx] - maxInput));
        expSum += expInput[idx];
        // cout << input[idx] << " -> " << input[idx] - maxInput << " -> " << expInput[idx] << " -> " << expSum << endl;
    }

    for (int idx = 0; idx < size; idx += 1) {
        output[idx] = expInput[idx] / expSum;
        // cout << idx << ": " << expInput[idx] << " / " << expSum << " = " << output[idx] << endl;
    }

    delete[] expInput;

    return output;
}

int *pool2dAverage(int ***&output, int ***input, int inputShape[3], int filterShape[2], int stride) {
    int outputSize = ceil((inputShape[0] - filterShape[0]) / (float)stride + 1);
    int *outputShape = new int[3]{outputSize, outputSize, inputShape[2]};

    createPointer3(output, outputShape);

    for (int idxChannel = 0; idxChannel < outputShape[2]; idxChannel += 1) {
        for (int idxRow = 0; idxRow < outputShape[0]; idxRow += 1) {
            for (int idxCol = 0; idxCol < outputShape[1]; idxCol += 1) {
                output[idxChannel][idxRow][idxCol] = 0;
                for (int idxFRow = 0; idxFRow < filterShape[0]; idxFRow += 1) {
                    for (int idxFCol = 0; idxFCol < filterShape[1]; idxFCol += 1) {
                        output[idxChannel][idxRow][idxCol] += input[idxChannel][idxFRow + idxRow * stride][idxFCol + idxCol * stride];
                    }
                }
                output[idxChannel][idxRow][idxCol] /= (filterShape[0] * filterShape[1]);
            }
        }
    }

    return outputShape;
}