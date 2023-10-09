#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"


int getFirstAvailableBlockId(struct Disk* disk) {
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        if (disk->availableBlocks[i] != -1) {
            return disk->availableBlocks[i];
        }
    }
    return -1; // No available blocks found
}

int insertRecord(struct Block* block, struct Record rec) {
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (block->recordsList[i].GAME_DATE_EST == 0 &&
            block->recordsList[i].TEAM_ID_home == 0 &&
            block->recordsList[i].PTS_home == 0 &&
            block->recordsList[i].FG_PCT_home == 0 &&
            block->recordsList[i].FT_PCT_home == 0 &&
            block->recordsList[i].FG3_PCT_home == 0 &&
            block->recordsList[i].AST_home == 0 &&
            block->recordsList[i].REB_home == 0 &&
            block->recordsList[i].HOME_TEAM_WINS == 0) {
            
            // Insert the record
            block->recordsList[i] = rec;
            block->curRecords++;
            
            return i; // Return the offset where the record was inserted
        }
    }
    return -1; // Indicates that the block is full
}


struct Address insertRecordIntoBlock(struct Disk* disk, int blockPtr, struct Record rec) {
    if (blockPtr == -1) {
        return (struct Address){-1, -1};
    }
    struct Block* block = disk->blocks[blockPtr];
    struct Address address;
    address.offset = insertRecord(block, rec);
    disk->filledBlocks[blockPtr] = 1;

    if (!isBlockAvailable(block)) {
        disk->availableBlocks[blockPtr] = -1;
    }

    return address;
}

struct Address writeRecordToStorage(struct Disk* disk, struct Record rec) {
    disk->numOfRecords++;
    int blockPtr = getFirstAvailableBlockId(disk);
    if (blockPtr == -1) {
        // Handle the case where no available blocks are found
        return (struct Address){-1, -1};
    }
    struct Address addressofRecordStored = insertRecordIntoBlock(disk, blockPtr, rec);
    return addressofRecordStored;
}

int getNumberBlockUsed(struct Disk* disk) {
    int count = 0;
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        if (disk->filledBlocks[i] == 1) {
            count++;
        }
    }
    return count;
}

int getBlockAccesses(struct Disk* disk) {
    return disk->blockAccesses;
}

struct Block* getBlock(struct Disk* disk, int blockNumber) {
    struct Block* block = get(disk->lruCache, blockNumber);
    if (block != NULL) {
        disk->blockAccessReduced++;
    }
    if (block == NULL && blockNumber >= 0) {
        // 1 I/O
        block = disk->blocks[blockNumber];
        disk->blockAccesses++;
        put(disk->lruCache, blockNumber, block);
    }
    return block;
}

int getBlockAccessReduced(struct Disk* disk) {
    return disk->blockAccessReduced;
}

void deleteRecord(struct Block* block, int offset) {
    block->recordsList[offset].GAME_DATE_EST = -1;
    block->recordsList[offset].TEAM_ID_home = -1; 
    block->recordsList[offset].PTS_home = -1;
    block->recordsList[offset].FG_PCT_home = -1.0f; 
    block->recordsList[offset].FT_PCT_home = -1.0f; // Float value
    block->recordsList[offset].FG3_PCT_home = -1.0f; 
    block->recordsList[offset].AST_home = -1;
    block->recordsList[offset].REB_home = -1;
    block->recordsList[offset].HOME_TEAM_WINS = 0;

    block->curRecords--;
}

int runBruteForceSearch(struct Disk* disk, double FG_PCT_homeValue, double FG_PCT_homeValueUpperRange) {
    int countBlockAccess = 0, countRetrieve = 0;
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        if (disk->filledBlocks[i] == 1) {
            countBlockAccess++;
            struct Block* block = disk->blocks[i];
            int numberOfRecordsInBlock = getCurSize(block);
            for (int j = 0; j < numberOfRecordsInBlock; j++) {
                struct Record rec = getRecordFromBlock(block, j);
                double curFG_PCT_home = rec.FG_PCT_home;
                if (FG_PCT_homeValue <= curFG_PCT_home && curFG_PCT_home <= FG_PCT_homeValueUpperRange) {  
                    countRetrieve++;
                }
            }
        }
    }
    return countBlockAccess;
}

int runBruteForceDelete(struct Disk* disk, double FG_PCT_homeValue, double FG_PCT_homeValueUpperRange) {
    int countBlockAccess = 0, countDeletion = 0;
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        if (disk->filledBlocks[i] == 1) {
            countBlockAccess++;
            struct Block* block = disk->blocks[i];
            int numberOfRecordsInBlock = getCurSize(block);
            for (int j = 0; j < numberOfRecordsInBlock; j++) {
                struct Record rec = getRecordFromBlock(block, j);
                double curFG_PCT_home = rec.FG_PCT_home;
                if (FG_PCT_homeValue <= curFG_PCT_home && curFG_PCT_home <= FG_PCT_homeValueUpperRange) {  
                    deleteRecord(block,j);    
                }
            }
        }
    }
    return countBlockAccess;
}

void freeBlock(struct Block* block) {
    if(block) {
        free(block);  // Simply free the memory associated with the block
    }
}

// Function to free memory used by the disk
void freeDisk(struct Disk* disk) {
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        freeBlock(disk->blocks[i]);
    }
    free(disk->blocks);
    free(disk->availableBlocks);
//    free(disk->filledBlocks);
if (disk->filledBlocks) {
    free(disk->filledBlocks);
    disk->filledBlocks = NULL;
}
    freeLRUCache(disk->lruCache);
    free(disk);
}

struct Disk* loadData(struct Disk* disk, const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return NULL;
    }

    char line[255];
    fgets(line, sizeof(line), file); // Skip header line

    int currentBlockId = 0; 
    struct Block* currentBlock = getBlock(disk, currentBlockId);
    int lineCounter = 0;  // To track the line number

    while (fgets(line, sizeof(line), file)) {
        lineCounter++;

        // Debug
        //printf("(for debug)Line %d: %s", lineCounter, line);

        // Parse the line to create a record
        struct Record record;

        int day, month, year;
        sscanf(line, "%d/%d/%d %d %d %f %f %f %d %d %s", &day, &month, &year, &record.TEAM_ID_home,
               &record.PTS_home, &record.FG_PCT_home, &record.FT_PCT_home,
               &record.FG3_PCT_home, &record.AST_home, &record.REB_home,
               (char*) & record.HOME_TEAM_WINS);
        record.GAME_DATE_EST = (year * 10000) + (month * 100) + day;

        if (currentBlock->curRecords >= (int)(MAX_RECORDS)) {
            disk->filledBlocks[currentBlockId] = 1; // Mark the block as filled
            currentBlockId++;
            currentBlock = getBlock(disk, currentBlockId);
            initBlock(currentBlock);
        }

        if (currentBlock->curRecords >= (int)(MAX_RECORDS)) {
            printf("Block is full.\n");
            return NULL;
        }

        writeRecordToStorage(disk, record);
        //printf("Record No.: %d, Stored in Block: %d\n", disk->numOfRecords, currentBlockId);
    }
    
    fclose(file);
    printf("Data loaded into disk.\n");
    return disk;
}