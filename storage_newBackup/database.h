
#ifndef DATABASE_H
#define DATABASE_H

#include "BT/bt_mgr.h"
#include "disk.h"
#include <string.h>
#include <stdint.h>
#include <time.h>

// declare global functions and objects. 
clock_t startT; 
clock_t endT;

typedef struct _listNode{
    double value; 
    struct _listNode *next;
}ListNode;

typedef struct _recordNode{
    struct Record record;
    struct _recordNode *next; 
}RecordNode; 


// declare functions
ListNode* searchDBKey(BTree *tree, double key);
ListNode* searchDBRangeKey(BTree *tree, double lowerRange, double upperRange);
RecordNode* retrieveRecords(ListNode* dataPtrs, double lowerRange, double upperRange);
int bruteForceSearch(struct Disk *disk, double lowerRange, double upperRange);
double calculateAverage(RecordNode* records);


int bruteForceDelete(struct Disk *disk, double lowerRange, double upperRange);
void deleteDBKey(BTree *tree, double key);
ListNode* getDataBlocks(BTree *tree, double ptr);
ListNode* getDataBlocksAndDelete(BTree *tree, double ptr);
void deleteRecords(BTree *tree, ListNode *ptrs, double startKey, double endKey);
ListNode* insertListNodeVal(ListNode *list,double newVal);
double findListNodeVal(ListNode *findList, double findVal);


#endif //DATABASE_H