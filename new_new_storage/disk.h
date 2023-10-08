#ifndef DISK_STORAGE_H
#define DISK_STORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "LRUCache.h"

struct Disk {
    struct Block** blocks; 
    int* availableBlocks;
    int* filledBlocks; // blocks that has any record
    int memdiskSize;
    int blkSize;
    int numOfRecords;
    int blockAccesses;
    int blockAccessReduced;
    struct LRUCache* lruCache;
};

struct Address {
    int blockID;
    int offset;
};

struct Disk* createDisk(int diskSize, int blkSize, int lruCacheSize){
    struct Disk* disk = (struct Disk*)malloc(sizeof(struct Disk));
    disk->memdiskSize = diskSize;
    disk->blkSize = blkSize;
    disk->blocks = (struct Block**)malloc(sizeof(struct Block*) * (diskSize / blkSize));
    disk->availableBlocks = (int*)malloc(sizeof(int) * (diskSize / blkSize));
    disk->filledBlocks = (int*)malloc(sizeof(int) * (diskSize / blkSize));
    disk->numOfRecords = 0;
    disk->blockAccesses = 0;
    disk->blockAccessReduced = 0;
    disk->lruCache = createLRUCache(lruCacheSize);
    
    for (int i = 0; i < diskSize / blkSize; i++) {
        disk->blocks[i] = createBlock(); 
        disk->availableBlocks[i] = i;
    }

    return disk;
};
int getFirstAvailableBlockId(struct Disk* disk);
struct Address writeRecordToStorage(struct Disk* disk, struct Record rec);
int getNumberBlockUsed(struct Disk* disk);
int getBlockAccesses(struct Disk* disk);
struct Block* getBlock(struct Disk* disk, int blockNumber);
int getBlockAccessReduced(struct Disk* disk);
void deleteRecord(struct Block* block, int offset);
int runBruteForceSearch(struct Disk* disk, int FG_PCT_homeValue, int FG_PCT_homeValueUpperRange);
void freeDisk(struct Disk* disk);
int loadDataIntoDisk(struct Disk* disk);
struct Disk* loadData(struct Disk* disk, const char* filename);

#endif // DISK_STORAGE_H
