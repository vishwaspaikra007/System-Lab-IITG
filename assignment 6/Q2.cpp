/**
 * class System
 *  |- generateLogicalAddress
 *  |- getPhysicalAddress
 *  |- LoadPageIntoMainMemoryAndReturnPhysicalAddress
 * int main
 *  |- System sys
*/

#include <iostream>
#include <queue>
#include <vector>
#include <list>

using namespace std;

struct MainMemory {
    long long frameSize;
    long long addressSpace;
    vector<vector<long long>> data;
};

struct SecondaryMemory {
    long long pageSize;
    long long addressSpace;
    vector<vector<long long>> data;
};

struct PhysicalAddress {
    long long frameNumber;
    long long dataOffset;
};

struct LogicalAddress {
    long long pageNumber;
    long long dataOffset;
};

/**
 * For Least Recently Used we maintaina Doubly Linked List
 * such that every newest memory access is at front of the list
 * And least recent memory access at the last
 * LRU to for head and tail and Deque actual linked list
*/
struct Deque {
    LogicalAddress logicalAddress;
    Deque *prev;
    Deque *next;
};
struct LRU { 
    long long size;
    Deque *head;
    Deque *tail;
};

/**
 * In Most Frequently used we are maintaing a count 
 * which keeps count of number of memory reference of a partucular frame
 * If count is high then we remove it from main memory
 * thinking it has already been used alot and may not be used any further
*/
struct MFU {
    int count;
    LogicalAddress logicalAddress;
};

/**
 * PageTable maps Logical Address with Physical Address
 * frameNumber of main memory
 * present in main memeory
 * node for least recently used so that we can send node to the front of the Doubly Linked list
*/
struct PageTable {
    long long frameNumber;
    bool present = false;
    Deque *node;
};

class System {
    private:
        MainMemory mainMemory;
        SecondaryMemory secondaryMemory;

        /**
         * pageTable Size equal to number of pages
         * pageReplacementAlgorithm = FIFO | LRU | MFU
        */
        vector<PageTable>pageTable;
        string pageReplacementAlgorithm;

        /**
         * Data Structures for Page Replacement Algorithm
         * FIFO: qForFIFO
         * LRU: deque
         * MFU: listMFU
        */
        queue<LogicalAddress> qForFIFO;
        LRU deque = {0, NULL, NULL};
        vector<MFU> listMFU;
        
    public:
        // stats
        long long pageFaults = 0;
        long long  swaps = 0;

        /**
         * System constructor initializes following
         * INPUT: #frames, #pages, page/frame size, page replacement algorithm for the system object
        */
        System(long long numberOfFrames, long long numberOfPages, long long size, string _pageReplacementAlgorithm, int _swapFileSize = 0) {
            mainMemory.frameSize = size;
            secondaryMemory.pageSize = size;

            mainMemory.addressSpace = numberOfFrames;
            secondaryMemory.addressSpace = numberOfPages;

            mainMemory.data.resize(numberOfFrames , vector<long long>(size, -1));
            secondaryMemory.data.resize(numberOfPages , vector<long long>(size, -1));

            pageTable.resize(numberOfPages);
            pageReplacementAlgorithm = _pageReplacementAlgorithm;
        }
        /**
         * LoadPageIntoMainMemoryAndReturnPhysicalAddress
         * INPUT: logical address
         * WORKING: * if there is space in main memory
         *          *   then it increments page fault and maps available frame in paging table
         *          * else
         *          *   As per page replacement algorithm replace one frame with another frame in main memory
         *          *   and maps frame in paging table
         * OUTPUT: return physical address with is maped with logical address in page table
        */
        PhysicalAddress LoadPageIntoMainMemoryAndReturnPhysicalAddress(LogicalAddress logicalAddress) {
            long long frameNumber;
            LogicalAddress logicalAddressToBeRemoved;
            bool swap = false;

            /**
             * FIFO
             * new memory access goes to qForFIFO which stores logical address
             * Maps logical address to physical address in page table
             * first logical address in qForFIFO is removed from page table and memory access is mapped in page table 
            */
            if(pageReplacementAlgorithm == "FIFO") {
                ++pageFaults;
                if(qForFIFO.size() == mainMemory.addressSpace) {
                    ++swaps;
                    logicalAddressToBeRemoved = qForFIFO.front();
                    qForFIFO.pop();
                    frameNumber = pageTable[logicalAddressToBeRemoved.pageNumber].frameNumber;

                    pageTable[logicalAddressToBeRemoved.pageNumber].present = false;
                    swap = true;
                } else {
                    frameNumber = qForFIFO.size();
                }

                pageTable[logicalAddress.pageNumber].present = true;
                pageTable[logicalAddress.pageNumber].frameNumber = frameNumber;
                qForFIFO.push(logicalAddress);

            } 
            /**
             * LRU
             * new memory access goes to deque.head
             * dequeu.tail is removed if main memory is full
             * And freed frame in main memory is used for new memory access
             * if any memory is accessed present in main memory the node is shifted at front
            */
            else if(pageReplacementAlgorithm == "LRU") {
                ++pageFaults;
                if(deque.size == mainMemory.addressSpace) {
                    ++swaps;
                    logicalAddressToBeRemoved = deque.tail->logicalAddress;

                    Deque *nodeTobeDeleted = deque.tail;
                    deque.tail = deque.tail->prev;
                    delete(nodeTobeDeleted);

                    frameNumber = pageTable[logicalAddressToBeRemoved.pageNumber].frameNumber;

                    pageTable[logicalAddressToBeRemoved.pageNumber].present = false;
                    swap = true;

                } else {
                    frameNumber = deque.size;
                    ++deque.size;
                }

                Deque *node = new Deque();
                node->logicalAddress = logicalAddress;
                node->next = deque.head;
                node->prev = NULL;

                deque.head = node;
                if(deque.tail == NULL) deque.tail = node;

                pageTable[logicalAddress.pageNumber].present = true;
                pageTable[logicalAddress.pageNumber].frameNumber = frameNumber;
                pageTable[logicalAddress.pageNumber].node = node;

            }  
            /**
             * MFU: most frequently used
             * maintains a vector of main memory size
             * if main memory is full it find logical address with most count and removes it with amin memory and page table
             * and freed frame is used fro new mapping
            */
            else if(pageReplacementAlgorithm == "MFU") {
                ++pageFaults;
                if(listMFU.size() == mainMemory.addressSpace) {
                    ++swaps;
                    long long mini = 0;
                    for(long long i = 0; i < mainMemory.addressSpace; ++i) {
                        if(listMFU[mini].count < listMFU[i].count)
                            mini = i;
                    }
                    logicalAddressToBeRemoved = listMFU[mini].logicalAddress;
                    frameNumber = mini;
                    listMFU[mini].logicalAddress = logicalAddress;
                    listMFU[mini].count = 0;
                    // frameNumber = pageTable[logicalAddressToBeRemoved.pageNumber].frameNumber;

                    pageTable[logicalAddressToBeRemoved.pageNumber].present = false;
                    swap = true;

                } else {
                    frameNumber = listMFU.size();
                    listMFU.push_back({0, logicalAddress});
                }

                pageTable[logicalAddress.pageNumber].present = true;
                pageTable[logicalAddress.pageNumber].frameNumber = frameNumber;
            }

            return (PhysicalAddress){frameNumber, logicalAddress.dataOffset};
        }

        /**
         * INPUT: none
         * WORKING: randomly genereates logical address
         * OUTPUT: logical address {page number, data offset}
        */
        LogicalAddress generateLogicalAddress() {
            long long pageNumberOffset =  rand() % (secondaryMemory.addressSpace);
            long long dataOffset = rand() % secondaryMemory.pageSize ;
            return (LogicalAddress){pageNumberOffset, dataOffset};
        }


        /**
         * INPUT: logical address
         * WORKING: if page table is present then return physical address
         *          else calls LoadPageIntoMainMemoryAndReturnPhysicalAddress
         * OUTPUT: physical address {page number, data offset}
        */
        PhysicalAddress getPhysicalAddress(LogicalAddress logicalAddress) {
            if(pageTable[logicalAddress.pageNumber].present == true) {
                if(pageReplacementAlgorithm == "LRU") {
                    Deque *node = pageTable[logicalAddress.pageNumber].node;
                    if(node->prev) {
                        node->prev->next = node->next;
                        if(node->next) node->next->prev = node->prev;
                        node->next = deque.head;
                        node->prev = NULL;
                        deque.head = node;
                    }
                } else if(pageReplacementAlgorithm == "MFU") {
                    ++listMFU[pageTable[logicalAddress.pageNumber].frameNumber].count;
                }
                return (PhysicalAddress){pageTable[logicalAddress.pageNumber].frameNumber, logicalAddress.dataOffset};
            } else {
                return LoadPageIntoMainMemoryAndReturnPhysicalAddress(logicalAddress);
            }
        }
};

/**
 * INPUT: #memory access, System Object
 * prints page faults for FIFO, LRU, MFU
*/
void startCPU(long long numberOfMemoryAccesses, System *sys) {
    PhysicalAddress physicalAddress;
    LogicalAddress logicalAddress;
    for(long long i = 0; i < numberOfMemoryAccesses; ++i) {
        logicalAddress = sys[0].generateLogicalAddress();

        physicalAddress = sys[0].getPhysicalAddress(logicalAddress); // for FIFO
        physicalAddress = sys[1].getPhysicalAddress(logicalAddress); // for LRU
        physicalAddress = sys[2].getPhysicalAddress(logicalAddress); // for MFU

        // mainMemory.data[physicalAddress.frameNumber][physicalAddress.dataOffset] = rand(); // random instruction
    }
    printf("\n%10Ld %10Ld %12Ld %12Ld", numberOfMemoryAccesses, sys[0].pageFaults, sys[1].pageFaults, sys[2].pageFaults);
} 

int main() {
    srand(time(0));
    // size preferably of power of 2

    // n = #frames, m = #pages, size = size(frame) = size(page)
    int n = 1 << 8, m = 1 << 6, size = 1 << 3, swapFileSize = 1 << 0; 
    System sys[3] = {System(n, m, size, "FIFO"), System(n, m, size, "LRU"), System(n, m, size, "MFU")};
    System sys1[3] = {System(n, m, size, "FIFO"), System(n, m, size, "LRU"), System(n, m, size, "MFU")};
    System sys2[3] = {System(n, m, size, "FIFO"), System(n, m, size, "LRU"), System(n, m, size, "MFU")};


    printf("\nframes = %d pages = %d page size = %d", n, m, size);
    printf("\n-------------------------------------------------------------------");
    printf("\niterations       FIFO          LRU          MFU");
    startCPU(100, sys);
    startCPU(5000, sys1);
    startCPU(10000, sys2);
    // ---------------------------------------------------------------------------------

    // n = #frames, m = #pages, size = size(frame) = size(page)
    n = 1 << 9, m = 1 << 16, size = 1 << 6;
    System sys3[3] = {System(n, m, size, "FIFO"), System(n, m, size, "LRU"), System(n, m, size, "MFU")};
    System sys4[3] = {System(n, m, size, "FIFO"), System(n, m, size, "LRU"), System(n, m, size, "MFU")};
    System sys5[3] = {System(n, m, size, "FIFO"), System(n, m, size, "LRU"), System(n, m, size, "MFU")};

    printf("\n\nframes = %d pages = %d page size = %d", n, m, size);
    printf("\n-------------------------------------------------------------------");
    printf("\niterations       FIFO          LRU          MFU");
    startCPU(100, sys3);
    startCPU(5000, sys4);
    startCPU(10000, sys5);

    printf("\n");
    return 0;
}