#include <iostream>

#include "conv.h"

using namespace std;

int main() {
    cout << "Efficientnet-lite0 int8" << endl;

    // int ts[3][4][4] = {
    //     {{1, 2, 3, 4},
    //      {5, 6, 7, 8},
    //      {9, 10, 11, 12},
    //      {13, 14, 15, 16}},
    //     {{1, 2, 3, 4},
    //      {5, 6, 7, 8},
    //      {9, 10, 11, 12},
    //      {13, 14, 15, 16}},
    //     {{1, 2, 3, 4},
    //      {5, 6, 7, 8},
    //      {9, 10, 11, 12},
    //      {13, 14, 15, 16}},
    // };

    // cout << sizeof(ts[0]) << endl;
    // cout << sizeof(*ts[0]) << endl;
    // cout << sizeof(ts[0]) / sizeof(*ts[0]) << endl;

    // cout << sizeof(int) << endl;

    conv2d_4x2x2();

    return 0;
}