#include <iostream>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <bits/stdc++.h>

pthread_mutex_t locks[10];
pthread_mutex_t lock;

int startTime = time(0);

void *fun(void *arg) {
    int *x = (int *)arg;
    printf("\ninit from %d", *x);
    // for(int i = 0; i < 10; ++i) {
        //printf("\ninit from %d", *x);
        // if(*x == i+1) {
            pthread_mutex_lock(&(locks[(*x) % 10]));
            sleep(2);
            printf("\n(%ld) hello from %d", time(0) - startTime, *x);
            pthread_mutex_unlock(&(locks[(*x) % 10]));
            
        // }
    // }
    return NULL;
}
int main() {

    pthread_t threads[40];
    pthread_mutex_init(&lock, NULL);

    int x[40];
    for(int i = 0; i < 40; ++i) {
        x[i] = i+1;
        if(i < 10)
            pthread_mutex_init(&(locks[i]), NULL);
        pthread_create(&threads[i], NULL, fun, &x[i]);
    }

    for(int  i = 0; i < 40; ++i) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}