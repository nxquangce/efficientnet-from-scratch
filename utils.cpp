#include <iostream>

using namespace std;

void print1(int *pointer, int size) {
    for (int i = 0; i < size; i++) {
        cout << pointer[i] << " ";
    }
    cout << endl;
}

void print3(int ***pointer, int shape[3]) {
    for (int i = 0; i < shape[2]; i++) {
        for (int j = 0; j < shape[0]; j++) {
            cout << "   ";
            for (int k = 0; k < shape[1]; k++) {
                if (pointer[i][j][k] < 10) cout << " ";
                cout << pointer[i][j][k] << " ";
            }

            cout << endl;
        }
    }
}

void print4(int ****pointer, int shape[4]) {
    for (int f = 0; f < shape[0]; f++) {
        for (int c = 0; c < shape[3]; c++) {
            for (int i = 0; i < shape[1]; i++) {
                for (int j = 0; j < shape[2]; j++) {
                    cout << pointer[f][c][i][j] << " ";
                }
                cout << endl;
            }
            cout << endl;
        }
    }
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

void delete2(int **pointer, int size[2]) {
    for (int i = 0; i < size[0]; i++) {
        delete[] pointer[i];
    }
    delete[] pointer;
}