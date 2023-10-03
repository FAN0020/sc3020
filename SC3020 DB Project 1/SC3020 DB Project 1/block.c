#include <stdio.h>

#define MAX_RECORDS 1957764

struct Record {
    int data; 
};

struct Block {
    int curRecords; 
    struct Record recordsList[MAX_RECORDS]; // Create array to store the records
};

//initialise block
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

int insertRecordIntoBlock(struct Block* block, struct Record rec) { //insert record in free space
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (block->recordsList[i].data == 0) {
            block->recordsList[i] = rec;
            block->curRecords++;
            return i;
        }
    }
    return -1;
}

struct Record getRecord(struct Block* block, int offset) {
    return block->recordsList[offset];
}

void deleteRecord(struct Block* block, int offset) {
    block->recordsList[offset].data = 0; 
    block->curRecords--;
}

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