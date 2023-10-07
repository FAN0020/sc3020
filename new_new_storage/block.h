#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>

#define BLOCK_SIZE 400
#define RECORD_SIZE sizeof(struct Record)
#define MAX_RECORDS (int)(BLOCK_SIZE / RECORD_SIZE)

struct Record {
    int GAME_DATE_EST;
    int TEAM_ID_home;
    int PTS_home;
    float FG_PCT_home;
    float FT_PCT_home;
    float FG3_PCT_home;
    int AST_home;
    int REB_home;
    bool HOME_TEAM_WINS; 
};

struct Block{
    int curRecords; 
    struct Record recordsList[MAX_RECORDS]; // Create array to store the records
};

// Function prototypes
struct Block* createBlock();
void initBlock(struct Block* block);
struct Record getRecordFromBlock(struct Block* block, int recordPos);
int getCurSize(struct Block* block);
int isBlockAvailable(struct Block* block);


#endif // BLOCK_H