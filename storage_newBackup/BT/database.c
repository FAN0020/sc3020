#include "bt_mgr.h"
#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
/**
 * A function to delete keys and records in the database 
 * @param tree the tree containing the index
 * @param key the key value we're trying to delete.
*/
void deleteDBKey(BTree *tree, double key){
    // creating deletion information
    DeleteNode *deleteInfo = (DeleteNode*)malloc(sizeof(DeleteNode));
    deleteInfo->key = key;
    deleteInfo->ptr = -2; //representing deleting all records with this key. 
    // creating update Info placeholder.
    UpdateNode *updateInfo = (UpdateNode*) malloc(sizeof(UpdateNode));

    // search and delete key in the tree
    deleteBTreeKey(tree,updateInfo,deleteInfo);
    // deletion returns the pointer to the data to delete (overflow node or datablock id)
    // results are stored in global variable resultPtr. 
    if(resultPtr == -1){
        printf("No records found. No records deleted.\n");
        return;
    }

    // get datablocks of corresponding key.
    ListNode* ptrs = getDataBlocks(tree, resultPtr);

    printf("Datablock List\n");
    ListNode* curPtr = ptrs;
    while(curPtr!=NULL){
        printf("%f ", curPtr->value);
        curPtr=curPtr->next;
    }
    printf("\n");
    // delete records.

    deleteRecords(tree,ptrs,key,key);
    
    free(updateInfo);
    free(deleteInfo);
}

/**
 * A function to delete all records within a range of keys. 
 * 
*/
void deleteDBRangeKey(BTree *tree, double startKey, double endKey){
    // get the keys within the range.
    ListNode* keys = NULL; 
    ListNode* ptrs = NULL; 
    double ptr = searchBTreeRangeKey(tree,startKey); // get first leaf node containing range keys.
    int nodeType = searchPageRecord(tree->page, ptr);
    if (nodeType != 2) {
        printf("No records found. No records deleted.\n");
        return;
    }
    int foundAllKeys = 0; // binary value of whether all range keys have been retrieved.
    LeafNode* lNode = (LeafNode*)(uintptr_t)lNode, *curNode; 
    curNode = lNode; 
    // loop through leaf node to find keys within range. 
    while(curNode != NULL & !foundAllKeys){
        for(int i = 0; i<N; i++){
            if(curNode->keys[i] == -1) break; 
            else if (curNode->keys[i] >= startKey & curNode->keys[i] <= endKey){
                keys = insertListNodeVal(keys,curNode->keys[i]);
            }
            else if (curNode->keys[i] > endKey){
                foundAllKeys = 1; 
                break; 
            }
        }
    }

    // retrieve the datablocks
    // creating deletion information
    DeleteNode *deleteInfo = (DeleteNode*)malloc(sizeof(DeleteNode));
    deleteInfo->key = 0;
    deleteInfo->ptr = -2; //representing deleting all records with this key. 
    // creating update Info placeholder.
    UpdateNode *updateInfo = (UpdateNode*) malloc(sizeof(UpdateNode));

    // delete key from

    // ListNode* ptrs = getDataBlocks(tree, resultPtr);

    // // delete records.
    // deleteRecords(ptrs);
}

/**
 * A function to get the datablocks containing data records to be deleted. 
 * @param tree the tree containing the records. 
 * @param ptr the pointer to the datablocks found in the B+ tree (either a datablock pointer or an overflow node pointer)
 * @return returns the list of datablocks.
*/
ListNode* getDataBlocks(BTree *tree, double ptr){
    // creating list of datablocks to delete records from.
    ListNode *ptrs = NULL; // linked list storing all the keys.

    // retrieve all ptrs and deleting the overflow node after.
    int nodeType = searchPageRecord(tree->page,ptr);
    if(nodeType == 3){
        OverflowNode *curNode = (OverflowNode*)(uintptr_t)ptr, *nextNode;
        while(curNode != NULL){
            // update IO count for accessing the overflow nodes. 
            ioCount++; 
            // add datablocks into the list.
            nextNode = curNode->next;
            for(int i=0; i<OVERFLOW_RECS ;i++){
                // if valid datablock and datablock not in list, add into list.
                if(curNode->dataBlocks[i] != -1 & !findListNodeVal(ptrs, curNode->dataBlocks[i])){
                    ptrs = insertListNodeVal(ptrs,curNode->dataBlocks[i]);
                }
            }
            deleteOverflowNode(tree->page,curNode,curNode); // delete overflow node.
            curNode=nextNode;
        }
    }
    // enter lone datablock into list
    else{
        insertListNodeVal(ptrs,resultPtr);
    }

    return ptrs; 
}

/**
 * A function to loop through a list of datablocks and delete data records.
 * @param tree object containing details of the tree 
 * @param datablockList list of datablocks to loop through.
 * @param startKey the starting range of keys of records to be deleted.
 * @param endKey the ending range of keys of records to be deleted.
*/
void deleteRecords(BTree *tree, ListNode *datablockList, double startKey, double endKey){
    // delete records from data block. 
    ListNode *curBlock = datablockList, *nxtBlock;
    struct Block* block = NULL; 
    while(curBlock!=NULL){
        nxtBlock = curBlock->next;
        // EDIT: STORAGE SIDE NOW: delete record in datablock(curPtr->ptr contains the datablock id)
        // block = (struct Block*)(uintptr_t)(curBlock->ptr);
        // for(int i=0;i<MAX_RECORDS;i++){
        //     struct Record rec = getRecordFromBlock(block, i);
        //     if(rec.FG_PCT_home <= startKey & rec.FG_PCT_home >= endKey){
        //         deleteRecord(block,i);
        //     }
        // }
        // END OF EDIT. 
        free(curBlock); 
        curBlock = nxtBlock;
    }
}

/**
 * A function check if value already exists in the list.
 * @param list the list of values (can be keys or pointers)
 * @param findVal the value we're looking for in the list. 
 * @return returns binary, 1 if value is found, 0 if not found.
*/
double findListNodeVal(ListNode *list, double findval){
    if(list == NULL) return 0; 
    else{
        if(list->value == findval) return 1;
        else findListNodeVal(list->next, findval);
    }
}

/**
 * A function to add a double value to an existing list. 
 * @param list existing list of values (can be keys or pointers)
 * @param newVal new value to be added into list. 
*/
ListNode* insertListNodeVal(ListNode *list,double newVal){
    ListNode *insertNode = (ListNode*)malloc(sizeof(ListNode));
    insertNode->value = newVal;
    insertNode->next = list;
    return insertNode;

}