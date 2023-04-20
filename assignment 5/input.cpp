#include <iostream>
#include <time.h>

using namespace std;

int main() {
    srand(time(0));
    FILE *f = fopen("thread.txt", "w");
    int n = 5;
    int m = 5;
    int tn = 30;
    int maxval = 20;
    int minval = 5;

    fprintf(f, "%d\n%d\n", n, m);
    for(int j = 0; j < n; ++j) {
        for(int i = 0; i < m; ++i) {
            int p = (rand() % maxval) + minval;
            int r = p + (rand() % maxval);
            fprintf(f, "%d %d\n", p, r);
        }
    }
    fprintf(f, "%d\n", tn);
    for(int i = 0; i < tn; ++i) {
        int p = rand() % n;
        int r = i%6 == 0 ? 100: (rand() % maxval) + minval;
        fprintf(f, "%d %d\n", p, r);
    }
}