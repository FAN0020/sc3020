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
    int* filledBlocks;
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
void load_data_to_disk(struct Disk* disk, const char* filename){
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    char line[256]; // Buffer for each line in the file.
    fgets(line, sizeof(line), file); // Skip the first line.

    struct Block* current_block = NULL;
    int block_index = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (!current_block || current_block->curRecords >= MAX_RECORDS) {
            // Start with a new block.
            current_block = disk->blocks[block_index++];
            initBlock(current_block);
        }

        struct Record record;

        sscanf(line, "%19s\t%d\t%d\t%f\t%f\t%f\t%d\t%d\t%d", 
            record.GAME_DATE_EST, 
            &record.TEAM_ID_home, 
            &record.PTS_home,
            &record.FG_PCT_home, 
            &record.FT_PCT_home,
            &record.FG3_PCT_home, 
            &record.AST_home, 
            &record.REB_home, 
            &record.HOME_TEAM_WINS
        );

        current_block->recordsList[current_block->curRecords] = record;
        current_block->curRecords++;
    }

    fclose(file);
};

#endif // DISK_STORAGE_H
