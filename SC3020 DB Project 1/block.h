#ifndef BLOCK_H
#define BLOCK_H

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

#define MAX_RECORDS 1957764

// Define a structure for a block
typedef struct Block{
    int curRecords; 
    struct Record recordsList[MAX_RECORDS]; // Create array to store the records
} ;

// Function prototypes
struct Block* createBlock();
void initBlock(struct Block* block);
struct Record getRecordFromBlock(struct Block* block, int recordPos);
int getCurSize(struct Block* block);
//int getFirstAvailableBlockId(struct Disk* disk);
int isBlockAvailable(struct Block* block);
//struct Address insertRecordIntoBlock(struct Disk* disk, int blockPtr, struct Record rec);
//struct Record getRecord(struct Disk* disk, struct Address add);
//void deleteRecord(struct Disk* disk, struct Address* addList, int numRecordsToDelete);

#endif // BLOCK_H