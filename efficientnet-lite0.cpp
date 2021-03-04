#include <stdio.h>

#include <cstring>
#include <iostream>

#include "activation.h"
#include "conv.h"
#include "utils.h"

using namespace std;

enum Command {
    CONV2D,
    DEPTHWISE_CON2D,
    ADD,
    RELU6,
    NONE,
    WEIGHT,
    BIAS
};

void parseLine(int *output, string line);
int *collect(FILE *fp, int *cmd, int ****&weights, int *&biases);

int main() {
    cout << "Efficientnet-lite0 int8" << endl;

    FILE *fp;
    fp = fopen("weights.txt", "r");
    if (fp == NULL) perror("Error opening file");

    // Input
    // TODO: Repalce below code: Use opencv to read an image

    cout << " Created input" << endl;
    int inputShape[3] = {224, 224, 3};
    int ***input;
    createPointer3(input, inputShape);

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

    char line[100];
    int cmd[5] = {};
    int ****weights;
    int *biases;

    bool pingpong = true;

    int ***output0;
    int *outputShape0;
    int ***output1;
    int *outputShape1;
    int ***mem;
    int *memShape;

    // First
    output1 = input;
    outputShape1 = inputShape;

    while (fgets(line, 100, fp)) {
        // fgets(line, 100, fp);
        line[strcspn(line, "\n")] = 0;
        parseLine(cmd, line);

        int layerCmd[5] = {cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]};
        int stride = layerCmd[2];

        if (layerCmd[0] == CONV2D) {
            fgets(line, 100, fp);
            line[strcspn(line, "\n")] = 0;
            parseLine(cmd, line);
            int numFilter = cmd[1];
            int size = cmd[2];
            int numChannel = cmd[4];

            int weightsShape[4] = {numFilter, size, size, numChannel};
            collect(fp, cmd, weights, biases);
            // print4(weights, weightsShape);

            if (layerCmd[1] == 1) {
                fgets(line, 100, fp);
                line[strcspn(line, "\n")] = 0;
                parseLine(cmd, line);
                collect(fp, cmd, weights, biases);
                int biasSize = cmd[1];
                // print1(biases, 32);
            }

            if (pingpong) {
                if (layerCmd[3] == RELU6)
                    outputShape0 = conv2d(output0, output1, outputShape1, weights, weightsShape, biases, stride, relu6);
                else
                    outputShape0 = conv2d(output0, output1, outputShape1, weights, weightsShape, biases, stride);

                if (layerCmd[4] == 1) {
                    copy3(mem, output0, outputShape0);
                    copy1(memShape, outputShape0, 3);
                }

                delete3(output1, outputShape1);
            } else {
                if (layerCmd[3] == RELU6)
                    outputShape1 = conv2d(output1, output0, outputShape0, weights, weightsShape, biases, stride, relu6);
                else
                    outputShape1 = conv2d(output1, output0, outputShape0, weights, weightsShape, biases, stride);

                if (layerCmd[4] == 1) {
                    copy3(mem, output1, outputShape1);
                    copy1(memShape, outputShape1, 3);
                }

                delete3(output0, outputShape0);
            }
            // print3(output, outputShape);

            delete[] biases;
            delete4(weights, weightsShape);
        }

        if (layerCmd[0] == DEPTHWISE_CON2D) {
            fgets(line, 100, fp);
            line[strcspn(line, "\n")] = 0;
            parseLine(cmd, line);
            int numFilter = cmd[1];
            int size = cmd[2];
            int numChannel = cmd[4];

            int weightsShape[4] = {numFilter, size, size, numChannel};

            collect(fp, cmd, weights, biases);
            // print4(weights, weightsShape);

            if (layerCmd[1] == 1) {
                fgets(line, 100, fp);
                line[strcspn(line, "\n")] = 0;
                parseLine(cmd, line);
                collect(fp, cmd, weights, biases);
                int biasSize = cmd[1];
                // print1(biases, 32);
            }

            if (pingpong) {
                if (layerCmd[3] == RELU6)
                    outputShape0 = conv2d_depthwise(output0, output1, outputShape1, weights, weightsShape, biases, stride, relu6);
                else
                    outputShape0 = conv2d_depthwise(output0, output1, outputShape1, weights, weightsShape, biases, stride);

                if (layerCmd[4] == 1) {
                    copy3(mem, output0, outputShape0);
                    copy1(memShape, outputShape0, 3);
                }

                delete3(output1, outputShape1);
            } else {
                if (layerCmd[3] == RELU6)
                    outputShape1 = conv2d_depthwise(output1, output0, outputShape0, weights, weightsShape, biases, stride, relu6);
                else
                    outputShape1 = conv2d_depthwise(output1, output0, outputShape0, weights, weightsShape, biases, stride);

                if (layerCmd[4] == 1) {
                    copy3(mem, output0, outputShape0);
                    copy1(memShape, outputShape0, 3);
                }

                delete3(output0, outputShape0);
            }

            // print3(output, outputShape);

            delete[] biases;
            delete4(weights, weightsShape);
        }

        if (layerCmd[0] == ADD) {
            if (pingpong) {
                outputShape0 = add(output0, mem, output1, memShape);
            } else {
                outputShape1 = add(output1, mem, output0, memShape);
            }
        }

        if (layerCmd[3] == RELU6) {
            cout << "|           ReLU6          |" << endl;
            cout << "============================" << endl;
        }

        pingpong = !pingpong;
        cout << endl;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Conv2d - 1
    ///////////////////////////////////////////////////////////////////////////
    // fgets(line, 100, fp);
    // line[strcspn(line, "\n")] = 0;
    // parseLine(cmd, line);
    // int numFilter = cmd[1];
    // int size = cmd[2];
    // int numChannel = cmd[4];

    // int weightsShape[4] = {numFilter, size, size, numChannel};
    // collect(fp, cmd, weights, biases);
    // // print4(weights, weightsShape);

    // fgets(line, 100, fp);
    // line[strcspn(line, "\n")] = 0;
    // parseLine(cmd, line);
    // collect(fp, cmd, weights, biases);
    // int biasSize = cmd[1];
    // // print1(biases, 32);

    // int ***output0;
    // int *outputShape0 = conv2d(output0, input, inputShape, weights, weightsShape, biases, 2, relu6);
    // // print3(output, outputShape);

    // delete[] biases;
    // delete4(weights, weightsShape);

    // ///////////////////////////////////////////////////////////////////////////
    // // Depthwise Conv2d - 2
    // ///////////////////////////////////////////////////////////////////////////
    // fgets(line, 100, fp);
    // line[strcspn(line, "\n")] = 0;
    // cout << line << endl;
    // parseLine(cmd, line);
    // numFilter = cmd[1];
    // size = cmd[2];
    // numChannel = cmd[4];

    // weightsShape[0] = numFilter;
    // weightsShape[1] = size;
    // weightsShape[2] = size;
    // weightsShape[3] = numChannel;
    // collect(fp, cmd, weights, biases);
    // // print4(weights, weightsShape);

    // fgets(line, 100, fp);
    // line[strcspn(line, "\n")] = 0;
    // parseLine(cmd, line);
    // collect(fp, cmd, weights, biases);
    // biasSize = cmd[1];
    // // print1(biases, 32);

    // int ***output1;
    // int *outputShape1 = conv2d_depthwise(output1, output0, outputShape0, weights, weightsShape, biases, 1, relu6);
    // // print3(output, outputShape);

    // delete[] biases;
    // delete4(weights, weightsShape);

    // ///////////////////////////////////////////////////////////////////////////
    // // Conv2d - 3
    // ///////////////////////////////////////////////////////////////////////////
    // fgets(line, 100, fp);
    // line[strcspn(line, "\n")] = 0;
    // cout << line << endl;
    // parseLine(cmd, line);
    // numFilter = cmd[1];
    // size = cmd[2];
    // numChannel = cmd[4];

    // weightsShape[0] = numFilter;
    // weightsShape[1] = size;
    // weightsShape[2] = size;
    // weightsShape[3] = numChannel;
    // collect(fp, cmd, weights, biases);
    // // print4(weights, weightsShape);

    // fgets(line, 100, fp);
    // line[strcspn(line, "\n")] = 0;
    // parseLine(cmd, line);
    // collect(fp, cmd, weights, biases);
    // biasSize = cmd[1];
    // // print1(biases, 32);

    // delete3(output0, outputShape0);
    // delete[] outputShape0;

    // outputShape0 = conv2d(output0, output1, outputShape1, weights, weightsShape, biases, 1);
    // // print3(output, outputShape);

    // delete[] biases;
    // delete4(weights, weightsShape);

    ///////////////////////////////////////////////////////////////////////////
    FILE *fConvOut;
    fConvOut = fopen("out_conv1.txt", "w+");
    for (int i = 0; i < outputShape1[2]; i++) {
        for (int j = 0; j < outputShape1[0]; j++) {
            for (int k = 0; k < outputShape1[1]; k++) {
                fputs((to_string(output1[i][j][k]) + " ").c_str(), fConvOut);
            }
            fputs("\n", fConvOut);
        }
        fputs("\n", fConvOut);
    }
    fclose(fConvOut);

    fclose(fp);

    return 0;
}

void parseLine(int *output, string line) {
    cout << "> " << line << endl;

    string delimiter = " ";
    size_t pos = 0;

    int paramCount = 0;
    while (pos != string::npos) {
        const char *word = popWord(line, delimiter, pos);

        if (strcmp(word, "conv2d") == 0)
            output[paramCount] = CONV2D;
        else if (strcmp(word, "depthwiseconv2d") == 0)
            output[paramCount] = DEPTHWISE_CON2D;
        else if (strcmp(word, "add") == 0)
            output[paramCount] = ADD;
        else if (strcmp(word, "none") == 0)
            output[paramCount] = NONE;
        else if (strcmp(word, "relu6") == 0)
            output[paramCount] = RELU6;
        else if (strcmp(word, "weight") == 0)
            output[paramCount] = WEIGHT;
        else if (strcmp(word, "bias") == 0)
            output[paramCount] = BIAS;
        else
            output[paramCount] = charTOint(word);
        paramCount += 1;
        delete[] word;
    }
}

int *collect(FILE *fp, int *cmd, int ****&weights, int *&biases) {
    if (cmd[0] == WEIGHT) {
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

    if (cmd[0] == BIAS) {
        int numBias = cmd[1];

        biases = new int[numBias];

        char lineRaw[800];
        fgets(lineRaw, 800, fp);
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