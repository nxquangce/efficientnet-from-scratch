#include <math.h>

#include <iostream>

using namespace std;

typedef int ts224x3[3][224][224];
typedef int ts112x32[32][112][112];
typedef int wts32x3[32][3][3][3];
typedef int vt32[32];

/**
 * Calculate the Toeplitz matrix that will be created
 **/
int *calToeplitzSize(int inputShape[3], int filterShape[4], int stride) {
    int iNumChannel = inputShape[2];
    int iSize = inputShape[0];
    int numFilter = filterShape[0];
    int fSize = filterShape[1];
    int oSize = (iSize - fSize) / stride + 1;

    int inputToeplitzWidth = oSize * oSize;
    int inputToeplitzHeight = fSize * fSize * iNumChannel;

    int toepSize[2] = {inputToeplitzHeight, inputToeplitzWidth};

    return toepSize;
}

/**
 * Calculate the flatten filter matrix size for Toep transformation of the input
 **/
int *calFlattenFilterSize(int filterShape[4]) {
    int size[2] = {filterShape[0], filterShape[1] * filterShape[2] * filterShape[3]};
    return size;
}

/**
 * Create Toeplitz matrix from input tensor,
 * flatten weights tensors
 **/
void createToeplitz(int **inputToeplitz, int **filterFlattened, int ***input, int inputShape[3], int ****filter, int filterShape[4], int stride) {
    // Size of output

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
    int oSize = (iSize - fSize) / stride + 1;

    cout << "  iNumChannel: " << iNumChannel << endl;
    cout << "  iSize:       " << iSize << endl;
    cout << "  numFilter:   " << numFilter << endl;
    cout << "  fSize:       " << fSize << endl;
    cout << "  oSize:       " << oSize << endl;

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
        if ((idxToepRow % (fSize * fSize)) == 3) tmpChannel += 1;
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

void matMul(int **output, int **matrixA, int matrixASize[2], int **matrixB, int matrixBSize[2]) {
    int outputHeight = matrixASize[0];
    int outputWidth = matrixBSize[1];

    for (int idxOutRow = 0; idxOutRow < outputHeight; idxOutRow += 1) {
        for (int idxOutCol = 0; idxOutCol < outputWidth; idxOutCol += 1) {
            output[idxOutRow][idxOutCol] = 0;
            for (int idxACol = 0; idxACol < matrixASize[1]; idxACol += 1) {
                output[idxOutRow][idxOutCol] += matrixA[idxOutRow][idxACol] * matrixB[idxACol][idxOutCol];
            }
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

/**
 * Create a 2D array with pointer
 **/
void createPointer2(int **&pointer, int size[2]) {
    pointer = new int *[size[0]];
    for (int i = 0; i < size[0]; i++)
        pointer[i] = new int[size[1]];

    return;
}

/**
 * Create a 3D array with pointer
 **/
void createPointer3(int ***&pointer, int size[3]) {
    pointer = new int **[size[2]];
    for (int i = 0; i < size[2]; i++) {
        pointer[i] = new int *[size[0]];
        for (int j = 0; j < size[0]; j++) {
            pointer[i][j] = new int[size[1]];
        }
    }

    return;
}

/**
 * Create a 4D array with pointer
 **/
void createPointer4(int ****&pointer, int size[4]) {
    pointer = new int ***[size[0]];
    for (int i = 0; i < size[0]; i++) {
        pointer[i] = new int **[size[3]];
        for (int j = 0; j < size[3]; j++) {
            pointer[i][j] = new int *[size[1]];
            for (int k = 0; k < size[1]; k++) {
                pointer[i][j][k] = new int[size[2]];
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

    createToeplitz(iToep, fFatten, input, inputShape, weights, weightsShape, 1);

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

    matMul(multipliedMatrix, fFatten, fFattenSize, iToep, iToepSize);

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

void conv2d(int ***&output, int ***input, int inputShape[3], int ****weights, int weightsShape[4], int *biases, int stride = 2) {
    int *iToepSize = calToeplitzSize(inputShape, weightsShape, stride);
    int **iToep;
    createPointer2(iToep, iToepSize);

    int *fFlattenSize = calFlattenFilterSize(weightsShape);
    int **fFlatten;
    createPointer2(fFlatten, fFlattenSize);

    createToeplitz(iToep, fFlatten, input, inputShape, weights, weightsShape, stride);

    int toepOutputSize[2] = {weightsShape[0], iToepSize[1]};
    int **toepOutput;
    createPointer2(toepOutput, toepOutputSize);

    matMul(toepOutput, fFlatten, fFlattenSize, iToep, iToepSize);

    int outputSize = (inputShape[0] - weightsShape[1]) / stride + 1;
    int outputShape[3] = {weightsShape[0], outputSize, outputSize};
    createPointer3(output, outputShape);
    reshape(output, outputShape, toepOutput, toepOutputSize);

    return;
}