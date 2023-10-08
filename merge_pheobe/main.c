#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"
#include "common.h"
#include "b+tree/bt_page.h"
#include "../simple_version//bt_node.h"
#include "../simple_version/bt_mgr.h"
#include "../simple_version/bt_page.c"
#include "../simple_version/bt_node.c"
#include "../simple_version/bt_mgr.c"

#include "disk.c"
#include "block.c"
#include "LRUCache.c"

#define LEAF_NODE_TYPE 2

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

    //EXPERIMENT 1
    printf("\n----------------------EXPERIMENT 1-----------------------\n");
    printf("Total Number of Records Stored: %d\n", disk->numOfRecords);
    printf("Size of Each Record: %zu Bytes\n", sizeof(struct Record));
    printf("Number of records stored in a block: %d\n", MAX_RECORDS);
    printf("Number of blocks for storing the data: %d\n", getNumberBlockUsed(disk));
    
    // free the memory
    //freeDisk(disk);


    //EXPERIMENT 2
    // • the parameter n of the B+ tree;
    // • the number of nodes of the B+ tree;
    // • the number of levels of the B+ tree;
    // • the content of the root node (only the keys);
    char field[20] = "FG_PCT_home";
    BTree *tree = createTree(field);
    InsertNode* insertInfo = (InsertNode*)malloc(sizeof(InsertNode));
    printf("%s \n","info created");

    // for each data block 
    //     for each data record
    //         insertInfo->key = recordKey
    //         insertInfo->ptr = datablock pointer
    //         insertBTreeKey(tree,insertInfo);
    int counter = 0; 
    for (int i = 0; i < disk->memdiskSize / disk->blkSize; i++) {
        printf("i=%d \n",i);
        if (disk->filledBlocks[i] == 1) { // Ensure block has data before proceeding
            counter++; 
            struct Block* block = getBlock(disk, i);
            int numberOfRecordsInBlock = getCurSize(block);
            
            for (int j = 0; j < numberOfRecordsInBlock; j++) {
                struct Record rec = getRecordFromBlock(block, j);
                insertInfo->key = rec.TEAM_ID_home; // Using TEAM_ID_home as the key
                insertInfo->ptr = i; // The pointer in this context can be the block number. Adjust as needed.
                insertBTreeKey(tree, insertInfo);
            }
        }
        if(counter == getNumberBlockUsed(disk)) break;
    }
    printf("\n----------------------EXPERIMENT 2-----------------------\n");
    printf("Parameter n of the B+ tree: %d\n", N);
    // Get number of nodes by traversing the tree
    int nodeCount = countNodes(tree->root, tree->page);
    printf("Number of nodes of the B+ tree: %d\n", nodeCount);
    // // Get number of levels
    int levelCount = countLevels(tree->root, tree->page);
    printf("Number of levels of the B+ tree: %d\n", levelCount);
    // // Print content of the root node
    // printf("Keys in the root node: ");
    // if (tree->root != -1) {
    //     NonLeafNode *rootNode = (NonLeafNode*)searchPageRecord(page, tree->root);
    //     for (int i = 0; i < N && rootNode->keys[i] != 0; i++) {
    //         printf("%f ", rootNode->keys[i]);
    //     }
    // }
    printf("\n");

    printf("\n----------------------EXPERIMENT 3-----------------------\n");
    search_counter = 0;
    double something = searchBTreeKey(tree, 0.5);
    printf("%d", search_counter);


    return 0;
}