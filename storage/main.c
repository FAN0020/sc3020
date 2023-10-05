#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"

#include "disk.c"
#include "block.c"
#include "LRUCache.c"


// int loadDataIntoDisk(struct Disk* disk) {
//     FILE *file = fopen("games.txt", "r");
//     if (file == NULL) {
//         printf("Failed to open the file.\n");
//         return -1;
//     }

//     char line[255];
//     fgets(line, sizeof(line), file); // Skip header line

//     int currentBlockId = 0; 
//     struct Block* currentBlock = getBlock(disk, currentBlockId);

//     // Initialize the filledBlocks pointer and allocate memory for it
//     disk->filledBlocks = (int*) malloc(sizeof(int) * (disk->memdiskSize / disk->blkSize));
//     int filledBlocksIndex = 0;

//     int lineCounter = 0;  // To track the line number

//     while (fgets(line, sizeof(line), file)) {
//         lineCounter++;
//         // Debug
//         printf("Line %d: %s", lineCounter, line);

//         // Parse the line to create a record
//         struct Record record;
//         sscanf(line, "%s %d %d %f %f %f %d %d %d", record.GAME_DATE_EST, &record.TEAM_ID_home, 
//                &record.PTS_home, &record.FG_PCT_home, &record.FT_PCT_home, 
//                &record.FG3_PCT_home, &record.AST_home, &record.REB_home, 
//                &record.HOME_TEAM_WINS);

//         // Populate the Disk
//         if (currentBlock->curRecords >= 7) {
//             disk->filledBlocks[filledBlocksIndex++] = currentBlockId;
//             currentBlockId++;
//             currentBlock = getBlock(disk, currentBlockId);
//         }

//         if (currentBlock->curRecords >= MAX_RECORDS) {
//             printf("Block is full. This shouldn't happen.\n");
//             free(disk->filledBlocks);
//             fclose(file);
//             return -1;
//         }

//         writeRecordToStorage(disk, record);
//     }
    
//     fclose(file);
//     printf("Data loaded into disk.\n");
//     return 0;
// }

// int main() {
//     // Step 1: Create a Disk
//     int diskSize = 500 * 1024 * 1024; // 500 MB
//     int blkSize = 400; // 400 B
//     int lruCacheSize = 50; // Example size
//     struct Disk* disk = createDisk(diskSize, blkSize, lruCacheSize);
    
//     // Step 2: Load data into disk
//     if (loadDataIntoDisk(disk) == -1) {
//         freeDisk(disk);
//         return 1; // Some error occurred
//     }

//     printf("\n----------------------EXPERIMENT 1-----------------------\n");
//     printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
//     printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
//     printf("Number of records stored in a block: %d\n", (disk->filledBlocks[0] == 0 ? getBlock(disk, disk->filledBlocks[0])->curRecords : 7));
//     printf("Number of blocks for storing the data: %d\n", disk->filledBlocks[0] + 1); // +1 since it's 0-indexed
    
//     // ... Later, when finished using the disk ...
//     free(disk->filledBlocks);
//     freeDisk(disk);
//     return 0;
// }



int main() {
    // Step 1: Create a Disk
    int diskSize = 500 * 1024 * 1024; // 500 MB
    int blkSize = 400; // 400 B
    int lruCacheSize = 50; // Example size
    struct Disk* disk = createDisk(diskSize, blkSize, lruCacheSize);
    
    // Step 2: Load the file
    FILE *file = fopen("games.txt", "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }

    char line[255];
    fgets(line, sizeof(line), file); // Skip header line

    int currentBlockId = 0; 
    struct Block* currentBlock = getBlock(disk, currentBlockId);

    int lineCounter = 0;  // To track the line number

    while (fgets(line, sizeof(line), file)) {
        lineCounter++;

        // Debug
        printf("Line %d: %s", lineCounter, line);

        // Parse the line to create a record
        struct Record record;
        sscanf(line, "%s %d %d %f %f %f %d %d %d", record.GAME_DATE_EST, &record.TEAM_ID_home, 
               &record.PTS_home, &record.FG_PCT_home, &record.FT_PCT_home, 
               &record.FG3_PCT_home, &record.AST_home, &record.REB_home, 
               &record.HOME_TEAM_WINS);

        // Step 3: Populate the Disk
        if (currentBlock->curRecords >= 7) {  // Assuming average of 7 records per block
            currentBlockId++;
            currentBlock = getBlock(disk, currentBlockId);
        }

        if (currentBlock->curRecords >= MAX_RECORDS) {
            printf("Block is full. This shouldn't happen.\n");
            return 1;
        }

        writeRecordToStorage(disk, record);
        
        // Debug
        //printf("Record No.: %d, Stored in Block: %d\n", disk->numOfRecords, currentBlockId);

       
    }
    
    fclose(file);
    printf("Data loaded into disk.\n");

    printf("\n----------------------EXPERIMENT 1-----------------------\n");
    printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
    printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
    printf("Number of records stored in a block: %d\n", (currentBlockId == 0 ? currentBlock->curRecords : 7)); // Assuming average of 7 records/block
    printf("Number of blocks for storing the data: %d\n", currentBlockId + 1); // +1 since it's 0-indexed
    
    // ... Later, when finished using the disk ...
    freeDisk(disk);
    return 0;
}

