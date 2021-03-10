#include <stdio.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "activation.h"
#include "conv.h"
#include "utils.h"

using namespace std;
using namespace cv;

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

void parseLine(int *output, string line, double quantizeParams[2]);
int *collect(FILE *fp, int *cmd, int8_t ****&weights, int8_t *&biases);

int main() {
    cout << "Efficientnet-lite0 int8" << endl;

    FILE *fp;
    fp = fopen("weights.txt", "r");
    if (fp == NULL) perror("Error opening file");

    Mat inImg = imread("input.jpg");

    resize(inImg, inImg, Size(224, 224), 0, 0);
    imshow("Resized", inImg);

    cout << " Created input" << endl;
    int inputShape[3] = {224, 224, 3};
    int8_t ***input;
    createPointer3(input, inputShape);

    for (int i = 0; i < inImg.rows; i++) {
        for (int j = 0; j < inImg.cols; j++) {
            // 0.012566016986966133 * (q - 131)
            input[0][i][j] = 0.012566016986966133 * (0.012566016986966133 * (inImg.at<Vec3b>(i, j)[0] - 131) - 3);  // BRG
            input[1][i][j] = 0.012566016986966133 * (0.012566016986966133 * (inImg.at<Vec3b>(i, j)[1] - 131) - 3);  // BRG
            input[2][i][j] = 0.012566016986966133 * (0.012566016986966133 * (inImg.at<Vec3b>(i, j)[2] - 131) - 3);  // BRG
        }
    }

    char line[100];
    int cmd[5] = {};
    int8_t ****weights;
    int8_t *biases;
    double *quantizeParams = new double[2];

    bool pingpong = true;
    int layerCount = 1;

    int8_t ***output0;
    int *outputShape0;
    int8_t ***output1;
    int *outputShape1;
    int8_t ***mem;
    int *memShape;
    int8_t **output2d0;
    int *output2dShape0;
    int8_t **output2d1;
    int *output2dShape1;
    double *finalOutput;
    int finalOutputSize;

    // First
    output1 = input;
    outputShape1 = inputShape;

    auto start = chrono::high_resolution_clock::now();
    while (fgets(line, 100, fp)) {
        cout << "# " << layerCount << endl;
        // fgets(line, 100, fp);
        line[strcspn(line, "\n")] = 0;
        parseLine(cmd, line, quantizeParams);

        int layerCmd[5] = {cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]};
        int stride = layerCmd[2];

        if (layerCmd[0] == CONV2D) {
            fgets(line, 100, fp);
            line[strcspn(line, "\n")] = 0;
            parseLine(cmd, line, quantizeParams);
            int numFilter = cmd[1];
            int size = cmd[2];
            int numChannel = cmd[4];

            int weightsShape[4] = {numFilter, size, size, numChannel};
            collect(fp, cmd, weights, biases);
            // print4(weights, weightsShape);

            if (layerCmd[1] == 1) {
                fgets(line, 100, fp);
                line[strcspn(line, "\n")] = 0;
                parseLine(cmd, line, quantizeParams);
                collect(fp, cmd, weights, biases);
                int biasSize = cmd[1];
                // print1(biases, 32);
            }

            if (pingpong) {
                if (layerCmd[3] == RELU6)
                    outputShape0 = conv2d(output0, output1, outputShape1, weights, weightsShape, biases, stride, relu6);
                else {
                    outputShape0 = conv2d(output0, output1, outputShape1, weights, weightsShape, biases, stride);
                    quantize3(output0, outputShape0, quantizeParams[0], quantizeParams[1]);
                }

                if (layerCmd[4] == 1) {
                    copy3(mem, output0, outputShape0);
                    copy1(memShape, outputShape0, 3);
                }

                delete3(output1, outputShape1);
            } else {
                if (layerCmd[3] == RELU6)
                    outputShape1 = conv2d(output1, output0, outputShape0, weights, weightsShape, biases, stride, relu6);
                else {
                    outputShape1 = conv2d(output1, output0, outputShape0, weights, weightsShape, biases, stride);
                    quantize3(output1, outputShape1, quantizeParams[0], quantizeParams[1]);
                }

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
            parseLine(cmd, line, quantizeParams);
            int numFilter = cmd[1];
            int size = cmd[2];
            int numChannel = cmd[4];

            int weightsShape[4] = {numFilter, size, size, numChannel};

            collect(fp, cmd, weights, biases);
            // print4(weights, weightsShape);

            if (layerCmd[1] == 1) {
                fgets(line, 100, fp);
                line[strcspn(line, "\n")] = 0;
                parseLine(cmd, line, quantizeParams);
                collect(fp, cmd, weights, biases);
                int biasSize = cmd[1];
                // print1(biases, 32);
            }

            if (pingpong) {
                if (layerCmd[3] == RELU6)
                    outputShape0 = conv2d_depthwise(output0, output1, outputShape1, weights, weightsShape, biases, stride, relu6);
                else {
                    outputShape0 = conv2d_depthwise(output0, output1, outputShape1, weights, weightsShape, biases, stride);
                    quantize3(output0, outputShape0, quantizeParams[0], quantizeParams[1]);
                }

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
                    quantize3(output1, outputShape1, quantizeParams[0], quantizeParams[1]);
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
                quantize3(output0, outputShape0, quantizeParams[0], quantizeParams[1]);
                cout << "  Input 2 shape: " << outputShape1[0] << "x" << outputShape1[1] << "x" << outputShape1[2] << endl;
            } else {
                outputShape1 = matAdd(output1, mem, output0, memShape);
                quantize3(output1, outputShape1, quantizeParams[0], quantizeParams[1]);
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
            parseLine(cmd, line, quantizeParams);

            int weightShape[2] = {cmd[1], cmd[2]};
            int8_t **weight;
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
            parseLine(cmd, line, quantizeParams);
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

                quantize2(output2d0, output2dShape0, 0.09174981713294983, -62);
            } else {
                cout << "  Input shape : " << output2dShape0[0] << "x" << output2dShape0[1] << endl;
                cout << "  Filter shape : " << weightShape[0] << "x" << weightShape[1] << endl;
                output2dShape1 = new int[2]{output2dShape0[0], weightShape[1]};
                createPointer2(output2d1, output2dShape1);
                matMul(output2d1, output2d0, output2dShape0, weight, weightShape, biases);
                cout << "  Output shape : " << output2dShape1[0] << "x" << output2dShape1[1] << endl;

                quantize2(output2d1, output2dShape1, 0.09174981713294983, -62);
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

        quantizeParams[0] = 1;
        quantizeParams[1] = 0;
        pingpong = !pingpong;
        layerCount++;
        cout << endl;
    }
    auto stop = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

    cout << "Execution time: " << duration.count() << " microseconds ~ " << (double)(duration.count()/1000.0) << "ms" << endl;

    FILE *fLabel;
    fLabel = fopen("label.txt", "r");
    if (fLabel == NULL) perror("Error opening file");

    string *labels = new string[1000];
    for (int idx = 0; idx < 1000; idx += 1) {
        char lline[200];
        fgets(lline, 200, fLabel);
        lline[strcspn(lline, "\n")] = 0;
        string sline = string(lline);
        size_t start = sline.find(" ") + 2;
        size_t end = sline.length() - start - 2;
        labels[idx] = sline.substr(start, end);
    }

    fclose(fLabel);

    double top = finalOutput[0];
    int topIdx = 0;
    for (int idx = 0; idx < finalOutputSize; idx += 1) {
        if (finalOutput[idx] > top) {
            top = finalOutput[idx];
            topIdx = idx;
        }
    }

    cout << "Top: " << topIdx << " : " << top * 100 << " %" << endl;
    cout << labels[topIdx] << endl;

    for (int idx = 0; idx < 1000; idx += 1) {
        if (finalOutput[idx] == top) {
            cout << idx << " : " << labels[idx] << endl;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    FILE *fOut0;
    fOut0 = fopen("out_0.txt", "w+");
    for (int i = 0; i < outputShape0[2]; i++) {
        for (int j = 0; j < outputShape0[0]; j++) {
            for (int k = 0; k < outputShape0[1]; k++) {
                fputs((to_string((int)output0[i][j][k]) + " ").c_str(), fOut0);
            }
            fputs("\n", fOut0);
        }
        fputs("\n", fOut0);
    }
    fclose(fOut0);

    FILE *fOut1;
    fOut1 = fopen("out_1.txt", "w+");
    for (int i = 0; i < outputShape1[2]; i++) {
        for (int j = 0; j < outputShape1[0]; j++) {
            for (int k = 0; k < outputShape1[1]; k++) {
                fputs((to_string((int)output1[i][j][k]) + " ").c_str(), fOut1);
            }
            fputs("\n", fOut1);
        }
        fputs("\n", fOut1);
    }
    fclose(fOut1);

    FILE *fOut2d0;
    fOut2d0 = fopen("out_2d0.txt", "w+");
    for (int i = 0; i < output2dShape0[0]; i++) {
        for (int j = 0; j < output2dShape0[1]; j++) {
            fputs((to_string((int)output2d0[i][j]) + " ").c_str(), fOut2d0);
        }
        fputs("\n", fOut2d0);
    }
    fclose(fOut2d0);

    FILE *fOut2d1;
    fOut2d1 = fopen("out_2d1.txt", "w+");
    for (int i = 0; i < output2dShape1[0]; i++) {
        for (int j = 0; j < output2dShape1[1]; j++) {
            fputs((to_string((int)output2d1[i][j]) + " ").c_str(), fOut2d1);
        }
        fputs("\n", fOut2d1);
    }
    fclose(fOut2d1);

    FILE *foutput;
    foutput = fopen("out_final.txt", "w+");
    for (int i = 0; i < finalOutputSize; i++) {
        fputs((to_string(finalOutput[i]) + " ").c_str(), foutput);
    }
    fclose(foutput);

    fclose(fp);

    delete3(output0, outputShape0);
    delete3(output1, outputShape1);
    delete[] outputShape0;
    delete[] outputShape1;
    delete2(output2d0, output2dShape0);
    delete[] output2dShape0;
    delete2(output2d1, output2dShape1);
    delete[] output2dShape1;
    delete[] finalOutput;

    return 0;
}

void parseLine(int *output, string line, double *quantizeParams) {
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
        else if (paramCount > 4) {
            quantizeParams[paramCount - 5] = atof(word);
            cout << word << " --> " << quantizeParams[paramCount - 5] << endl;
        } else
            output[paramCount] = charTOint(word);

        paramCount += 1;
        delete[] word;
    }
}

int *collect(FILE *fp, int *cmd, int8_t ****&weights, int8_t *&biases) {
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

        biases = new int8_t[numBias];

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