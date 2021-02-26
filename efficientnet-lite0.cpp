#include <stdio.h>

#include <cstring>
#include <iostream>

#include "conv.h"

using namespace std;

void parseLine(int *output, string line);
void collect(FILE *fp, int *cmd, int ****&weights, int *&biases);

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
    cout << "Parsed: " << cmd[0] << " " << cmd[1] << " " << cmd[2] << " " << cmd[3] << " " << cmd[4] << endl;
    collect(fp, cmd, weights, biases);
    cout << "Collected" << endl;
    for (int f = 0; f < 32; f++) {
        for (int c = 0; c < 3; c++) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    cout << weights[f][c][i][j] << " ";
                }
                cout << endl;
            }
            cout << endl;
        }
    }

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

void collect(FILE *fp, int *cmd, int ****&weights, int *&biases) {
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

        char *line;
        fgets(line, 100, fp);
        line[strcspn(line, "\n")] = 0;
        string delimiter = " ";
        size_t pos = 0;
        int idx = 0;
        while ((pos = string(line).find(delimiter)) != string::npos) {
            const char *word = string(line).substr(0, pos).c_str();
            string(line).erase(0, pos + delimiter.length());

            biases[idx] = charTOint(word);
            idx += 1;
        }
    }
}