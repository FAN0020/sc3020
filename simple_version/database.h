
#ifndef DATABASE_H
#define DATABASE_H

#include "bt_mgr.h"
#include <string.h>

typedef struct _keylist{
    float key; 
    struct _keylist *next;
}KeyList;

typedef struct _ptrlist{
    double ptr; 
    struct _ptrlist *next;
}PtrList;

void deleteDBKey(BTree *tree, float key);
PtrList* getDataBlocks(BTree *tree, double ptr);
void deleteRecords(PtrList *datablockList,float startKey, float endKey);
KeyList* insertKeyListKey(KeyList *keyList,double newKey);
double findPtrListPtr(PtrList *dataBlockList, double findPtr);
PtrList* insertPtrListPtr(PtrList *dataBlockList,double newPtr);


#endif //DATABASE_H