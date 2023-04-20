#include <iostream>
#include <time.h>

using namespace std;

int main() {
    srand(time(0));
    FILE *f = fopen("thread.txt", "r");

    fscanf(f, "%d %d", &n, &m);
    printf("%d %d\n", n, m)
    for(int j = 0; j < n; ++j) {
        for(int i = 0; i < m; ++i) {
            fscanf(f, "%d %d", &n, &m);
            printf("%d %d\n", n, m);
        }
    }
    for(int i = 0; i < tn; ++i) {
        int p = rand() % n;
        int r = (rand() % maxval) + minval;
        fprintf(f, "%d %d\n", p, r);
    }
}