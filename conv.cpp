#include <math.h>
#include <unistd.h>

#include <iostream>

#include "utils.h"
using namespace std;

typedef int ts224x3[3][224][224];
typedef int ts112x32[32][112][112];
typedef int wts32x3[32][3][3][3];
typedef int vt32[32];

int *paddingSame(int ***&output, int ***input, int inputShape[3], int filterShape[4]) {
    int numPad = (filterShape[1] - 1) / 2;
    if (numPad == 0) {
        output = input;
        int *outputShape = inputShape;
        return outputShape;
    }

    int *outputShape = new int[3]{inputShape[0] + numPad * 2, inputShape[1] + numPad * 2, inputShape[2]};
    createPointer3(output, outputShape);

    for (int idxChannel = 0; idxChannel < outputShape[2]; idxChannel += 1) {
        for (int idxPad = 0; idxPad < numPad; idxPad += 1) {
            for (int idx = 0; idx < outputShape[1]; idx += 1) {
                output[idxChannel][idxPad][idx] = 0;
                output[idxChannel][outputShape[0] - idxPad - 1][idx] = 0;
            }
        }
    }

    for (int idxChannel = 0; idxChannel < outputShape[2]; idxChannel += 1) {
        for (int idxRow = numPad; idxRow < outputShape[0] - numPad; idxRow += 1) {
            for (int idxCol = 0; idxCol < outputShape[1]; idxCol += 1) {
                output[idxChannel][idxRow][idxCol] = (((idxCol >= 0) && (idxCol < numPad)) || ((idxCol >= (outputShape[1] - numPad) && (idxCol <= (outputShape[1] - 1))))) ? 0 : input[idxChannel][idxRow - numPad][idxCol - numPad];
            }
        }
    }

    return outputShape;
}

/**
 * Calculate the Toeplitz matrix that will be created
 **/
int *calToeplitzSize(int inputShape[3], int filterShape[4], int stride) {
    int iNumChannel = inputShape[2];
    int iSize = inputShape[0];
    int numFilter = filterShape[0];
    int fSize = filterShape[1];
    // VALID padding
    // int oSize = ceil((iSize - fSize) / (float)stride + 1.0);
    // SAME padding
    int oSize = ceil((iSize - ((fSize - 1))) / (float)stride);

    int inputToeplitzWidth = oSize * oSize;
    int inputToeplitzHeight = fSize * fSize * iNumChannel;

    int *toepSize = new int[2]{inputToeplitzHeight, inputToeplitzWidth};

    return toepSize;
}

/**
 * Calculate the flatten filter matrix size for Toep transformation of the input
 **/
int *calFlattenFilterSize(int filterShape[4]) {
    int *size = new int[2]{filterShape[0], filterShape[1] * filterShape[2] * filterShape[3]};
    return size;
}

/**
 * Create Toeplitz matrix from input tensor,
 * flatten weights tensors
 **/
void createToeplitz(int **inputToeplitz, int **filterFlattened, int ***input, int inputShape[3], int ****filter, int filterShape[4], int stride) {
    // for array
    // int iNumChannel = int(sqrt(sizeof(input) / sizeof(*input)));
    // int iSize = int(sqrt(sizeof(input[0]) / sizeof(*input[0])));
    // int numFilter = int(sqrt(sizeof(filter) / sizeof(*filter)));
    // int fSize = int(sqrt(sizeof(filter[0][0]) / sizeof(*filter[0][0])));
    // int oSize = (iSize - fSize) / stride + 1;

    // from shape
    int iNumChannel = inputShape[2];
    int iSize = inputShape[0];
    int numFilter = filterShape[0];
    int fSize = filterShape[1];
    // VALID padding
    // int oSize = ceil((iSize - fSize) / (float)stride + 1.0);

    // SAME padding
    int oSize = ceil((iSize - ((fSize - 1))) / (float)stride);

    // cout << "-----------------------" << endl;
    // cout << "  Input shape : " << iSize << "x" << iSize << "x" << iNumChannel << endl;
    // cout << "  Filter shape: " << numFilter << "x" << fSize << "x" << fSize << "x" << filterShape[3] << endl;
    // cout << "    stride    : " << stride << endl;
    // cout << "  Output shape: " << oSize << "x" << oSize << "x" << numFilter << endl;
    // cout << "-----------------------" << endl;

    // Toeplitz output matrix
    int inputToeplitzWidth = oSize * oSize;
    int inputToeplitzHeight = fSize * fSize * iNumChannel;
    // int inputToeplitz[inputToeplitzHeight][inputToeplitzWidth];

    int tmpChannel = 0, tmpRow = 0, tmpCol = 0;
    for (int idxToepRow = 0; idxToepRow < inputToeplitzHeight; idxToepRow += 1) {
        for (int idxToepCol = 0; idxToepCol < inputToeplitzWidth; idxToepCol += 1) {
            inputToeplitz[idxToepRow][idxToepCol] = input[tmpChannel][tmpRow][tmpCol];

            tmpCol += stride;
            if ((tmpCol - idxToepRow % fSize) > (iSize - fSize)) {
                tmpCol = idxToepRow % fSize;
                tmpRow += 1;
            }
        }

        tmpRow = (int)(((idxToepRow + 1) % (fSize * fSize)) / fSize);
        tmpCol += 1;
        if (tmpCol == fSize) tmpCol = 0;
        if ((idxToepRow % (fSize * fSize)) == (fSize * fSize - 1)) tmpChannel += 1;
    }

    // Flatten filter
    int filterFlattenWidth = fSize * fSize * iNumChannel;
    int filterFlattenHeight = numFilter;

    int tmpIdxFilter = 0;
    tmpChannel = 0, tmpRow = 0, tmpCol = 0;
    for (int idxFilterRow = 0; idxFilterRow < filterFlattenHeight; idxFilterRow += 1) {
        for (int idxFilterCol = 0; idxFilterCol < filterFlattenWidth; idxFilterCol += 1) {
            filterFlattened[idxFilterRow][idxFilterCol] = filter[tmpIdxFilter][tmpChannel][tmpRow][tmpCol];

            tmpCol += 1;
            if (tmpCol == fSize) {
                tmpCol = 0;
                tmpRow += 1;
            }
            if (tmpRow == fSize) {
                tmpRow = 0;
                tmpChannel += 1;
            }
        }

        tmpIdxFilter += 1;
        tmpChannel = 0;
    }
}

void matMul(
    int **output, int **matrixA, int matrixASize[2], int **matrixB, int matrixBSize[2], int *biases, int (*actFn)(int) = [](int x) { return x; }) {
    int outputHeight = matrixASize[0];
    int outputWidth = matrixBSize[1];

    int *bias = (biases) ? biases : new int[outputHeight];

    // for (int idx = 0; idx < outputHeight; idx += 1) bias[idx] = 0;

    for (int idxOutRow = 0; idxOutRow < outputHeight; idxOutRow += 1) {
        for (int idxOutCol = 0; idxOutCol < outputWidth; idxOutCol += 1) {
            output[idxOutRow][idxOutCol] = 0;
            for (int idxACol = 0; idxACol < matrixASize[1]; idxACol += 1) {
                output[idxOutRow][idxOutCol] += matrixA[idxOutRow][idxACol] * matrixB[idxACol][idxOutCol];
            }

            output[idxOutRow][idxOutCol] += bias[idxOutRow];
            output[idxOutRow][idxOutCol] = actFn(output[idxOutRow][idxOutCol]);
        }
    }

    return;
}

void reshape(int ***output, int outputShape[3], int **matrix, int matrixSize[2]) {
    int numChannel = outputShape[2];
    int outSize = outputShape[0];

    int tmpRow = 0, tmpCol = 0;
    for (int idxChannel = 0; idxChannel < numChannel; idxChannel += 1) {
        for (int idxRow = 0; idxRow < outSize; idxRow += 1) {
            for (int idxCol = 0; idxCol < outSize; idxCol += 1) {
                output[idxChannel][idxRow][idxCol] = matrix[tmpRow][tmpCol];

                tmpCol += 1;
                if (tmpCol == matrixSize[1]) {
                    tmpCol = 0;
                    tmpRow += 1;
                }
            }
        }
    }

    return;
}

void conv2d_4x2x2() {  // int input[3][4][4], int weights[2][3][2][2], int stride = 1) {
    cout << "Test conv2d 3x4x4 * 2x2x2" << endl;
    int iToepSize[2] = {12, 9};
    int **iToep;
    createPointer2(iToep, iToepSize);

    cout << " Created iToep" << endl;

    int fFattenSize[2] = {2, 12};
    int **fFatten;
    createPointer2(fFatten, fFattenSize);

    cout << " Created fFatten" << endl;

    int inputShape[3] = {4, 4, 3};
    int ***input;
    createPointer3(input, inputShape);

    cout << " Created input" << endl;
    cout << "  Input:" << endl;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            cout << "   ";
            for (int k = 0; k < 4; k++) {
                input[i][j][k] = i * 16 + j * 4 + k + 1;

                if (input[i][j][k] < 10) cout << " ";
                cout << input[i][j][k] << " ";
            }

            cout << endl;
        }
    }

    int weightsShape[4] = {2, 2, 2, 3};
    int ****weights;
    createPointer4(weights, weightsShape);

    for (int i = 0; i < 2; i++) {
        weights[i] = new int **[3];
        for (int j = 0; j < 4; j++) {
            weights[i][j] = new int *[2];
            for (int k = 0; k < 2; k++) {
                weights[i][j][k] = new int[2];
                for (int u = 0; u < 2; u++)
                    weights[i][j][k][u] = i * 12 + j * 4 + k * 2 + u + 1;
            }
        }
    }

    cout << " Created weights" << endl;
    cout << "  Weights:" << endl;

    for (int i = 0; i < 2; i++) {
        for (int c = 0; c < 3; c++) {
            for (int y = 0; y < 2; y++) {
                cout << "   ";
                for (int x = 0; x < 2; x++) {
                    if (weights[i][c][y][x] < 10) cout << " ";
                    cout << weights[i][c][y][x] << " ";
                }

                cout << endl;
            }
        }
        cout << endl;
    }

    int ***iInput;
    int *iInputShape = paddingSame(iInput, input, inputShape, weightsShape);
    cout << "  Padding input:" << endl;
    print3(iInput, iInputShape);

    createToeplitz(iToep, fFatten, iInput, iInputShape, weights, weightsShape, 1);

    cout << " Calculated Toeplitz" << endl;
    cout << "  Toeplitz:" << endl;

    for (int i = 0; i < 12; i++) {
        cout << "   ";
        for (int j = 0; j < 9; j++) {
            if (iToep[i][j] < 10) cout << " ";
            cout << iToep[i][j] << " ";
        }
        cout << endl;
    }

    cout << "  Flattened Filter:" << endl;

    for (int i = 0; i < 2; i++) {
        cout << "   ";
        for (int j = 0; j < 12; j++) {
            if (fFatten[i][j] < 10) cout << " ";
            cout << fFatten[i][j] << " ";
        }
        cout << endl;
    }

    int multipliedMatrixSize[2] = {2, 9};
    int **multipliedMatrix;
    createPointer2(multipliedMatrix, multipliedMatrixSize);

    matMul(multipliedMatrix, fFatten, fFattenSize, iToep, iToepSize, NULL, [](int x) { return x; });

    cout << "  Multiplied Matrices" << endl;

    for (int i = 0; i < 2; i++) {
        cout << "   ";
        for (int j = 0; j < 9; j++) {
            cout << multipliedMatrix[i][j] << " ";
        }
        cout << endl;
    }

    int reshapedOutputSize[3] = {3, 3, 2};
    int ***reshapedOutput;
    createPointer3(reshapedOutput, reshapedOutputSize);

    reshape(reshapedOutput, reshapedOutputSize, multipliedMatrix, multipliedMatrixSize);

    cout << "  Reshaped output" << endl;
    for (int c = 0; c < 2; c += 1) {
        for (int i = 0; i < 3; i += 1) {
            cout << "   ";
            for (int j = 0; j < 3; j += 1) {
                cout << reshapedOutput[c][i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    return;
}

int *conv2d(
    int ***&output, int ***input, int inputShape[3], int ****weights, int weightsShape[4], int *biases, int stride = 2, int (*actFn)(int) = [](int x) { return x; }) {
    int ***iPadding;
    int *iPaddingShape = paddingSame(iPadding, input, inputShape, weightsShape);

    int *iToepSize = calToeplitzSize(iPaddingShape, weightsShape, stride);
    int **iToep;
    createPointer2(iToep, iToepSize);

    int *fFlattenSize = calFlattenFilterSize(weightsShape);
    int **fFlatten;
    createPointer2(fFlatten, fFlattenSize);

    createToeplitz(iToep, fFlatten, iPadding, iPaddingShape, weights, weightsShape, stride);

    int toepOutputSize[2] = {weightsShape[0], iToepSize[1]};
    int **toepOutput;
    createPointer2(toepOutput, toepOutputSize);
    matMul(toepOutput, fFlatten, fFlattenSize, iToep, iToepSize, biases, actFn);

    // int outputSize = ceil((inputShape[0] - weightsShape[1]) / (float)stride + 1.0);
    int outputSize = ceil((iPaddingShape[0] - (weightsShape[1] - 1)) / (float)stride);
    int *outputShape = new int[3]{outputSize, outputSize, weightsShape[0]};
    createPointer3(output, outputShape);
    reshape(output, outputShape, toepOutput, toepOutputSize);

    cout << "============================" << endl;
    cout << "|          Conv2d          |" << endl;
    cout << "----------------------------" << endl;
    cout << "  Input shape : " << iPaddingShape[0] << "x" << iPaddingShape[1] << "x" << iPaddingShape[2] << endl;
    cout << "  Filter shape: " << weightsShape[0] << "x" << weightsShape[1] << "x" << weightsShape[2] << "x" << weightsShape[3] << endl;
    cout << "    stride    : " << stride << endl;
    cout << "  Output shape: " << outputShape[0] << "x" << outputShape[1] << "x" << outputShape[2] << endl;
    cout << "----------------------------" << endl;

    delete2(iToep, iToepSize);
    delete2(fFlatten, fFlattenSize);
    delete[] iToepSize;
    delete[] fFlattenSize;
    delete2(toepOutput, toepOutputSize);

    return outputShape;
}

int *conv2d_depthwise(
    int ***&output, int ***input, int inputShape[3], int ****weights, int weightsShape[4], int *biases, int stride = 2, int (*actFn)(int) = [](int x) { return x; }) {
    int ***iPadding;
    int *iPaddingShape = paddingSame(iPadding, input, inputShape, weightsShape);

    int outputSize = ceil((iPaddingShape[0] - (weightsShape[1] - 1)) / (float)stride);
    int *outputShape = new int[3]{outputSize, outputSize, inputShape[2]};
    // createPointer3(output, outputShape);
    output = new int **[outputShape[2]];

    cout << "============================" << endl;
    cout << "|     Depthwise Conv2d     |" << endl;
    cout << "----------------------------" << endl;
    cout << "  Input shape : " << iPaddingShape[0] << "x" << iPaddingShape[1] << "x" << iPaddingShape[2] << endl;
    cout << "  Filter shape: " << weightsShape[0] << "x" << weightsShape[1] << "x" << weightsShape[2] << "x" << weightsShape[3] << endl;
    cout << "    stride    : " << stride << endl;
    cout << "  Output shape: " << outputShape[0] << "x" << outputShape[1] << "x" << outputShape[2] << endl;
    cout << "----------------------------" << endl;

    for (int idxChannel = 0; idxChannel < inputShape[2]; idxChannel += 1) {
        int localWeightShape[4] = {weightsShape[0], weightsShape[1], weightsShape[2], 1};
        int ****localWeights = new int ***[localWeightShape[0]];
        localWeights[0] = new int **[localWeightShape[2]];
        localWeights[0][0] = weights[0][idxChannel];

        int localIPaddingShape[3] = {iPaddingShape[0], iPaddingShape[1], 1};
        int ***localIPadding = new int **[1];
        localIPadding[0] = iPadding[idxChannel];

        int *iToepSize = calToeplitzSize(localIPaddingShape, localWeightShape, stride);
        int **iToep;
        createPointer2(iToep, iToepSize);

        int *fFlattenSize = calFlattenFilterSize(localWeightShape);
        int **fFlatten;
        createPointer2(fFlatten, fFlattenSize);

        createToeplitz(iToep, fFlatten, localIPadding, localIPaddingShape, localWeights, localWeightShape, stride);

        int toepOutputSize[2] = {weightsShape[0], iToepSize[1]};
        int **toepOutput;
        createPointer2(toepOutput, toepOutputSize);
        matMul(toepOutput, fFlatten, fFlattenSize, iToep, iToepSize, biases, actFn);

        // int outputSize = ceil((inputShape[0] - weightsShape[1]) / (float)stride + 1.0);
        int localOutputSize = ceil((iPaddingShape[0] - (weightsShape[1] - 1)) / (float)stride);
        int *localOutputShape = new int[3]{localOutputSize, localOutputSize, 1};
        int ***localOutput;
        createPointer3(localOutput, localOutputShape);
        reshape(localOutput, localOutputShape, toepOutput, toepOutputSize);

        output[idxChannel] = localOutput[0];

        delete[] localWeights;
        delete[] localIPadding;
        delete[] localOutput;
        delete2(iToep, iToepSize);
        delete2(fFlatten, fFlattenSize);
        delete[] iToepSize;
        delete[] fFlattenSize;
        delete2(toepOutput, toepOutputSize);
    }

    return outputShape;
}

int *matAdd(int ***&output, int ***matrixA, int ***matrixB, int shape[3]) {
    int *outputShape = new int[3]{shape[0], shape[1], shape[2]};
    createPointer3(output, outputShape);

    for (int idxChannel = 0; idxChannel < shape[2]; idxChannel += 1) {
        for (int idxRow = 0; idxRow < shape[0]; idxRow += 1) {
            for (int idxCol = 0; idxCol < shape[1]; idxCol += 1) {
                output[idxChannel][idxCol][idxRow] = matrixA[idxChannel][idxCol][idxRow] + matrixB[idxChannel][idxCol][idxRow];
            }
        }
    }

    return outputShape;
}