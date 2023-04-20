/*
    program contains Schedue Class nad it contains following functions
        1. Schedue() : initializes all the variables
        2. void start : starts all worker threads and startServer() startRequestGenerators() and startRequestScheduler()  
        3. void startServer() : assign requests to worker thread based on the priority of the thread and available resources
        4. void startRequestGenerators() : takes transaction input
        5. void startRequestScheduler() : pushes requests from main queue to dedicated server queue
        6. void startThread(Thread *thread) : processes the request and prints output
*/

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

#define SERVER_SLEEP_TIME (rand() % 2) + 2
#define THREAD_SLEEP_TIME (rand() % 5) + 3
#define SCHEDULER_SLEEP_TIME 1 
#define REQUEST_GENERATOR_SLEEP_TIME 1


using namespace std;

pthread_mutex_t lock_server_index, lock_stats, lock_LackOfResources, lock_LackOfAvailableResources, lock_LackOfThread;

int startTime = time(0);

void *callStartRequestGenerators(void *arg);
void *callStartRequestScheduler(void *arg);
void *callStartServer(void *arg);
void *callStartThread(void *arg);

struct ServiceRequest {
    int id;
    int type;
    int resources;
    int arrivalTime;
};

struct Request {
    int id;
    int resources;
    int arrivalTime;
    int blockTime;
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
    int availableResources;
    queue<Request> requests;
};

class Schedule {
    private:
        int numberOfServers; // number of services
        int totalWorkerThreadsForEachServer; // worker threads of each service
        int totalTurnaroundTime;
        int totalWaitTime;
        int rejected_requests_due_to_lack_of_resources;
        unordered_set<int> requests_forced_to_wait_due_to_lack_of_resources;
        unordered_set<int> blocked_requests_due_to_absence_of_available_workerthreads;
        int totalRequests;

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

            totalWaitTime = 0;
            totalTurnaroundTime = 0;            
            rejected_requests_due_to_lack_of_resources = 0;
            //requests_forced_to_wait_due_to_lack_of_resources = 0;
            //blocked_requests_due_to_absence_of_available_workerthreads = 0;

            serverThreads.resize(numberOfServers);
            queueOfService.resize(numberOfServers);

            workerThreads.resize(numberOfServers,vector<Thread>(totalWorkerThreadsForEachServer));
            sharedTable.resize(numberOfServers,vector<SharedTable>(totalWorkerThreadsForEachServer));
            locks.resize(numberOfServers, vector<pthread_mutex_t>(totalWorkerThreadsForEachServer));

            printf("\nqueueofservice %ld ", queueOfService.size());
        }
        // prints all stats ---------------------------------------------------------------------------------------------------
        void printStats() {
            printf("\n\n------------------------------ Stats ------------------------------");
            printf("\ntotal Threshold: %d s", totalTurnaroundTime);
            printf("\ntotal wait time: %d s", totalWaitTime);
            printf("\nAverage Threshold: %f s", totalTurnaroundTime / (float)totalRequests);
            printf("\nAverage wait time: %f s", totalWaitTime / (float)totalRequests);
            printf("\nrejected_requests_due_to_lack_of_resources: %d", rejected_requests_due_to_lack_of_resources);
            printf("\nrequests_forced_to_wait_due_to_lack_of_resources: %ld", requests_forced_to_wait_due_to_lack_of_resources.size());
            printf("\nblocked_requests_due_to_absence_of_available_workerthreads: %ld", blocked_requests_due_to_absence_of_available_workerthreads.size());

            printf("\n-------------------------------------------------------------------");
        }
        // prints all stats ---------------------------------------------------------------------------------------------------

        // updates all stats variables ----------------------------------------------------------------------------------------
        void updateStats(int turnaroundTime, int waitTime) {
            pthread_mutex_lock(&lock_stats);
            totalTurnaroundTime += turnaroundTime;
            totalWaitTime += waitTime;
            pthread_mutex_unlock(&lock_stats);
        }
        void updateResources(int i, int j, int val) {
            pthread_mutex_lock(&(locks[i][j]));
            sharedTable[i][j].availableResources += val;
            pthread_mutex_unlock(&(locks[i][j]));
        }
        void updateLackOfResources() {
            pthread_mutex_lock(&lock_LackOfResources);
                ++rejected_requests_due_to_lack_of_resources;
            pthread_mutex_unlock(&lock_LackOfResources);
        }
        void updateLackOfAvailableResources(int id) {
            pthread_mutex_lock(&lock_LackOfAvailableResources);
                requests_forced_to_wait_due_to_lack_of_resources.insert(id);
            pthread_mutex_unlock(&lock_LackOfAvailableResources);
        }
        void updateLackOfThread(int id) {
            pthread_mutex_lock(&lock_LackOfThread);
                blocked_requests_due_to_absence_of_available_workerthreads.insert(id);
            pthread_mutex_unlock(&lock_LackOfThread);
        }
        // updates all stats variables ----------------------------------------------------------------------------------------

        // takes transaction input --------------------------------------------------------------------------------------------
        void startRequestGenerators() {
            int totalTransactions;
            int requestType;
            int requestResources;

            scanf("%d", &totalTransactions);
            totalRequests = totalTransactions;
            for(int i = 0; i < totalTransactions; ++i) {
                scanf("%d %d", &requestType, &requestResources);
                // pushing request to main queue
                requestQueue.push({i, requestType, requestResources, (int)(time(0) - startTime)});
                if(i%5 == 0)
                    sleep(REQUEST_GENERATOR_SLEEP_TIME);
            }
        }
        // takes transaction input --------------------------------------------------------------------------------------------

        // pushes requests from main queue to dedicated server queue ----------------------------------------------------------
        void startRequestScheduler() {
            while(time(0) - startTime < RUN_TIME || !requestQueue.empty()) {
                if(!requestQueue.empty()) {
                    ServiceRequest request = requestQueue.front();
                    requestQueue.pop();
                    printf("\n(%ld) request id: %d | request type: %d | request resources: %d",time(0) - startTime, request.id, request.type, request.resources);
                    // pushing request to dedicated server/service queue
                    queueOfService[request.type].push({request.id, request.resources, request.arrivalTime});
                }
                // sleep(SCHEDULER_SLEEP_TIME);
            }
        }
        // pushes requests from main queue to dedicated server queue ----------------------------------------------------------

        // assign requests to worker thread based on the priority of the thread and available resources -----------------------
        void startServer() {
            Request request;
            bool isResourceDemandTooHigh;
            static int serverId = -1;
            pthread_mutex_lock(&lock_server_index);
            int index = ++serverId;
            pthread_mutex_unlock(&lock_server_index);

            printf("\nserver : %d", index);
            
            while(time(0) - startTime < RUN_TIME || !queueOfService[index].empty()) {
                if(!queueOfService[index].empty()) {
                    request = queueOfService[index].front();
                    queueOfService[index].pop();
                    isResourceDemandTooHigh = true;
                    int i;
                    // go through all worker thread for each request
                    // appropriate worker thread is chosen to process the request
                    // usually worker thread with higher priority and avaialble resources
                    // lower index => higher priority sorted earlier based on priority
                    for(i = 0; i < totalWorkerThreadsForEachServer; ++i) {
                        if(isResourceDemandTooHigh && sharedTable[index][i].totalResources > request.resources) isResourceDemandTooHigh = false;
                        if(sharedTable[index][i].availableResources >= request.resources) {
                            // need lock
                            updateResources(index, i, -request.resources);
                            request.blockTime = (int)(time(0) - startTime);
                            sharedTable[index][i].requests.push(request);
                            break;
                        }
                    }
                    // In case no Worker threas is assigned
                    if(i >= totalWorkerThreadsForEachServer) {
                        if(isResourceDemandTooHigh) {
                            printf("\nrejected due to lack of resources id: %d", request.id);
                            updateLackOfResources();
                        } else {
                            updateLackOfAvailableResources(request.id);
                            queueOfService[index].push(request);
                            printf("\nsending back to queue due to lack of resources id: %d", request.id);
                            sleep(1);
                        }
                    }
                }
            }
        }
        // assign requests to worker thread based on the priority of the thread and available resources -----------------------
        
        // processes the request and prints output ----------------------------------------------------------------------------
        void startThread(Thread *thread) { 
            printf("\n(%ld) initialization server : %d thread : %d priority: %d resources: %d", time(0) - startTime, thread->serverId, thread->id, thread->priority, thread->totalResources);
            int totalRequestForOneThread, totalResources, time1, time2, turnaroundTime, waitTime;
            // continuously checks shared table for available requests 
            while(time(0) - startTime < RUN_TIME || !sharedTable[thread->serverId][thread->id].requests.empty()) {
                // if any request
                if(sharedTable[thread->serverId][thread->id].requests.size() > 0) {
                    totalRequestForOneThread = sharedTable[thread->serverId][thread->id].requests.size();
                    totalResources = 0;
                    time1 = (int)(time(0) - startTime);
                    sleep(THREAD_SLEEP_TIME);
                    time2 = (int)(time(0) - startTime);
                    // process all requests assigned to this thread
                    printf("\n--------------------------------------------------------------------------------------------");
                    while(totalRequestForOneThread--) {
                        Request request = sharedTable[thread->serverId][thread->id].requests.front();
                        // in case thread had wait if this thread was busy but had enough resources
                        if(time1 - request.blockTime > 0) {
                            updateLackOfThread(request.id);
                        }
                        turnaroundTime = time2 - request.arrivalTime;
                        waitTime = time1 - request.arrivalTime;
                        sharedTable[thread->serverId][thread->id].requests.pop();
                        totalResources += request.resources;

                        // prints the output
                        printf("\n(%2ld) server id: %2d | thread [id: %2d, priority: %2d, resources: %2d] | request [id: %2d, resources: %2d, arrival time: %2d, turnaround time: %2d, waiting time: %2d]"
                        , time(0) - startTime, thread->serverId, thread->id, thread->priority, thread->totalResources, request.id, request.resources, request.arrivalTime, turnaroundTime, waitTime);
                    }
                    printf("\n--------------------------------------------------------------------------------------------\n");
                    // updating turnaround time and wait time
                    updateResources(thread->serverId, thread->id, totalResources);
                    updateStats(turnaroundTime, waitTime);
                }
            }

        }
        // processes the request and prints output ----------------------------------------------------------------------------

        // assign requests to worker thread based on the priority of the thread and available resources -----------------------
        void start() {
            // scanning priorities and resources for each thread
            for(int i = 0; i < numberOfServers; ++i) {
                for(int j = 0; j < totalWorkerThreadsForEachServer; ++j) {
                    scanf("%d %d", &(workerThreads[i][j].priority), &(workerThreads[i][j].totalResources));
                }
                // sorting based on priority for each server
                sort(begin(workerThreads[i]), end(workerThreads[i]), [&](Thread A, Thread B) {
                    return A.priority > B.priority;
                });
            } 

            for(int i = 0; i < numberOfServers; ++i) {
                // thread creation for server to assign request to worker threads
                pthread_create(&(serverThreads[i]), NULL, callStartServer, this); 
                // thread creation for worker threads
                for(int j = 0; j < totalWorkerThreadsForEachServer; ++j) {
                    workerThreads[i][j].id = j;
                    workerThreads[i][j].serverId = i;
                    workerThreads[i][j].obj = this;
                    workerThreads[i][j].occupiedResources = 0;

                    sharedTable[i][j].totalResources = workerThreads[i][j].totalResources;
                    sharedTable[i][j].availableResources = workerThreads[i][j].totalResources;

                    if (pthread_mutex_init(&(locks[i][j]), NULL) != 0) {
                        printf("\n mutex lock %d %d init has failed\n",i, j);
                        return;
                    }
                    pthread_create(&(workerThreads[i][j].thread), NULL, callStartThread, &(workerThreads[i][j])); 
                }
            } 

            pthread_create(&requestThread, NULL, callStartRequestGenerators, this); 
            pthread_create(&schedulerThread, NULL, callStartRequestScheduler, this);

            // waits for all threads to complete
            for(int i = 0; i < numberOfServers; ++i) {
                pthread_join(serverThreads[i], NULL);
                for(int j = 0; j < totalWorkerThreadsForEachServer; ++j) {
                    pthread_join(workerThreads[i][j].thread, NULL);
                }
            } 

            pthread_join(requestThread, NULL);
            pthread_join(schedulerThread, NULL);
        }
        // assign requests to worker thread based on the priority of the thread and available resources -----------------------

        // End of Class -------------------------------------------------------------------------------------------------------
};

// thread functions calling above functions -----------------------------------------------------------------------------------
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
// thread functions calling above functions -----------------------------------------------------------------------------------

int main() {
    srand(time(0));

    // initializing necessary locks
    if (pthread_mutex_init(&lock_server_index, NULL) != 0) {
        printf("\n mutex lock_process_index init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&lock_stats, NULL) != 0) {
        printf("\n mutex lock_process_index init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&lock_LackOfResources, NULL) != 0) {
        printf("\n mutex lock_process_index init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&lock_LackOfAvailableResources, NULL) != 0) {
        printf("\n mutex lock_process_index init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&lock_LackOfThread, NULL) != 0) {
        printf("\n mutex lock_process_index init has failed\n");
        return 1;
    }

    int n, m;
    scanf("%d%d", &n, &m);

    Schedule s_obj(n, m); // initializes all the variables
    s_obj.start();
    s_obj.printStats();

    printf("\n");
    return 0;
}