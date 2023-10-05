#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"

#include "disk.c"
#include "block.c"
#include "LRUCache.c"

// int main() {
//     struct Disk* disk = createDisk(500 * 1024 * 1024, MAX_RECORDS, 100); // Example values, you can modify as per your requirements.
//     load_data_to_disk(disk, "/Users/fanyupei/Codes/sc3020/games.txt");
//     // Add other functionalities or clean-up as needed...
//     //experimentOne(disk);
//     printf("\n----------------------EXPERIMENT 1-----------------------\n");
//     printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
//     printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
//     return 0;
// }

// void experimentOne(struct Disk* disk) {
//     printf("\n----------------------EXPERIMENT 1-----------------------\n");
//     printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
//     printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
//     // printf("Number of Records Stored in a Block: %d\n", getTotalRecordsInBlock());
//     // printf("Number of Blocks Allocated: %d\n", getNumberBlockUsed(disk));
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

    while (fgets(line, sizeof(line), file)) {
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
    }
    
    fclose(file);
    printf("Data loaded into disk.\n");

    printf("\n----------------------EXPERIMENT 1-----------------------\n");
    printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
    printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
    
    // ... Later, when finished using the disk ...
    freeDisk(disk);
    return 0;
}