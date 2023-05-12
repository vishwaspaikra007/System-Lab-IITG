/*
    1. Initializes all variables like eventNumber capacities etc.
    2. Creates threads to generate queries and process them.
    3. To generate query
        3.1 thread locks updateSharedTable() region (Critical Section) using mutex lock
        3.2 If there is space in shared table or if it's not violating mutual exclusion condition then it's update the table
        3.3 
    4. To process query
        4.2 Thread periodically checks a dedicated row of the table
        4.2 If query is present, it executes it and show the output in the terminal
        4.3 All these processing of query occur concurently. (CONCURRENT MAX ACTIVE QUERY)
*/
#include <iostream>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
using namespace std;

#define EVENT_NUMBER 100
#define CAPACITY 500
#define S_THREAD 20
#define MAX 5
#define NTRIES 10 // N seconds
#define RUN_TIME 60 // seconds
#define SLEEP_TIME_FOR_QUERY_THREAD x % 6 + 3 // x is random
#define NUMBER_OF_TICKETS x % 6 + 5
#define SLEEP_TIME_FOR_PROCESS_THREAD 2
#define SLEEP_TIME_FOR_RETRY 1

int startTime = time(0);

pthread_mutex_t lock_shared_table, lock_query_index, lock_process_index;

struct SharedTable {
    int eventNumber;
    string queryType;
    int threadNumber;
    int totalTickets;
};

struct BookedTickets {
    int eventNumber;
    int TotalTickets;
    BookedTickets *next;
}; // In case of cancellation of ticket data will be collected from linked list. locally saved in each thread


void *randomQuery(void *arg); // for threads to generate query
void *processRequest(void *arg); // for threads to process query

class EventReservationSystem {
    private:
        struct SharedTable *shared_table; 
        int *availableSeatsForEvent;
        int capacity;
        int MaxActiveQueries;
        int totalWorkerThread;
    public:

        int numberOfEvents;
        // EventReservationSystem(int _capacity, int _numberOfEvents, int _MaxActiveQueries, int _totalWorkerThread))
        EventReservationSystem(int _numberOfEvents, int _capacity, int _MaxActiveQueries, int _totalWorkerThread)
            :   numberOfEvents(_numberOfEvents), 
                capacity(_capacity), 
                MaxActiveQueries(_MaxActiveQueries),
                totalWorkerThread(_totalWorkerThread) 
        {
            availableSeatsForEvent = new int[_numberOfEvents];
            for(int i = 0; i < _numberOfEvents; ++i)
                availableSeatsForEvent[i] = _capacity;

            shared_table = new SharedTable[MaxActiveQueries];
            for(int i = 0; i < MaxActiveQueries; ++i)
                shared_table[i] = (SharedTable){-1, "null", -1, -1};

        }

        bool isQuery(int index) {
            if(shared_table[index].eventNumber != -1) return true;
            return false;
        } // to check whether active query is present or not in shared table

        int updateSharedTable(int eventNumber, string queryType, int threadnumber, int totalTickets) {

            for(int i = 0; i < MaxActiveQueries; ++i)
                if(shared_table[i].eventNumber == eventNumber && 
                (shared_table[i].queryType == "book" || shared_table[i].queryType == "cancel" ||
                    (shared_table[i].queryType == "inquiry" && (queryType == "book" || queryType == "cancel"))
                ))
                    return -1;

            int i = 0;
            while(i < MaxActiveQueries && shared_table[i].eventNumber != -1) ++i;
            if(queryType == "book" && totalTickets > availableSeatsForEvent[eventNumber]) return -2;
            if(i == MaxActiveQueries) return 1;
            shared_table[i] = (SharedTable){eventNumber, queryType, threadnumber, totalTickets} ;
            return 0;
        }

        int executeQuery(int processId) {
            int eventNumber = shared_table[processId].eventNumber;
            if(eventNumber == -1) return -1;
            string queryType = shared_table[processId].queryType;
            int threadNumber = shared_table[processId].threadNumber;
            
            if(queryType == "inquire") {
                printf("\n*(%4ld) P Thread = %2d | Q thread ID = %2d | query type = %10s | event = %4d | Success | availabe seats = %4d | ", time(0) - startTime, processId, threadNumber, queryType.c_str(), eventNumber, availableSeatsForEvent[shared_table[processId].eventNumber]);
                
            } else if(queryType == "book") {
                int totalTickets = shared_table[processId].totalTickets;
                availableSeatsForEvent[shared_table[processId].eventNumber] -= totalTickets;
                printf("\n*(%4ld) P Thread = %2d | Q thread ID = %2d | query type = %10s | event = %4d | Success | availabe seats = %4d | booked tickets = %d", time(0) - startTime, processId, threadNumber, queryType.c_str(), eventNumber, availableSeatsForEvent[shared_table[processId].eventNumber], totalTickets);
            } else if(queryType == "cancel") {
                int totalTickets = shared_table[processId].totalTickets;
                availableSeatsForEvent[shared_table[processId].eventNumber] += totalTickets;
                printf("\n*(%4ld) P Thread = %2d | Q thread ID = %2d | query type = %10s | event = %4d | Success | availabe seats = %4d | cancelled tickets = %d", time(0) - startTime, processId, threadNumber, queryType.c_str(), eventNumber, availableSeatsForEvent[shared_table[processId].eventNumber], totalTickets);
            }
            shared_table[processId] = (SharedTable){-1, "null", -1, -1};
            //printf("\n******** %d, %s, %d, %d", shared_table[processId].eventNumber, shared_table[processId].queryType.c_str(), shared_table[processId].threadNumber, shared_table[processId].totalTickets);
            return 1;
        }


        void generateRandomQueriesAndProcessThem() {
            pthread_t QueryGenerators[totalWorkerThread];
            pthread_t QueryProcessors[MaxActiveQueries];

            for(int i = 0; i < totalWorkerThread; ++i) {
                pthread_create(&QueryGenerators[i], NULL, randomQuery, this);
            } 

            for(int i = 0; i < MaxActiveQueries; ++i) {
                pthread_create(&QueryProcessors[i], NULL, processRequest, this);
            } 

            for(int i = 0; i < totalWorkerThread; ++i) {
                pthread_join(QueryGenerators[i], NULL);
            } 

            for(int i = 0; i < MaxActiveQueries; ++i) {
                pthread_join(QueryProcessors[i], NULL);
            } 

        }

        void showStats() {
            printf("\n\n ********* stats ********* \n");
            printf("\nEvent | Available Seats | Booked Tickets");
            for(int i = 0; i < numberOfEvents; ++i)
                printf("\n%5d | %15d | %14d", i, availableSeatsForEvent[i], 500 - availableSeatsForEvent[i]);
            printf("\n");
        }
};


void *randomQuery(void *arg) {
    EventReservationSystem *obj = (EventReservationSystem *)arg;
    string arr[] = {"inquire", "book", "cancel", "book", "book"};
    static int index_counter = -1;
    pthread_mutex_lock(&lock_query_index);
    int index = ++index_counter;
    pthread_mutex_unlock(&lock_query_index);
    printf("\nQ thread = %d", index);

    BookedTickets *b_tickets = NULL;
    while(time(0) - startTime < RUN_TIME) {
        int x = rand();
        int eventNumber = x % obj->numberOfEvents;
        string queryType = arr[x % 5];
        x = rand();
        int totalTickets = NUMBER_OF_TICKETS;

        if(queryType == "cancel") { 
            if(b_tickets == NULL) continue;
            totalTickets = b_tickets->TotalTickets;
            eventNumber = b_tickets->eventNumber;
        }

        // periodic query after every random seconds
        sleep(SLEEP_TIME_FOR_QUERY_THREAD);

        bool retry = true;
        int n_retry = NTRIES;
        while(retry == 1 && n_retry--) {
            retry = false;
            // shared table is locked before accessing it
            pthread_mutex_lock(&lock_shared_table);
            int result = obj->updateSharedTable(eventNumber, queryType, index, totalTickets);
            pthread_mutex_unlock(&lock_shared_table); 

            if(result == 0) {
                if(queryType == "book") {
                    BookedTickets *temp = new BookedTickets();
                    temp->eventNumber = eventNumber;
                    temp->TotalTickets = totalTickets;
                    temp->next = b_tickets;
                    b_tickets = temp; 
                } else if(queryType == "cancel") {
                    BookedTickets *temp = b_tickets->next;
                    delete b_tickets;
                    b_tickets = temp;
                }
            } else if(result == 1) {  
                retry = true;
                printf("\n*(%4ld) P Thread = %2d | Q thread ID = %2d | query type = %10s | event = %4d | Wait    | No space in active query", time(0) - startTime, -1, index, queryType.c_str(), eventNumber);
                // At any point of time, at most MAX queries can be active. Any new query ((MAX + 1)st or
                // (MAX + 2)nd or so on) must wait until one or more of the active queries finish. Use
                // appropriate condition variable(s) to enforce this restriction. Note that this wait is to be
                // interpreted as blocking, that is, a thread waiting for the server load to reduce must block
                // until signaled by another thread during the completion of an active query.
            } else if(result == -1){
                printf("\n*(%4ld) P Thread = %2d | Q thread ID = %2d | query type = %10s | event = %4d | Failed  | Data Inconsistency problem", time(0) - startTime, -1, index, queryType.c_str(), eventNumber);
                // Notice that when a query fails in order to insure database consistency, the
                // thread is not blocked. It instead proceeds to make the next query (after a short sleep) in
                // the loop.
            } else {
                printf("\n*(%4ld) P Thread = %2d | Q thread ID = %2d | query type = %10s | event = %4d | Failed  | availabe seats = %4d | requested tickets = %d | not enough seats", time(0) - startTime, -1, index, queryType.c_str(), eventNumber, result, totalTickets);
            }
            sleep(SLEEP_TIME_FOR_RETRY);
        }
    }
    return NULL;
}

void *processRequest(void *arg) {
    EventReservationSystem *obj = (EventReservationSystem *)arg;

    static int processId = -1;
    pthread_mutex_lock(&lock_process_index);
    int index = ++processId;
    pthread_mutex_unlock(&lock_process_index);
    printf("\nP thread = %d", index);

    while(time(0) - startTime < RUN_TIME || obj->isQuery(index)) {
        obj->executeQuery(index);
        sleep(SLEEP_TIME_FOR_PROCESS_THREAD);
    }
    return NULL;
}

// ************************************************ main ************************************************ 
int main() {
    srand(time(0));

    if (pthread_mutex_init(&lock_shared_table, NULL) != 0) {
        printf("\n mutex lock_shared_table init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&lock_query_index, NULL) != 0) {
        printf("\n mutex lock_query_index init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&lock_process_index, NULL) != 0) {
        printf("\n mutex lock_process_index init has failed\n");
        return 1;
    }

    // EventReservationSystem(int _numberOfEvents, int _capacity, int _MaxActiveQueries, int _totalWorkerThread))

    int eventNumber = EVENT_NUMBER;
    int capacity = CAPACITY;
    int s_thread = S_THREAD; // number of worker threads to make queries
    int maxQueries = MAX;

    EventReservationSystem obj(eventNumber, capacity, maxQueries, s_thread);

    // generate random queries in a periodic interaval
    obj.generateRandomQueriesAndProcessThem();
    obj.showStats();

    return 0;
}

