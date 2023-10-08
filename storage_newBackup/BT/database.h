
#ifndef DATABASE_H
#define DATABASE_H

#include "bt_mgr.h"
#include <string.h>
#include <stdint.h>
typedef struct _listNode{
    double value; 
    struct _listNode *next;
}ListNode;

void deleteDBKey(BTree *tree, double key);
ListNode* getDataBlocks(BTree *tree, double ptr);
void deleteRecords(BTree *tree, ListNode *datablockList, double startKey, double endKey);
ListNode* insertListNodeVal(ListNode *list,double newVal);
double findListNodeVal(ListNode *findList, double findVal);


#endif //DATABASE_H