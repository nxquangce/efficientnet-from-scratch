#include <cstring>
#include <iostream>

using namespace std;

unsigned charTOunsigned(const char *c) {
    char p = *c;
    unsigned ergebnis = 0;
    while (p) {
        ergebnis = ergebnis * 10 + (p - '0');
        c++;
        p = *c;
    }
    return ergebnis;
}

int charTOint(const char *c) {
    return (*c == '-') ? -charTOunsigned(c + 1) : charTOunsigned(c);
}

const char *popWord(string &line, string delimiter, size_t &pos) {
    pos = line.find(delimiter);
    char *word;
    if (pos == string::npos) {
        word = new char[line.length()];
        strcpy(word, line.substr(0, line.length()).c_str());
    } else {
        word = new char[pos];
        strcpy(word, line.substr(0, pos).c_str());
    }
    size_t posDel = (pos == string::npos) ? line.length() : pos;
    line.erase(0, posDel + delimiter.length());

    return word;
}

void print1(int8_t *pointer, int size) {
    for (int i = 0; i < size; i++) {
        cout << (int)pointer[i] << " ";
    }
    cout << endl;
}

void print3(int8_t ***pointer, int shape[3]) {
    for (int i = 0; i < shape[2]; i++) {
        for (int j = 0; j < shape[0]; j++) {
            cout << "   ";
            for (int k = 0; k < shape[1]; k++) {
                if (pointer[i][j][k] < 10) cout << " ";
                cout << (int)pointer[i][j][k] << " ";
            }

            cout << endl;
        }
    }
}

void print4(int8_t ****pointer, int shape[4]) {
    for (int f = 0; f < shape[0]; f++) {
        for (int c = 0; c < shape[3]; c++) {
            for (int i = 0; i < shape[1]; i++) {
                for (int j = 0; j < shape[2]; j++) {
                    cout << (int)pointer[f][c][i][j] << " ";
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
void createPointer2(int8_t **&pointer, int size[2]) {
    pointer = new int8_t *[size[0]];
    for (int i = 0; i < size[0]; i++)
        pointer[i] = new int8_t[size[1]];

    return;
}

/**
 * Create a 3D array with pointer
 **/
void createPointer3(int8_t ***&pointer, int size[3]) {
    pointer = new int8_t **[size[2]];
    for (int i = 0; i < size[2]; i++) {
        pointer[i] = new int8_t *[size[0]];
        for (int j = 0; j < size[0]; j++) {
            pointer[i][j] = new int8_t[size[1]];
        }
    }

    return;
}

/**
 * Create a 4D array with pointer
 **/
void createPointer4(int8_t ****&pointer, int size[4]) {
    pointer = new int8_t ***[size[0]];
    for (int i = 0; i < size[0]; i++) {
        pointer[i] = new int8_t **[size[3]];
        for (int j = 0; j < size[3]; j++) {
            pointer[i][j] = new int8_t *[size[1]];
            for (int k = 0; k < size[1]; k++) {
                pointer[i][j][k] = new int8_t[size[2]];
            }
        }
    }

    return;
}

void delete2(int8_t **pointer, int size[2]) {
    for (int i = 0; i < size[0]; i++) {
        delete[] pointer[i];
    }
    delete[] pointer;
    pointer = NULL;

    return;
}

void delete3(int8_t ***&pointer, int size[3]) {
    for (int i = 0; i < size[2]; i++) {
        for (int j = 0; j < size[0]; j++) {
            delete[] pointer[i][j];
        }
        delete[] pointer[i];
    }
    delete[] pointer;

    return;
}

void delete4(int8_t ****&pointer, int size[4]) {
    for (int i = 0; i < size[0]; i++) {
        for (int j = 0; j < size[3]; j++) {
            for (int k = 0; k < size[1]; k++) {
                delete[] pointer[i][j][k];
            }
            delete[] pointer[i][j];
        }
        delete[] pointer[i];
    }
    delete[] pointer;
    pointer = NULL;

    return;
}

void copy1(int *&output, int *input, int size) {
    output = new int[size];
    for (int i = 0; i < size; i++)
        output[i] = input[i];

    return;
}

void copy3(int8_t ***&output, int8_t ***input, int shape[3]) {
    output = new int8_t **[shape[2]];
    for (int i = 0; i < shape[2]; i++) {
        output[i] = new int8_t *[shape[0]];
        for (int j = 0; j < shape[0]; j++) {
            output[i][j] = new int8_t[shape[1]];
            for (int k = 0; k < shape[1]; k++) {
                output[i][j][k] = input[i][j][k];
            }
        }
    }

    return;
}

int *reshapey1xtxy(int8_t **&output, int8_t ***input, int inputShape[3]) {
    int *outputSize = new int[2]{inputShape[1], inputShape[2]};
    createPointer2(output, outputSize);
    for (int idxRow = 0; idxRow < outputSize[0]; idxRow += 1) {
        for (int idxCol = 0; idxCol < outputSize[1]; idxCol += 1) {
            output[idxRow][idxCol] = input[idxCol][0][idxRow];
        }
    }

    return outputSize;
}

void quantize3(int8_t ***input, int shape[3], double param1, double param2) {
    for (int idxChannel = 0; idxChannel < shape[2]; idxChannel += 1) {
        for (int idxRow = 0; idxRow < shape[0]; idxRow += 1) {
            for (int idxCol = 0; idxCol < shape[1]; idxCol += 1) {
                input[idxChannel][idxRow][idxCol] = param1 * (input[idxChannel][idxRow][idxCol] - param2);
            }
        }
    }
}

void quantize2(int8_t **input, int shape[2], double param1, double param2) {
        for (int idxRow = 0; idxRow < shape[0]; idxRow += 1) {
            for (int idxCol = 0; idxCol < shape[1]; idxCol += 1) {
                input[idxRow][idxCol] = param1 * (input[idxRow][idxCol] - param2);
            }
        }
}