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
#define EXPECTED_RESOURCES_FOR_THREADS workerThreads[i][j].priority * 1

#define SERVER_SLEEP_TIME (rand() % 2) + 1
#define THREAD_SLEEP_TIME (rand() % 2) + 1
#define SCHEDULER_SLEEP_TIME 1 
#define REQUEST_GENERATOR_SLEEP_TIME 1


using namespace std;

pthread_mutex_t lock_server_index;

int startTime = time(0);

void *callStartRequestGenerators(void *arg);
void *callStartRequestScheduler(void *arg);
void *callStartServer(void *arg);
void *callStartThread(void *arg);

struct ServiceRequest {
    int id;
    int type;
    int resources;
};

struct Request {
    int id;
    int resources;
};

struct Thread {
    int id;
    int serverId;
    void *obj;
    pthread_t thread;
    vector<Request> requests;
    int priority;
    int totalResources;
    int occupiedResources ;
};

struct SharedTable {
    int totalResources;
    queue<Request> requests;
};

class Schedule {
    private:
        int numberOfServers; // number of services
        int totalWorkerThreadsForEachServer; // worker threads of each service
        pthread_t requestThread, schedulerThread;

        queue<ServiceRequest> requestQueue;

        vector<pthread_t> serverThreads;
        vector<queue<Request>> queueOfService;

        vector<vector<Thread>> workerThreads;
        vector<vector<SharedTable>> sharedTable;
        vector<vector<pthread_mutex_t>> locks;

    public:
        Schedule(int _n, int _m): numberOfServers(_n), totalWorkerThreadsForEachServer(_m) {
            printf("\nnumber of services : %d",numberOfServers);
            printf("\nnumber of worker threads for each service : %d",totalWorkerThreadsForEachServer);

            serverThreads.resize(numberOfServers);
            queueOfService.resize(numberOfServers);

            workerThreads.resize(numberOfServers,vector<Thread>(totalWorkerThreadsForEachServer));
            sharedTable.resize(numberOfServers,vector<SharedTable>(totalWorkerThreadsForEachServer));
            locks.resize(numberOfServers, vector<pthread_mutex_t>(totalWorkerThreadsForEachServer));

            printf("\nqueueofservice %ld ", queueOfService.size());
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
                    printf("\nrequest id: %d | request type: %d | request resources: %d", request.id, request.type, request.resources);
                    queueOfService[request.type].push({request.id, request.resources});
                }
                sleep(SCHEDULER_SLEEP_TIME);
            }
        }

        void updateResources(int i, int j, int val) {
            pthread_mutex_lock(&(locks[i][j]));
            sharedTable[i][j].totalResources += val;
            pthread_mutex_unlock(&(locks[i][j]));
        }

        void startServer() {
            Request request;
            static int serverId = -1;
            pthread_mutex_lock(&lock_server_index);
            int index = ++serverId;
            pthread_mutex_unlock(&lock_server_index);

            printf("\nserver : %d", index);
            
            while(time(0) - startTime < RUN_TIME || !queueOfService[index].empty()) {
                if(!queueOfService[index].empty()) {
                    request = queueOfService[index].front();
                    queueOfService[index].pop();

                    int i;
                    for(i = 0; i < totalWorkerThreadsForEachServer; ++i) {
                        if(sharedTable[index][i].totalResources >= request.resources) {
                            // need lock
                            updateResources(index, i, -request.resources);
                            sharedTable[index][i].requests.push(request);
                            break;
                        }
                    }
                    if(i >= totalWorkerThreadsForEachServer) {
                        printf("\nRequest failed due to lack of resources");
                    }
                }
            }
        }

        
        void startThread(Thread *thread) { 
            printf("\n(%ld) initialization server : %d thread : %d priority: %d resources: %d", time(0) - startTime, thread->serverId, thread->id, thread->priority, thread->totalResources);
            int totalRequestForOneThread, totalResources;
            while(time(0) - startTime < RUN_TIME || !sharedTable[thread->serverId][thread->id].requests.empty()) {
                
                if(sharedTable[thread->serverId][thread->id].requests.size() > 0) {
                    totalRequestForOneThread = sharedTable[thread->serverId][thread->id].requests.size();
                    totalResources = 0;
                    while(totalRequestForOneThread--) {
                        Request request = sharedTable[thread->serverId][thread->id].requests.front();
                        sharedTable[thread->serverId][thread->id].requests.pop();
                        totalResources += request.resources;
                        printf("\n(%ld) server id: %d | thread id: %d | thread priority: %d | thread resources: %d | request id: %d | request resources: %d", time(0) - startTime, thread->serverId, thread->id, thread->priority, thread->totalResources, request.id, request.resources);
                    }
                    updateResources(thread->serverId, thread->id, totalResources);
                    sleep(THREAD_SLEEP_TIME);
                }
            }

        }

        void start() {

            pthread_create(&requestThread, NULL, callStartRequestGenerators, this); 
            pthread_create(&schedulerThread, NULL, callStartRequestScheduler, this);

            for(int i = 0; i < numberOfServers; ++i) {
                pthread_create(&(serverThreads[i]), NULL, callStartServer, this); 
                for(int j = 0; j < totalWorkerThreadsForEachServer; ++j) {
                    workerThreads[i][j].id = j;
                    workerThreads[i][j].serverId = i;
                    workerThreads[i][j].obj = this;
                    workerThreads[i][j].priority = totalWorkerThreadsForEachServer - j;
                    workerThreads[i][j].totalResources = EXPECTED_RESOURCES_FOR_THREADS + MIN_RESOURCES_FOR_THREADS;
                    workerThreads[i][j].occupiedResources = 0;

                    sharedTable[i][j].totalResources = workerThreads[i][j].totalResources;

                    if (pthread_mutex_init(&(locks[i][j]), NULL) != 0) {
                        printf("\n mutex lock %d %d init has failed\n",i, j);
                        return;
                    }
                    pthread_create(&(workerThreads[i][j].thread), NULL, callStartThread, &(workerThreads[i][j])); 
                }
            } 

            for(int i = 0; i < numberOfServers; ++i) {
                pthread_join(serverThreads[i], NULL);
                for(int j = 0; j < totalWorkerThreadsForEachServer; ++j) {
                    pthread_join(workerThreads[i][j].thread, NULL);
                }
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
    Thread *thread = (Thread *)arg;
    Schedule *obj = (Schedule *)thread->obj;
    obj->startThread(thread);
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

    printf("\n");
    return 0;
}