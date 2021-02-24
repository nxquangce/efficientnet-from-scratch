using namespace std;

int relu6(int value) {
    if (value < 0) return 0;
    if (value > 6) return 6;
    return value;
}