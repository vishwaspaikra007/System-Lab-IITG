#include <iostream>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <bits/stdc++.h>

#define MAX_PRIORITY 20
#define RUN_TIME 20 // seconds

#define MAX_RESOURCES 20
#define MAX_RESOURCES_FOR_REQUEST 15
#define MIN_RESOURCES_FOR_REQUEST 5
#define MIN_RESOURCES_FOR_THREADS 5
#define EXPECTED_RESOURCES_FOR_THREADS thread[i].priority * 1

#define SERVER_SLEEP_TIME (rand() % 2) + 1
#define THREAD_SLEEP_TIME (rand() % 2) + 1
#define SCHEDULER_SLEEP_TIME 1 
#define REQUEST_GENERATOR_SLEEP_TIME 1


using namespace std;

pthread_mutex_t lock_server_index;

int startTime = time(0);

struct Service {
    int totalWorkerThreads; // number of worker threads
    int priority;
    int resources;

    Service(int m) {

    }
};

// void *serverFunction(void *arg) {
//     int *obj = (int *)arg;

//     Service service[*arg];
// }

struct ServiceRequest {
    int id;
    int type;
    int resources;
};

struct Request {
    int id;
    int resources;
};

struct Threads {
    int id;
    int serverId;
    // Schedule *obj;
    // vector<Request> *sharedTable;
    pthread_t thread;
    vector<Request> requests;
    int priority;
    int totalResources;
    int occupiedResources ;
};

void *callStartRequestGenerators(void *arg);
void *callStartRequestScheduler(void *arg);
void *callStartServer(void *arg);
void *callStartThread(void *arg);

class Schedule {
    private:
        int numberOfServers; // number of services
        int totalWorkerThreadsForEachServer; // worker threads of each service
        queue<ServiceRequest> requestQueue;
        vector<queue<Request>> queueOfService;

        vector<vector<Threads>> threads;
    public:
        Schedule(int _n, int _m): numberOfServers(_n), totalWorkerThreadsForEachServer(_m) {
            printf("\nnumber of services : %d",numberOfServers);
            printf("\nnumber of worker threads for each service : %d",totalWorkerThreadsForEachServer);
            queueOfService.resize(numberOfServers);
        }

        void startRequestGenerators() {
            int requestId = 0;
            while(time(0) - startTime < RUN_TIME) {
                int requestType = rand() % numberOfServers;
                int requestResources = rand() % MAX_RESOURCES_FOR_REQUEST + MIN_RESOURCES_FOR_REQUEST;

                requestQueue.push({++requestId, requestType, requestResources});
                
                sleep(REQUEST_GENERATOR_SLEEP_TIME);
            }
        }

        void startRequestScheduler() {
            while(time(0) - startTime < RUN_TIME || !requestQueue.empty()) {

                if(!requestQueue.empty()) {
                    ServiceRequest request = requestQueue.front();
                    requestQueue.pop();
                    //printf("\nrequest id: %d | request type: %d | request resources: %d", request.id, request.type, request.resources);
                    queueOfService[request.type].push({request.id, request.resources});
                }
                sleep(SCHEDULER_SLEEP_TIME);
            }
        }

        void startThreads() {

        }

        void startServer() {
            static int serverId = -1;

            pthread_mutex_lock(&lock_server_index);
            int id = ++serverId;
            pthread_mutex_unlock(&lock_server_index);

            // vector<Request> sharedTable(totalWorkerThreadsForEachServer);

            // vector<pthread_t> thread(totalWorkerThreadsForEachServer);
            // vector<Threads> threadData(totalWorkerThreadsForEachServer);

            // for(int i = 0; i < numberOfServers; ++i) {
            //     thread[i].id = i;
            //     thread[i].serverId = id;
            //     thread[i].obj = this;
            //     thread[i].shared_table = shared_table;
            //     thread[i].priority = i;
            //     // thread[i].priority = rand() % MAX_PRIORITY + 1;
            //     thread[i].totalResources = EXPECTED_RESOURCES + MIN_RESOURCES;
            //     thread[i].occupiedResources = 0;
            //     pthread_create(&thread[i], NULL, callStartThreads, &threadData[i]); 
            // } 

            // for(int i = 0; i < numberOfServers; ++i) {
            //     pthread_join(thread[i], NULL);
            // } 


            vector<Threads> thread(totalWorkerThreadsForEachServer);
            // priority_queue<Threads, [&](Thread A, Thread B){
            //     return A.priority > B.priority;
            // }> pq;

            for(int i = 0; i < totalWorkerThreadsForEachServer; ++i) {
                thread[i].id = i;
                thread[i].serverId = id;
                thread[i].priority = rand() % MAX_PRIORITY + 1;
                thread[i].totalResources = EXPECTED_RESOURCES_FOR_THREADS + MIN_RESOURCES_FOR_THREADS;
                thread[i].occupiedResources = 0;
            }

            sort(begin(thread), end(thread), [&](Threads A, Threads B){
                return A.priority > B.priority;
            });

            printf("\ninitailzed server %d", id);

            while(time(0) - startTime < RUN_TIME || !queueOfService[id].empty()) {
                int index = 0;
                while(!queueOfService[id].empty() && index < totalWorkerThreadsForEachServer) {
                    Request request = queueOfService[id].front();
                    printf("\n(%ld) request id: %d | request type: %d | request resources: %d", time(0) - startTime, request.id, id, request.resources);
                    if(thread[index].totalResources < request.resources) {
                        queueOfService[id].pop();
                    }
                    else if(thread[index].totalResources - thread[index].occupiedResources >= request.resources) {
                        thread[index].occupiedResources += request.resources;
                        thread[index].requests.push_back({request.id, request.resources});
                        queueOfService[id].pop();
                    } else ++index;
                }

                // thread creation is consuming too much time
                for(int i = 0 ; i < index; ++i) {
                    pthread_create(&thread[i].thread, NULL, callStartThread, &thread[i]); 
                }
                for(int i = 0; i < index; ++i) {
                    pthread_join(thread[i].thread, NULL);
                } 
                // printf("server %d is sleeping", id);
                //sleep(SERVER_SLEEP_TIME);
            }
        }

        void start() {
            pthread_t requestThread, schedulerThread;
            pthread_t server[numberOfServers];

            pthread_create(&requestThread, NULL, callStartRequestGenerators, this); 
            pthread_create(&schedulerThread, NULL, callStartRequestScheduler, this);

            for(int i = 0; i < numberOfServers; ++i) {
                pthread_create(&server[i], NULL, callStartServer, this); 
            } 

            for(int i = 0; i < numberOfServers; ++i) {
                pthread_join(server[i], NULL);
            } 

            pthread_join(requestThread, NULL);
            pthread_join(schedulerThread, NULL);
        }
};

void *callStartRequestGenerators(void *arg) {
    Schedule *obj = (Schedule *)arg;
    obj->startRequestGenerators();
    return NULL;
}
void *callStartRequestScheduler(void *arg) {
    Schedule *obj = (Schedule *)arg;
    obj->startRequestScheduler();
    return NULL;
}

void *callStartServer(void *arg) {
    Schedule *obj = (Schedule *)arg;
    obj->startServer();
    return NULL;
}

void *callStartThread(void *arg) {
    Threads *thread = (Threads *)arg;
    printf("\nthread working %d and request size %ld", thread->id, thread->requests.size());
    for(int i = 0; i < thread->requests.size(); ++i) {
        printf("\n(%ld) server id: %d | thread id: %d | thread priority: %d | thread resources: %d | request id: %d | request resources: %d", time(0) - startTime, thread->serverId, thread->id, thread->priority, thread->totalResources, thread->requests[i].id, thread->requests[i].resources);
    }

    sleep(THREAD_SLEEP_TIME);

    return NULL;
}

int main() {
    srand(time(0));

    if (pthread_mutex_init(&lock_server_index, NULL) != 0) {
        printf("\n mutex lock_process_index init has failed\n");
        return 1;
    }

    int n, m;
    scanf("%d%d", &n, &m);

    Schedule s_obj(n, m);
    s_obj.start();

    return 0;
}