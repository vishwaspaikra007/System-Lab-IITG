#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <semaphore.h>

sem_t bridge;
sem_t dir;

using namespace std;

#define MAX_THREADS 20

void northBoundPeople(int id){
    this_thread::sleep_for(chrono::milliseconds(rand() % 100)); // waiting before the crossing bridge
    sem_wait(&bridge); // try to aquire semaphore
    this_thread::sleep_for(chrono::milliseconds(rand() % 100)); // crossing bridge

    cout<<"North bound "<<id<<" is trying to cross the bridge.\n";

    sem_wait(&dir); // try to aquire semaphore
    this_thread::sleep_for(chrono::milliseconds(rand() % 100)); // crossed the bridge

    cout<<"North bound "<<id<<" crossed the bridge.\n";

    sem_post(&dir);
    sem_post(&bridge);
}

void southBoundPeople(int id){
    this_thread::sleep_for(chrono::milliseconds(rand() % 100)); // waiting before crossing the bridge
    sem_wait(&dir); // try to aquire semaphore
    this_thread::sleep_for(chrono::milliseconds(rand() % 100)); // crossing the bridge

    cout<<"South bound "<<id<<" is trying to cross the bridge.\n";

    sem_wait(&bridge); // try to aquire semaphore
    this_thread::sleep_for(chrono::milliseconds(rand() % 100)); // crossed the bridge

    cout<<"South bound "<<id<<" crossed the bridge.\n";

    sem_post(&bridge);
    sem_post(&dir);
}


int main(int argc, char **argv){
    // creating threads that represents people crossing the bridge
    thread persons[MAX_THREADS];
    sem_init(&bridge, 0, 1); // initializing bridge semaphore
    int Case = stoi(argv[1]); // getting case number rom command line input

    if(Case == 2)sem_init(&dir, 0, 1); // for case 2 initialize dir semaphore the number of people
    else sem_init(&dir, 0, MAX_THREADS); // else initialize with 1

    for(int i = 0; i< MAX_THREADS; ++i){
        if(i%2 == 0)persons[i] = thread(northBoundPeople, i+1); // creating nothbpund people
        else persons[i] = thread(southBoundPeople, i+1); // creating southbound people
    }

    for(int i = 0; i< MAX_THREADS; ++i){
        persons[i].join(); // joining all the threads
    }

    return 0;
}