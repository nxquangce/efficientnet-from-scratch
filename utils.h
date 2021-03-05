#include <string>
using namespace std;
int charTOint(const char *c);
const char *popWord(string &line, string delimiter, size_t &pos);
void print1(int *pointer, int size);
void print3(int ***pointer, int shape[3]);
void print4(int ****pointer, int shape[4]);
void createPointer2(int **&pointer, int size[2]);
void createPointer3(int ***&pointer, int size[3]);
void createPointer4(int ****&pointer, int size[4]);
void delete2(int **pointer, int size[2]);
void delete3(int ***&pointer, int size[3]);
void delete4(int ****&pointer, int size[4]);
void copy1(int *&output, int *input, int size);
void copy3(int ***&output, int ***input, int shape[3]);
int *reshapey1xtxy(int **&output, int ***input, int inputShape[3]);