#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"



// struct Disk {
//     struct Block** blocks; 
//     int* availableBlocks; 
//     int* filledBlocks; 
//     int memdiskSize; 
//     int blkSize; 
//     int numOfRecords; 
//     int blockAccesses; 
//     int blockAccessReduced; 
//     struct LRUCache* lruCache; 
// };

// struct Address {
//     int blockID; 
//     int offset; 
// };

// struct Record {
//     char GAME_DATE_EST[20];
//     int TEAM_ID_home;
//     int PTS_home;
//     float FG_PCT_home;
//     float FT_PCT_home;
//     float FG3_PCT_home;
//     int AST_home;
//     int REB_home;
//     int HOME_TEAM_WINS; 
// };


// struct Disk* createDisk(int diskSize, int blkSize, int lruCacheSize) {
//     struct Disk* disk = (struct Disk*)malloc(sizeof(struct Disk));
//     disk->memdiskSize = diskSize;
//     disk->blkSize = blkSize;
//     disk->blocks = (struct Block**)malloc(sizeof(struct Block*) * (diskSize / blkSize));
//     disk->availableBlocks = (int*)malloc(sizeof(int) * (diskSize / blkSize));
//     disk->filledBlocks = (int*)malloc(sizeof(int) * (diskSize / blkSize));
//     disk->numOfRecords = 0;
//     disk->blockAccesses = 0;
//     disk->blockAccessReduced = 0;
//     disk->lruCache = createLRUCache(lruCacheSize);
    
//     for (int i = 0; i < diskSize / blkSize; i++) {
//         disk->blocks[i] = createBlock(); 
//         disk->availableBlocks[i] = i;
//     }

//     return disk;
// }

int getFirstAvailableBlockId(struct Disk* disk) {
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        if (disk->availableBlocks[i] != -1) {
            return disk->availableBlocks[i];
        }
    }
    return -1; // No available blocks found
}

// int insertRecord(struct Block* block, struct Record rec) {
//     for (int i = 0; i < MAX_RECORDS; i++) {
//         if (block->recordsList[i].GAME_DATE_EST == 0 &&
//             block->recordsList[i].TEAM_ID_home[0] == '\0' &&
//             block->recordsList[i].PTS_home == 0 &&
//             block->recordsList[i].FG_PCT_home == 0 &&
//             block->recordsList[i].FT_PCT_home == 0 &&
//             block->recordsList[i].FG3_PCT_home == 0 &&
//             block->recordsList[i].AST_home == 0 &&
//             block->recordsList[i].REB_home == 0 &&
//             block->recordsList[i].HOME_TEAM_WINS == 0) {
            
//             // Insert the record
//             block->recordsList[i] = rec;
//             block->curRecords++;
            
//             return i; // Return the offset where the record was inserted
//         }
//     }
//     return -1; // Indicates that the block is full
// }

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

// struct Record getRecord(struct Disk* disk, struct Address add) {
//     struct Block* block = disk->blocks[add.blockID];
//     return getRecordFromBlock(block, add.offset);
// }

// struct Record getRecord(struct Block* block, int offset) {
//     return block->recordsList[offset];
// }


/*void deleteRecord(struct Block* block, int offset) {
    block->recordsList[offset].data = 0; 
    block->curRecords--;
}
*/

// void deleteRecord(struct Block* block, int offset) {
//     // Clear the data fields
//     block->recordsList[offset].GAME_DATE_EST = -1;
//     strcpy(block->recordsList[offset].TEAM_ID_home, ""); // Assuming TEAM_ID_home is a char array
//     block->recordsList[offset].PTS_home = -1;
//     block->recordsList[offset].FG_PCT_home = -1.0f; // Float value
//     block->recordsList[offset].FT_PCT_home = -1.0f; // Float value
//     block->recordsList[offset].FG3_PCT_home = -1.0f; // Float value
//     block->recordsList[offset].AST_home = -1;
//     block->recordsList[offset].REB_home = -1;
//     block->recordsList[offset].HOME_TEAM_WINS = 0; // Assuming HOME_TEAM_WINS is a boolean (0 for false, 1 for true)

//     block->curRecords--;
// }


void deleteRecord(struct Block* block, int offset) {
    // Clear the data fields
    block->recordsList[offset].GAME_DATE_EST = -1;
    block->recordsList[offset].TEAM_ID_home = -1; // Assuming TEAM_ID_home is a char array
    block->recordsList[offset].PTS_home = -1;
    block->recordsList[offset].FG_PCT_home = -1.0f; // Float value
    block->recordsList[offset].FT_PCT_home = -1.0f; // Float value
    block->recordsList[offset].FG3_PCT_home = -1.0f; // Float value
    block->recordsList[offset].AST_home = -1;
    block->recordsList[offset].REB_home = -1;
    block->recordsList[offset].HOME_TEAM_WINS = 0; // Assuming HOME_TEAM_WINS is a boolean (0 for false, 1 for true)

    block->curRecords--;
}



int runBruteForceSearch(struct Disk* disk, int FG_PCT_homeValue, int FG_PCT_homeValueUpperRange) {
    int countBlockAccess = 0;
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        if (disk->filledBlocks[i] == 1) {
            struct Block* block = disk->blocks[i];
            int numberOfRecordsInBlock = getCurSize(block);
            for (int j = 0; j < numberOfRecordsInBlock; j++) {
                countBlockAccess++;
                struct Record rec = getRecordFromBlock(block, j);
                int curFG_PCT_home = rec.FG_PCT_home;
                if (FG_PCT_homeValue <= curFG_PCT_home && curFG_PCT_home <= FG_PCT_homeValueUpperRange) {             
                }
            }
        }
    }
    return countBlockAccess;
}



// void deleteRecord(struct Disk* disk, struct Address* addList, int numRecordsToDelete) {
//     for (int i = 0; i < numRecordsToDelete; i++) {
//         int blockId = addList[i].blockID;
//         int offset = addList[i].offset;
//         struct Block* block = disk->blocks[blockId];
//         deleteRecordInBlock(block, offset);
//         disk->availableBlocks[blockId] = 1;
//     }
// }

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

// void experimentOne(struct Disk* disk) {
//     printf("\n----------------------EXPERIMENT 1-----------------------\n");
//     printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
//     printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
//     //printf("Number of Records Stored in a Block: %d\n", getTotalRecordsInBlock());
//     //printf("Number of Blocks Allocated: %d\n", getNumberBlockUsed(disk));
// }

// int main() {
//     // Example usage
//     struct Disk* disk = createDisk(500000, sizeof(struct Block), 256);
    
//     struct Record rec1 = {"tconst1", 7, 100};
//     struct Record rec2 = {"tconst2", 8, 200};
//     struct Record rec3 = {"tconst3", 6, 50};
    
//     struct Address addr1 = writeRecordToStorage(disk, rec1);
//     struct Address addr2 = writeRecordToStorage(disk, rec2);
//     struct Address addr3 = writeRecordToStorage(disk, rec3);
    
//     experimentOne(disk);
    
//     freeDisk(disk);
    
//     return 0;
// }


// Assume that other structures and functions are already defined...

// void load_data_to_disk(struct Disk* disk, const char* filename) {
//     FILE* file = fopen(filename, "r");
//     if (!file) {
//         printf("Failed to open file: %s\n", filename);
//         return;
//     }

//     char line[256]; // Buffer for each line in the file.
//     fgets(line, sizeof(line), file); // Skip the first line.

//     struct Block* current_block = NULL;
//     int block_index = 0;

//     while (fgets(line, sizeof(line), file) != NULL) {
//         if (!current_block || current_block->curRecords >= MAX_RECORDS) {
//             // Start with a new block.
//             current_block = disk->blocks[block_index++];
//             initBlock(current_block);
//         }

//         struct Record record;

//         sscanf(line, "%19s\t%d\t%d\t%f\t%f\t%f\t%d\t%d\t%d", 
//             record.GAME_DATE_EST, 
//             &record.TEAM_ID_home, 
//             &record.PTS_home,
//             &record.FG_PCT_home, 
//             &record.FT_PCT_home,
//             &record.FG3_PCT_home, 
//             &record.AST_home, 
//             &record.REB_home, 
//             &record.HOME_TEAM_WINS
//         );

//         current_block->recordsList[current_block->curRecords] = record;
//         current_block->curRecords++;
//     }

//     fclose(file);
// }

// int main() {
//     struct Disk* disk = createDisk(500 * 1024 * 1024, MAX_RECORDS, 100); // Example values, you can modify as per your requirements.
//     load_data_to_disk(disk, "/Users/fanyupei/Codes/sc3020/game.txt");
//     // Add other functionalities or clean-up as needed...
//     return 0;
// }