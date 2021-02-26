#include <stdio.h>

#include <cstring>
#include <iostream>
#include <string>

#include "conv.h"
#include "utils.h"

using namespace std;

void parseLine(int *output, string line);
int *collect(FILE *fp, int *cmd, int ****&weights, int *&biases);

int main() {
    cout << "Efficientnet-lite0 int8" << endl;

    FILE *fp;
    fp = fopen("weights.txt", "r");
    if (fp == NULL) perror("Error opening file");

    char line[100];
    fgets(line, 100, fp);
    line[strcspn(line, "\n")] = 0;
    int cmd[5] = {};

    // Conv2d - 1
    int ****weights;
    int *biases;

    parseLine(cmd, line);
    int numFilter = cmd[1];
    int size = cmd[2];
    int numChannel = cmd[4];

    int weightsShape[4] = {numFilter, size, size, numChannel};
    collect(fp, cmd, weights, biases);
    // print4(weights, weightsShape);

    fgets(line, 100, fp);
    line[strcspn(line, "\n")] = 0;
    parseLine(cmd, line);
    collect(fp, cmd, weights, biases);
    int biasSize = cmd[1];
    // print1(biases, 32);

    int inputShape[3] = {224, 224, 3};
    int ***input;
    createPointer3(input, inputShape);

    cout << " Created input" << endl;
    for (int i = 0; i < inputShape[2]; i++) {
        for (int j = 0; j < inputShape[0]; j++) {
            // cout << "   ";
            for (int k = 0; k < inputShape[1]; k++) {
                input[i][j][k] = (i * 224 * 224 + j * 4 + k + 1) % 2;

                // if (input[i][j][k] < 10) cout << " ";
                // cout << input[i][j][k] << " ";
            }

            // cout << endl;
        }
    }

    int ***output;
    int *outputShape = conv2d(output, input, inputShape, weights, weightsShape, biases, 2);
    // print3(output, outputShape);

    FILE *fConvOut;
    fConvOut = fopen("out_conv1.txt", "w+");
    for (int i = 0; i < outputShape[2]; i++) {
        for (int j = 0; j < outputShape[0]; j++) {
            for (int k = 0; k < outputShape[1]; k++) {
                if (output[i][j][k] < 10) cout << " ";
                fputs((to_string(output[i][j][k]) + " ").c_str(), fConvOut);
            }
            fputs("\n", fConvOut);
        }
        fputs("\n", fConvOut);
    }
    fclose(fConvOut);

    fclose(fp);

    return 0;
}

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

void parseLine(int *output, string line) {
    string delimiter = " ";
    size_t pos = 0;

    int paramCount = 0;
    while (line != "") {
        pos = line.find(delimiter);
        const char *word = (pos != string::npos) ? line.substr(0, pos).c_str() : line.substr(0).c_str();
        if (pos == string::npos) pos = strlen(line.c_str()) - delimiter.length();
        line.erase(0, pos + delimiter.length());

        if (strcmp(word, "weight") == 0)
            output[paramCount] = 1;
        else if (strcmp(word, "bias") == 0)
            output[paramCount] = 2;
        else
            output[paramCount] = charTOint(word);
        paramCount += 1;
    }
}

int *collect(FILE *fp, int *cmd, int ****&weights, int *&biases) {
    if (cmd[0] == 1) {
        int numFilter = cmd[1];
        int size = cmd[2];
        int numChannel = cmd[4];

        int shape[4] = {numFilter, size, size, numChannel};

        createPointer4(weights, shape);

        int numLine = numFilter * numChannel * size;

        char lineRaw[100];
        int idxFilter = 0, idxChannel = 0, idxRow = 0, idxCol = 0;
        for (int idx = 0; idx < numLine; idx += 1) {
            fgets(lineRaw, 100, fp);
            lineRaw[strcspn(lineRaw, "\n")] = 0;
            string line = string(lineRaw);

            string delimiter = " ";
            size_t pos = 0;

            int paramCount = 0;

            int value;
            while (line != "") {
                pos = line.find(delimiter);
                const char *word = (pos != string::npos) ? line.substr(0, pos).c_str() : line.substr(0).c_str();
                if (pos == string::npos) pos = strlen(string(line).c_str()) - delimiter.length();
                line.erase(0, pos + delimiter.length()).c_str();

                value = charTOint(word);

                weights[idxFilter][idxChannel][idxRow][idxCol] = value;

                idxCol += 1;
                if (idxCol == size) {
                    idxCol = 0;
                    idxRow += 1;
                }
                if (idxRow == size) {
                    idxRow = 0;
                    idxChannel += 1;
                }
                if (idxChannel == numChannel) {
                    idxChannel = 0;
                    idxFilter += 1;
                }
            }
        }
    }

    if (cmd[0] == 2) {
        int numBias = cmd[1];

        biases = new int[numBias];

        char lineRaw[200];
        fgets(lineRaw, 200, fp);
        lineRaw[strcspn(lineRaw, "\n")] = 0;
        string line = string(lineRaw);

        string delimiter = " ";
        size_t pos = 0;
        int idx = 0;
        while (line != "") {
            pos = line.find(delimiter);
            const char *word = (pos != string::npos) ? line.substr(0, pos).c_str() : line.substr(0).c_str();
            if (pos == string::npos) pos = strlen(string(line).c_str()) - delimiter.length();
            line.erase(0, pos + delimiter.length()).c_str();

            biases[idx] = charTOint(word);
            idx += 1;
        }
    }

    for (int idx = 0; idx < 5; idx++)
        cmd[idx] = 0;
}