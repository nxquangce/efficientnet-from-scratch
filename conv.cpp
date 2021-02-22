#include <math.h>

#include <iostream>

using namespace std;

typedef int ts224x3[3][224][224];
typedef int ts112x32[32][112][112];
typedef int wts32x3[32][3][3][3];
typedef int vt32[32];

void createToeplitz(int **inputToeplitz, int **filterFlattened, int ***input, int ****filter, int stride) {
    // Size of output
    int iNumChannel = int(sqrt(sizeof(input) / sizeof(*input)));
    int iSize = int(sqrt(sizeof(input[0]) / sizeof(*input[0])));
    int numFilter = int(sqrt(sizeof(filter) / sizeof(*filter)));
    int fSize = int(sqrt(sizeof(filter[0][0]) / sizeof(*filter[0][0])));
    int oSize = (iSize - fSize) / stride + 1;

    // Toeplitz output matrix
    int inputToeplitzWidth = oSize * oSize;
    int inputToeplitzHeight = fSize * fSize * iNumChannel;
    // int inputToeplitz[inputToeplitzHeight][inputToeplitzWidth];

    int tmpChannel = 0, tmpRow = 0, tmpCol = 0;
    for (int idxToepRow = 0; idxToepRow < inputToeplitzHeight; idxToepRow += 1) {
        for (int idxToepCol = 0; idxToepCol < inputToeplitzWidth; idxToepCol += 1) {
            inputToeplitz[idxToepRow][idxToepCol] = input[tmpChannel][tmpRow][tmpCol];

            tmpCol += stride;
            if (tmpCol > iSize - fSize) {
                tmpCol = 0;
                tmpRow += 1;
            }
        }

        tmpRow = 0;
        tmpCol += 1;
        if (tmpCol == fSize) tmpCol = 0;
    }

    // Fltten filter
    int filterFlattenWidth = fSize * fSize * iNumChannel;
    int filterFlattenHeight = numFilter;

    int tmpIdxFilter = 0;
    tmpChannel = 0, tmpRow = 0, tmpCol = 0;
    for (int idxFilterRow = 0; idxFilterRow < filterFlattenWidth; idxFilterRow += 1) {
        for (int idxFilterCol = 0; idxFilterCol < filterFlattenHeight; idxFilterCol += 1) {
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

void conv2d_4x2x2() {  // int input[3][4][4], int weights[2][3][2][2], int stride = 1) {
    int **iToep;
    iToep = new int *[12];
    for (int i = 0; i < 12; i++)
        iToep[i] = new int[11];

    int **fFatten;
    fFatten = new int *[2];
    for (int i = 0; i < 2; i++)
        fFatten[i] = new int[12];

    int ***input;
    input = new int **[3];
    for (int i = 0; i < 3; i++) {
        input[i] = new int *[4];
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                input[i][j][k] = j * 4 + k;
    }

    int ****weights;
    weights = new int ***[2];
    for (int i = 0; i < 3; i++) {
        weights[i] = new int **[3];
        for (int j = 0; j < 4; j++) {
            weights[i][j] = new int *[2];
            for (int k = 0; k < 2; k++) {
                for (int u = 0; u < 2; u++)
                    weights[i][j][k][u] = j * 2 + k;
            }
        }
    }

    createToeplitz(iToep, fFatten, input, weights, 1);

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 11; j++) {
            cout << iToep[i][j];
        }
        cout << endl;
    }

    return;
}

ts112x32 *conv2d_224x32(ts224x3 input, wts32x3 weights, vt32 biases, int stide_h = 2, int stride_w = 2) {
    ts112x32 output;

    return &output;
}