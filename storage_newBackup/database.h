
#ifndef DATABASE_H
#define DATABASE_H

#include "BT/bt_mgr.h"
#include <string.h>
#include <stdint.h>
#include <time.h>

// declare global functions and objects. 
clock_t startT = 0; 
clock_t endT = 0;

typedef struct _listNode{
    double value; 
    struct _listNode *next;
}ListNode;


// declare functions
void deleteDBKey(BTree *tree, double key);
ListNode* getDataBlocks(BTree *tree, double ptr);
void deleteRecords(BTree *tree, ListNode *ptrs, double startKey, double endKey);
ListNode* insertListNodeVal(ListNode *list,double newVal);
double findListNodeVal(ListNode *findList, double findVal);


#endif //DATABASE_H