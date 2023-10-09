#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"
#include "database.h"

#include "disk.c"
#include "block.c"
#include "LRUCache.c"
#include "database.c"
#include <stdint.h>
//#include "common.h"
#include "BT/bt_page.h"
#include "BT/bt_node.h"
#include "BT/bt_mgr.h"
#include "BT/bt_page.c"
#include "BT/bt_node.c"
#include "BT/bt_mgr.c"


int main() {
    //create a Disk
    int diskSize = 500 * 1024 * 1024; // 500 MB
    int blkSize = 400; // 400 B
    int lruCacheSize = 50 * 1024 * 1024; // Example size
    struct Disk* disk = createDisk(diskSize, blkSize, lruCacheSize);

    //Load the data
    if (!loadData(disk, "games.txt")) {
        freeDisk(disk);
        return 1;
    }
    //visualizeDisk(disk);

    //EXPERIMENT 1
    printf("\n----------------------EXPERIMENT 1-----------------------\n");
    printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
    printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
    printf("Number of records stored in a block: %d\n", MAX_RECORDS);
    printf("Number of blocks for storing the data: %d\n", getNumberBlockUsed(disk));
    //printRecords(disk, 100);
    
    //EXPERIMENT 2
    // • the parameter n of the B+ tree;
    // • the number of nodes of the B+ tree;
    // • the number of levels of the B+ tree;
    // • the content of the root node (only the keys);
    char field[20] = "FG_PCT_home";
    BTree *tree = createTree(field);
    InsertNode* insertInfo = (InsertNode*)malloc(sizeof(InsertNode));
    //printf("%s \n","info created");
    int counter = 0; 
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        //printf("i=%d \n",i);
        if (disk->filledBlocks[i] == 1) { // Ensure block has data before proceeding
            counter++; 
            struct Block* block = getBlock(disk, i);
            int numberOfRecordsInBlock = getCurSize(block);
            
            for (int j = 0; j < numberOfRecordsInBlock; j++) {
                struct Record rec = getRecordFromBlock(block, j);
                //insertInfo->key = rec.TEAM_ID_home; // Using TEAM_ID_home as the key
                insertInfo->key = rec.FG_PCT_home;
                //insertInfo->ptr = i; // The pointer in this context can be the block number. Adjust as needed.
                insertInfo->ptr = (double)(uintptr_t)(block);
                //printf("Inserting key: %f into the B+ tree...\n", insertInfo->key);
                insertBTreeKey(tree, insertInfo);
                //printf("Root of the B+ tree after insertion: %f\n", tree->root);

                //printf("%f, %f\n",insertInfo->key, insertInfo->ptr);
            }
        }
        if(counter == getNumberBlockUsed(disk)) break;
    }
    printf("\n----------------------EXPERIMENT 2-----------------------\n");
    printf("Parameter n of the B+ tree: %ld\n", (int)N);
    int nodeCount = countNode(tree);   // Get number of nodes by traversing the tree
    printf("Number of nodes of the B+ tree: %d\n", nodeCount);
    int levelCount = countLevel(tree->page, tree->root); // // Get number of levels
    //int levelCount = countLevel(tree->root, tree->page);
    printf("Number of levels of the B+ tree: %d\n", levelCount);
    printf("Root keys of the B+ tree: ");
    printRootKeys(tree);
    //printBTree(tree); // print the whole tree to debug
    printf("\n");

    printf("\n----------------------EXPERIMENT 3-----------------------\n");
    ListNode* dataPtrs = searchDBKey(tree,0.5);
    clock_t retrieveDataBlockT = endT - startT; 
    printf("Number of index nodes the process accessed: %d\n",ioCount);
    RecordNode* records = retrieveRecords(dataPtrs,0.5,0.5);
    clock_t retrieveRecordsT = endT - startT; 
    printf("Number of datablocks the process accessed: %d\n",ioCount);
    printf("The average of 'FG3_PCT_home' of the records: %f\n",calculateAverage(records));
    printf("The running time of the retrieval process(B+ Tree): %lu\n",retrieveDataBlockT+retrieveRecordsT);
    printf("Number of datablocks the process accessed by brute-force: %d\n",bruteForceSearch(disk,0.5,0.5));
    printf("The running time of the retrieval process(brute-force): %lu\n",endT-startT);
    
    printf("\n----------------------EXPERIMENT 4-----------------------\n");
    dataPtrs = searchDBRangeKey(tree,0.6,1);
    retrieveDataBlockT = endT - startT; 
    printf("Number of index nodes the process accessed: %d\n",ioCount);
    records = retrieveRecords(dataPtrs,0.6,1);
    retrieveRecordsT = endT - startT; 
    printf("Number of datablocks the process accessed: %d\n",ioCount);
    printf("The average of 'FG3_PCT_home' of the records: %f\n",calculateAverage(records));
    printf("The running time of the retrieval process(B+ Tree): %lu\n",retrieveDataBlockT+retrieveRecordsT);
    printf("Number of datablocks the process accessed by brute-force: %d\n",bruteForceSearch(disk,0.6,1));
    printf("The running time of the retrieval process(brute-force): %lu\n",endT-startT);
    

    printf("\n----------------------EXPERIMENT 5-----------------------\n");
    // execute the delete.
    deleteDBRangeKey(tree,0,0.35);
    printf("Number of nodes of the updated B+ Tree: %d\n",countNode(tree));
    printf("Number of nodes of the updated B+ Tree: %d\n",countLevel(tree->page, tree->root));
    printf("Root keys of the updated B+ tree: ");
    printRootKeys(tree);
    //printBTree(tree); // print the whole tree to debug
    printf("The running time of the process: %lu, (IO count = %d)\n",endT-startT,ioCount);

    printf("Number of datablock accessed by a brute-force method: %d\n",bruteForceDelete(disk,0,0.35));
    printf("The running time of the process: %lu\n",endT-startT);


    // free the memory
    deleteBTree(tree);
    freeDisk(disk);
    return 0;   
}


void printRecords(struct Disk* disk, int numRecordsToPrint) {
    int recordsPrinted = 0;
    for (int i = 0; i < disk->memdiskSize / BLOCK_SIZE && recordsPrinted < numRecordsToPrint; i++) {
        struct Block* currentBlock = disk->blocks[i];
        if (currentBlock == NULL) {
            continue;  // skip any uninitialized blocks
        }
        
        for (int j = 0; j < currentBlock->curRecords && recordsPrinted < numRecordsToPrint; j++) {
            struct Record record = currentBlock->recordsList[j];
            printf("Record %d:\n", recordsPrinted + 1);
            printf("GAME_DATE_EST: %d\n", record.GAME_DATE_EST);
            printf("TEAM_ID_home: %d\n", record.TEAM_ID_home);
            printf("PTS_home: %d\n", record.PTS_home);
            printf("FG_PCT_home: %f\n", record.FG_PCT_home);
            printf("FT_PCT_home: %f\n", record.FT_PCT_home);
            printf("FG3_PCT_home: %f\n", record.FG3_PCT_home);
            printf("AST_home: %d\n", record.AST_home);
            printf("REB_home: %d\n", record.REB_home);
            printf("HOME_TEAM_WINS: %s\n\n", record.HOME_TEAM_WINS ? "YES" : "NO");
            
            recordsPrinted++;
        }
    }
}

