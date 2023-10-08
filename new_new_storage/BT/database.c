#include "bt_mgr.h"
#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
    PtrList* ptrs = getDataBlocks(tree, resultPtr);

    printf("Datablock List\n");
    PtrList* curPtr = ptrs;
    while(curPtr!=NULL){
        printf("%f ", curPtr->ptr);
        curPtr=curPtr->next;
    }
    printf("\n");
    // delete records.

    deleteRecords(ptrs,key,key);

}

// void deleteDBRangeKey(){
//     // get datablocks of corresponding key.
//     PtrList* ptrs = getDataBlocks(tree, resultPtr, key);

//     // delete records.
//     deleteRecords(ptrs);
// }

/**
 * A function to get the datablocks containing data records to be deleted.
 * @param tree the tree containing the records.
 * @param ptr the pointer to the datablocks found in the B+ tree (either a datablock pointer or an overflow node pointer)
 * @return returns the list of datablocks.
*/
PtrList* getDataBlocks(BTree *tree, double ptr){
    // creating list of datablocks to delete records from.
    PtrList *ptrs = NULL; // linked list storing all the keys.

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
                if(curNode->dataBlocks[i] != -1 & !findPtrListPtr(ptrs, curNode->dataBlocks[i])){
                    ptrs = insertPtrListPtr(ptrs,curNode->dataBlocks[i]);
                }
            }
            deleteOverflowNode(tree->page,curNode,curNode); // delete overflow node.
            curNode=nextNode;
        }
    }
    // enter lone datablock into list
    else{
        insertPtrListPtr(ptrs,resultPtr);
    }

    return ptrs;
}

/**
 * A function to loop through a list of datablocks and delete data records.
 * @param datablockList list of datablocks to loop through.
 * @param startKey the starting range of keys of records to be deleted.
 * @param endKey the ending range of keys of records to be deleted.
*/
void deleteRecords(PtrList *datablockList, double startKey, double endKey){
    // delete records from data block.
    PtrList *curBlock = datablockList, *nxtBlock;
    while(curBlock!=NULL){
        printf("%f \n", curBlock->ptr);
        nxtBlock = curBlock->next;
        // EDIT: STORAGE SIDE NOW: delete record in datablock(curPtr->ptr contains the datablock id)

        // END OF EDIT.
        free(curBlock);
        curBlock = nxtBlock;
    }
}


/**
 * A function to add a key to an existing list. Will be added to start
 * @param keyList existing list of keys, used to contain list of keys to be deleted
 * @param newKey key to be added into list.
*/
KeyList* insertKeyListKey(KeyList *keyList,double newKey){
    KeyList *insertBlock = (KeyList*)malloc(sizeof(KeyList));
    insertBlock->key = newKey;
    insertBlock->next = keyList;
    return insertBlock;
}

/**
 * A function check if the ptr is already in the list to avoid duplicate access to datablock.
 * @param dataBlockList the list of pointers
 * @param findPtr the pointer we're looking for in the list.
 * @return returns binary, 1 if pointer is found, 0 if not found.
*/
double findPtrListPtr(PtrList *dataBlockList, double findPtr){
    if(dataBlockList == NULL) return 0;
    else{
        if(dataBlockList->ptr == findPtr) return 1;
        else findPtrListPtr(dataBlockList->next, findPtr);
    }
}

/**
 * A function to add a datablock pointer to an existing list. Will be added to start
 * @param dataBlockList existing list of datablock from which records are to be deleted from
 * @param newPtr datablock pointer to be added into list.
*/
PtrList* insertPtrListPtr(PtrList *dataBlockList,double newPtr){
    PtrList *insertBlock = (PtrList*)malloc(sizeof(PtrList));
    insertBlock->ptr = newPtr;
    insertBlock->next = dataBlockList;
    return insertBlock;
}