#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"

struct Block* createBlock() {
    struct Block* block = (struct Block*)malloc(sizeof(struct Block));
    if (block != NULL) {
        block->curRecords = 0;
        // You can initialize other block properties here if needed
    }
    return block;
}

//initialise existing block
void initBlock(struct Block* block) {
    block->curRecords = 0;
}

struct Record getRecordFromBlock(struct Block* block, int recordPos) {
    return block->recordsList[recordPos];
}

int getCurSize(struct Block* block) {
    return block->curRecords; //current no. of records
}

int isBlockAvailable(struct Block* block) {
    return block->curRecords < (int)MAX_RECORDS; 
}