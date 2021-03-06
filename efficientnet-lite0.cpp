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
    AVERAGE_POOL2D,
    FULLYCONNECTED,
    SOFTMAX,
    RELU6,
    NONE,
    WEIGHT,
    BIAS,
    RESHAPE
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
    int layerCount = 1;

    int ***output0;
    int *outputShape0;
    int ***output1;
    int *outputShape1;
    int ***mem;
    int *memShape;
    int **output2d0;
    int *output2dShape0;
    int **output2d1;
    int *output2dShape1;
    double *finalOutput;
    int finalOutputSize;

    // First
    output1 = input;
    outputShape1 = inputShape;

    while (fgets(line, 100, fp)) {
        cout << "# " << layerCount << endl;
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
                if (layerCmd[3] == RELU6) {
                    outputShape1 = conv2d_depthwise(output1, output0, outputShape0, weights, weightsShape, biases, stride, relu6);
                } else {
                    outputShape1 = conv2d_depthwise(output1, output0, outputShape0, weights, weightsShape, biases, stride);
                }

                if (layerCmd[4] == 1) {
                    copy3(mem, output0, outputShape0);
                    copy1(memShape, outputShape0, 3);
                }

                delete3(output0, outputShape0);
            }

            delete[] biases;
            delete4(weights, weightsShape);
        }

        if (layerCmd[0] == ADD) {
            cout << "============================" << endl;
            cout << "|            Add           |" << endl;
            cout << "----------------------------" << endl;
            cout << "  Input 1 shape: " << memShape[0] << "x" << memShape[1] << "x" << memShape[2] << endl;

            if (pingpong) {
                outputShape0 = matAdd(output0, mem, output1, memShape);
                cout << "  Input 2 shape: " << outputShape1[0] << "x" << outputShape1[1] << "x" << outputShape1[2] << endl;
            } else {
                outputShape1 = matAdd(output1, mem, output0, memShape);
                cout << "  Input 2 shape: " << outputShape0[0] << "x" << outputShape0[1] << "x" << outputShape0[2] << endl;
            }

            cout << "============================" << endl;

            delete3(mem, memShape);
            delete[] memShape;

            if (layerCmd[1] == 1) {
                if (pingpong) {
                    copy3(mem, output0, outputShape0);
                    copy1(memShape, outputShape0, 3);
                } else {
                    if (layerCmd[1] == 1) {
                        copy3(mem, output1, outputShape1);
                        copy1(memShape, outputShape1, 3);
                    }
                }
            }
        }

        if (layerCmd[3] == RELU6) {
            cout << "|           ReLU6          |" << endl;
            cout << "============================" << endl;
        }

        if (layerCmd[0] == AVERAGE_POOL2D) {
            cout << "============================" << endl;
            cout << "|      Average Pool2D      |" << endl;
            cout << "----------------------------" << endl;

            int filterShape[2] = {layerCmd[1], layerCmd[2]};

            if (pingpong) {
                outputShape0 = pool2dAverage(output0, output1, outputShape1, filterShape, layerCmd[3]);
                cout << "  Input shape : " << outputShape1[0] << "x" << outputShape1[1] << "x" << outputShape1[2] << endl;
                cout << "  Filter shape: " << filterShape[0] << "x" << filterShape[1] << endl;
                cout << "  Output shape: " << outputShape0[0] << "x" << outputShape0[1] << "x" << outputShape0[2] << endl;
            } else {
                outputShape1 = pool2dAverage(output1, output0, outputShape0, filterShape, layerCmd[3]);
                cout << "  Input shape : " << outputShape0[0] << "x" << outputShape0[1] << "x" << outputShape0[2] << endl;
                cout << "  Filter shape: " << filterShape[0] << "x" << filterShape[1] << endl;
                cout << "  Output shape: " << outputShape1[0] << "x" << outputShape1[1] << "x" << outputShape1[2] << endl;
            }

            cout << "============================" << endl;
        }

        if (layerCmd[0] == RESHAPE) {
            cout << "============================" << endl;
            cout << "|          Reshape         |" << endl;
            cout << "----------------------------" << endl;

            if (pingpong) {
                output2dShape0 = reshapey1xtxy(output2d0, output1, outputShape1);
                cout << "  Input shape : " << outputShape1[0] << "x" << outputShape1[1] << "x" << outputShape1[2] << endl;
                cout << "  Output shape: " << output2dShape0[0] << "x" << output2dShape0[1] << endl;

            } else {
                output2dShape1 = reshapey1xtxy(output2d1, output0, outputShape0);
                cout << "  Input shape : " << outputShape0[0] << "x" << outputShape0[1] << "x" << outputShape0[2] << endl;
                cout << "  Output shape: " << output2dShape1[0] << "x" << output2dShape1[1] << endl;
            }

            cout << "============================" << endl;
        }

        if (layerCmd[0] == FULLYCONNECTED) {
            fgets(line, 100, fp);
            line[strcspn(line, "\n")] = 0;
            parseLine(cmd, line);

            int weightShape[2] = {cmd[1], cmd[2]};
            int **weight;
            createPointer2(weight, weightShape);
            char lineRaw[4000];
            int idxRow = 0, idxCol = 0;
            for (int idx = 0; idx < weightShape[0]; idx += 1) {
                fgets(lineRaw, 4000, fp);
                lineRaw[strcspn(lineRaw, "\n")] = 0;
                string line = string(lineRaw);
                size_t pos = 0;

                int value;
                while (line != "") {
                    const char *word = popWord(line, " ", pos);
                    value = charTOint(word);

                    weight[idxRow][idxCol] = value;

                    idxCol += 1;
                    if (idxCol == weightShape[1]) {
                        idxCol = 0;
                        idxRow += 1;
                    }
                }
            }

            fgets(line, 100, fp);
            line[strcspn(line, "\n")] = 0;
            parseLine(cmd, line);
            collect(fp, cmd, weights, biases);
            int biasSize = cmd[1];

            cout << "============================" << endl;
            cout << "|      Fully connected     |" << endl;
            cout << "----------------------------" << endl;

            if (pingpong) {
                cout << "  Input shape : " << output2dShape1[0] << "x" << output2dShape1[1] << endl;
                cout << "  Filter shape : " << weightShape[0] << "x" << weightShape[1] << endl;
                output2dShape0 = new int[2]{output2dShape1[0], weightShape[1]};
                createPointer2(output2d0, output2dShape0);
                matMul(output2d0, output2d1, output2dShape1, weight, weightShape, biases);
                cout << "  Output shape : " << output2dShape0[0] << "x" << output2dShape0[1] << endl;
            } else {
                cout << "  Input shape : " << output2dShape0[0] << "x" << output2dShape0[1] << endl;
                cout << "  Filter shape : " << weightShape[0] << "x" << weightShape[1] << endl;
                output2dShape1 = new int[2]{output2dShape0[0], weightShape[1]};
                createPointer2(output2d1, output2dShape1);
                matMul(output2d1, output2d0, output2dShape0, weight, weightShape, biases);
                cout << "  Output shape : " << output2dShape1[0] << "x" << output2dShape1[1] << endl;
            }

            if (layerCmd[1] == SOFTMAX) {
                cout << "----------------------------" << endl;
                cout << "|          Softmax         |" << endl;
                cout << "============================" << endl;
                if (pingpong) {
                    finalOutputSize = output2dShape0[1];
                    finalOutput = softmax(output2d0[0], finalOutputSize);
                } else {
                    finalOutputSize = output2dShape1[1];
                    finalOutput = softmax(output2d1[0], finalOutputSize);
                }
            }

            delete2(weight, weightShape);
        }

        pingpong = !pingpong;
        layerCount++;
        cout << endl;
    }

    int top = finalOutput[0];
    int topIdx = 0;
    for (int idx = 0; idx < finalOutputSize; idx += 1) {
        if (finalOutput[idx] > top) {
            top = finalOutput[idx];
            topIdx = idx;
        }
    }

    cout << "Top: " << topIdx << " : " << top << " %" << endl;

    ///////////////////////////////////////////////////////////////////////////
    FILE *fConvOut;
    fConvOut = fopen("out_conv1.txt", "w+");
    for (int i = 0; i < outputShape0[2]; i++) {
        for (int j = 0; j < outputShape0[0]; j++) {
            for (int k = 0; k < outputShape0[1]; k++) {
                fputs((to_string(output0[i][j][k]) + " ").c_str(), fConvOut);
            }
            fputs("\n", fConvOut);
        }
        fputs("\n", fConvOut);
    }
    fclose(fConvOut);

    FILE *foutput;
    foutput = fopen("out_final.txt", "w+");
    for (int i = 0; i < finalOutputSize; i++) {
        fputs((to_string(finalOutput[i]) + " ").c_str(), foutput);
    }
    fclose(foutput);

    fclose(fp);

    delete3(output0, outputShape0);
    delete3(output1, outputShape1);
    delete3(mem, memShape);
    delete[] outputShape0;
    delete[] outputShape1;
    delete[] memShape;
    delete2(output2d0, output2dShape0);
    delete[] output2dShape0;
    delete2(output2d1, output2dShape1);
    delete[] output2dShape1;
    delete[] finalOutput;

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
        else if (strcmp(word, "averagepool2d") == 0)
            output[paramCount] = AVERAGE_POOL2D;
        else if (strcmp(word, "fullyconnected") == 0)
            output[paramCount] = FULLYCONNECTED;
        else if (strcmp(word, "softmax") == 0)
            output[paramCount] = SOFTMAX;
        else if (strcmp(word, "relu6") == 0)
            output[paramCount] = RELU6;
        else if (strcmp(word, "weight") == 0)
            output[paramCount] = WEIGHT;
        else if (strcmp(word, "bias") == 0)
            output[paramCount] = BIAS;
        else if (strcmp(word, "reshape") == 0)
            output[paramCount] = RESHAPE;
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
            size_t pos = 0;

            int value;
            while (line != "") {
                const char *word = popWord(line, " ", pos);

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
                delete[] word;
            }
        }
    }

    if (cmd[0] == BIAS) {
        int numBias = cmd[1];

        biases = new int[numBias];

        char lineRaw[9000];
        fgets(lineRaw, 9000, fp);
        lineRaw[strcspn(lineRaw, "\n")] = 0;
        string line = string(lineRaw);
        size_t pos = 0;

        int idx = 0;
        while (line != "") {
            const char *word = popWord(line, " ", pos);
            biases[idx] = charTOint(word);
            idx += 1;
        }
    }

    for (int idx = 0; idx < 5; idx++)
        cmd[idx] = 0;
}