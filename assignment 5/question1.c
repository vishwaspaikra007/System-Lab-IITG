#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int MAX_DINERS; // Maximum number of diners
int BATCH_SIZE; // Number of diners in each batch

pthread_mutex_t front_door_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for front door
pthread_mutex_t back_door_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for back door
pthread_mutex_t service_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for service
pthread_cond_t front_door_cond = PTHREAD_COND_INITIALIZER; // Condition variable for front door
pthread_cond_t back_door_cond = PTHREAD_COND_INITIALIZER; // Condition variable for back door
pthread_cond_t service_cond = PTHREAD_COND_INITIALIZER; // Condition variable for service

int num_diners_inside = 0; // Number of diners inside the restaurant
int num_diners_left; // Number of diners not entered from total diners

void* diner_thread(void*);
void* restaurant_thread(void*);

void* diner_thread(void* arg) // here diner threads will wait and enter restaurant to get served
{
    int id = *(int*) arg;
    
    printf("Diner %d is waiting outside the restaurant.\n", id);
    
    
    pthread_mutex_lock(&front_door_mutex); // front door lock
    while (num_diners_inside >= BATCH_SIZE) { //if batch is filled waint until front door is opened
        pthread_cond_wait(&front_door_cond, &front_door_mutex);
    }
    num_diners_inside++;
    num_diners_left--;
    pthread_mutex_unlock(&front_door_mutex);
    
    printf("Diner %d has entered the restaurant.\n", id);
    
    // Wait for all diners to arrive before starting service
    if (num_diners_inside == BATCH_SIZE || num_diners_left == 0 ) {
        //leep(2);
        //sleep(0);
        pthread_cond_broadcast(&service_cond);
    }

    if(num_diners_left == 0) //last diner in case number of diners is not divisible by batch size 
    {
        pthread_mutex_lock(&service_mutex);
        printf("Diner %d eating...\n", id);
        sleep(1);
        pthread_mutex_unlock(&service_mutex);
    }
    else
    {
        if(num_diners_inside != BATCH_SIZE) // other diners in the batch waiting for service when batch size is filled  
        {   
            
            pthread_mutex_lock(&service_mutex);
            pthread_cond_wait(&service_cond, &service_mutex);
            printf("Diner %d eating...\n", id);
            sleep(1);
            pthread_mutex_unlock(&service_mutex);
        }
        else    // for the last diner of each batch 
        {
            pthread_mutex_lock(&service_mutex);
            printf("Diner %d eating...\n", id);
            sleep(1);
            pthread_mutex_unlock(&service_mutex);
        }
    }
    // Wait for the back door to open before leaving
    pthread_mutex_lock(&back_door_mutex);
    num_diners_inside--;

    printf("Diner %d has left the restaurant.\n", id);
    printf("no of diners left inside = %d\n", num_diners_inside);
    // Signal the back door to open if all diners have left
    if (num_diners_inside == 0) {
        printf("All diners have left. The back door is closed.\n**********************\n");
        pthread_cond_broadcast(&front_door_cond);
    }
    pthread_mutex_unlock(&back_door_mutex);
    pthread_exit(NULL);
}

void* restaurant_thread(void* arg) //function to create all diner threads and then join them
{
    int num_diners = *(int*) arg;
    pthread_t diner_threads[MAX_DINERS];
    int diner_ids[MAX_DINERS];
    num_diners_left = MAX_DINERS;
    // Create diner threads
    for (int i = 0; i < num_diners; i++) {
        diner_ids[i] = i + 1;
        if (pthread_create(&diner_threads[i], NULL, diner_thread, &diner_ids[i]) != 0) {
            perror("Error creating diner thread");
            exit(EXIT_FAILURE);
        }
    }
    
    // Wait for diner threads to complete
    for (int i = 0; i < num_diners; i++) {
        if (pthread_join(diner_threads[i], NULL) != 0) {
            perror("Error joining diner thread");
            exit(EXIT_FAILURE);
        }
    }
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    pthread_t restaurant_thread_id;
    int num_diners;
    MAX_DINERS = atoi(argv[1]);
    BATCH_SIZE = atoi(argv[2]);
    num_diners = MAX_DINERS;
    
    // Create restaurant thread
    if (pthread_create(&restaurant_thread_id, NULL, restaurant_thread, &num_diners) != 0) {
        perror("Error creating restaurant thread");
        exit(EXIT_FAILURE);
    }
    
    // Wait for restaurant thread to complete
    if (pthread_join(restaurant_thread_id, NULL) != 0) {
        perror("Error joining threads");
        exit(EXIT_FAILURE);
    }
    return 0;
}