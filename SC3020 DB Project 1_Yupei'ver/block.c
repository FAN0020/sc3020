#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECORDS 400

struct Address {
    int blockID; 
    int offset;  
};

struct Record {
    char GAME_DATE_EST[20];
    int TEAM_ID_home;
    int PTS_home;
    float FG_PCT_home;
    float FT_PCT_home;
    float FG3_PCT_home;
    int AST_home;
    int REB_home;
    int HOME_TEAM_WINS; 
};

struct Block {
    int curRecords; 
    struct Record recordsList[MAX_RECORDS]; // Create array to store the records
};

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
    return block->curRecords < MAX_RECORDS; 
}


/*struct Record getRecord(struct Block* block, int offset) {
    return block->recordsList[offset];
}


void deleteRecord(struct Block* block, int offset) {
    block->recordsList[offset].data = 0; 
    block->curRecords--;
}
*/

/*int main() {
    struct Block block;
    initBlock(&block);

    struct Record record1;
    record1.data = 42;
    struct Record record2;
    record2.data = 99;

    int offset1 = insertRecordIntoBlock(&block, record1);
    int offset2 = insertRecordIntoBlock(&block, record2);

    printf("Current size of the block: %d\n", getCurSize(&block));
    printf("Record 1 data: %d\n", getRecord(&block, offset1).data);
    printf("Record 2 data: %d\n", getRecord(&block, offset2).data);

    deleteRecord(&block, offset1);
    printf("After deleting record 1, current size of the block: %d\n", getCurSize(&block));

    return 0;
}
*/