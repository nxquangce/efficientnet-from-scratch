#include <math.h>

#include <iostream>

using namespace std;

typedef int ts224x3[3][224][224];
typedef int ts112x32[32][112][112];
typedef int wts32x3[32][3][3][3];
typedef int vt32[32];

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

void conv2d_4x2x2() {  // int input[3][4][4], int weights[2][3][2][2], int stride = 1) {
    cout << "Test conv2d 3x4x4 * 2x2x2" << endl;
    int **iToep;
    iToep = new int *[12];
    for (int i = 0; i < 12; i++)
        iToep[i] = new int[11];

    cout << " Created iToep" << endl;

    int **fFatten;
    fFatten = new int *[2];
    for (int i = 0; i < 2; i++)
        fFatten[i] = new int[12];

    cout << " Created fFatten" << endl;

    int inputShape[3] = {4, 4, 3};
    int ***input;
    input = new int **[3];
    for (int i = 0; i < 3; i++) {
        input[i] = new int *[4];
        for (int j = 0; j < 4; j++) {
            input[i][j] = new int[4];
            for (int k = 0; k < 4; k++)
                input[i][j][k] = i * 16 + j * 4 + k + 1;
        }
    }

    cout << " Created input" << endl;
    cout << "  Input:" << endl;

    for (int i = 0; i < 3; i++) {
        for (int y = 0; y < 4; y++) {
            cout << "   ";
            for (int x = 0; x < 4; x++) {
                if (input[i][y][x] < 10) cout << " ";
                cout << input[i][y][x] << " ";
            }

            cout << endl;
        }
    }

    int weightsShape[4] = {2, 2, 2, 3};
    int ****weights;
    weights = new int ***[2];
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

    return;
}

ts112x32 *conv2d_224x32(ts224x3 input, wts32x3 weights, vt32 biases, int stide_h = 2, int stride_w = 2) {
    ts112x32 output;

    return &output;
}