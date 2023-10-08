#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"

#include "disk.c"
#include "block.c"
#include "LRUCache.c"

int main() {
    //create a Disk
    int diskSize = 500 * 1024 * 1024; // 500 MB
    int blkSize = 400; // 400 B
    int lruCacheSize = 50; // Example size
    struct Disk* disk = createDisk(diskSize, blkSize, lruCacheSize);

    //Load the data
    if (!loadData(disk, "games.txt")) {
        freeDisk(disk);
        return 1;
    }
    //visualizeDisk(disk);

<<<<<<< HEAD
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

        // Parse the date string and convert it to an integer
        int day, month, year;
        sscanf(line, "%d/%d/%d %d %d %f %f %f %d %d %s", &day, &month, &year, &record.TEAM_ID_home,
               &record.PTS_home, &record.FG_PCT_home, &record.FT_PCT_home,
               &record.FG3_PCT_home, &record.AST_home, &record.REB_home,
               &record.HOME_TEAM_WINS);

        // Convert the date to a single integer (e.g., 20220425)
        record.GAME_DATE_EST = (year * 10000) + (month * 100) + day;

        // Step 3: Populate the Disk
        if (currentBlock->curRecords >= (int)(MAX_RECORDS)) {  // Assuming average of 7 records per block
            currentBlockId++;
            currentBlock = getBlock(disk, currentBlockId);
        }

        if (currentBlock->curRecords >= (int)(MAX_RECORDS)) {
            printf("Block is full. This shouldn't happen.\n");
            return 1;
        }


        writeRecordToStorage(disk, record);
        
        // Debug
        //printf("Record No.: %d, Stored in Block: %d\n", disk->numOfRecords, currentBlockId);

       
    }
    
    fclose(file);
    printf("Data loaded into disk.\n");

=======
    //EXPERIMENT 1
>>>>>>> main
    printf("\n----------------------EXPERIMENT 1-----------------------\n");
    printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
    printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
    printf("Number of records stored in a block: %d\n", MAX_RECORDS);
    printf("Number of blocks for storing the data: %d\n", getNumberBlockUsed(disk));
    
    // free the memory
    freeDisk(disk);
    return 0;
}