#include "BT/bt_mgr.h"
#include "database.h"
#include "disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// functions 

/**
 * A function to retrieve list of datablocks of a key.
 * @param tree the index tree of the database. 
 * @param key the key of the records we are trying to retrieve.
 * @return returns the linked list of the datablocks
*/
ListNode* searchDBKey(BTree *tree, double key){
    ioCount = 0;
    startT=clock(); // start time
    // retrieve datablocks.
    double ptr = searchBTreeKey(tree,key);
    ListNode* dataPtrs = getDataBlocks(tree,ptr), *nxtPtr;
    endT=clock(); // end time
    return dataPtrs;
}

/**
 * A function to retrieve list of datablocks of a range of key
 * @param tree the index tree of the database. 
 * @param lowerRange lowerRange of the keys of records to retrieve.
 * @param upperRange upperRange of the keys of records to retrieve.
 * @return returns the linked list of the datablocks
*/
ListNode* searchDBRangeKey(BTree *tree, double lowerRange, double upperRange){
    ioCount = 0;
    startT=clock(); // start time
    // retrieve datablocks.
    double ptr = searchBTreeRangeKey(tree,lowerRange);

    int nodeType = searchPageRecord(tree->page, ptr);
    if (nodeType != 2) {
        printf("No records found.\n");
        return NULL;
    }
    ListNode* ptrs = NULL; 
    int foundAllKeys = 0; // binary value of whether all range keys have been retrieved.
    LeafNode* lNode = (LeafNode*)(uintptr_t)ptr, *curNode; 
    curNode = lNode; 
    // loop through leaf node to find keys within range and get their pointer
    while(curNode != NULL & !foundAllKeys){
        ioCount++;
        for(int i = 0; i<N; i++){
            if(curNode->keys[i] == -1) break; 
            else if (curNode->keys[i] >= lowerRange & curNode->keys[i] <= upperRange){
                ptrs = insertListNodeVal(ptrs,curNode->ptrs[i]);
            }
            else if (curNode->keys[i] > upperRange){
                foundAllKeys = 1; 
                break; 
            }
        }
        curNode = curNode->next;
    }

    // get datablock pointers.
    ListNode *dataPtrs=NULL, *curPtr = ptrs, *nxtPtr = NULL, *keyPtrs = NULL,*nxtKeyPtr=NULL; 
    while(curPtr != NULL){
        nxtPtr = curPtr->next;

        // find datablocks of each key.
        keyPtrs = getDataBlocks(tree, curPtr->value);

        // add pointers of key into the overall list, dataPtrs.
        while(keyPtrs != NULL){
            nxtKeyPtr = keyPtrs->next;
            if(findListNodeVal(dataPtrs,keyPtrs->value) == 0){
                dataPtrs = insertListNodeVal(dataPtrs,keyPtrs->value);
            }
            free(keyPtrs);
            keyPtrs = nxtKeyPtr;
        }
        free(curPtr);
        curPtr = nxtPtr;
    }
    endT=clock(); // end time
    return dataPtrs;
}

/**
 * A function to retrieve records of a key. 
 * @param dataPtrs linkedlist of all the datablocks containing the records. 
 * @param lowerRange lowerRange of the keys of records to retrieve.
 * @param upperRange upperRange of the keys of records to retrieve.
 * @return returns the linked list of the records.
*/
RecordNode* retrieveRecords(ListNode* dataPtrs, double lowerRange, double upperRange){
    ioCount = 0;
    startT=clock(); // start time
    // retrueve records from datablocks.
    RecordNode *records = NULL, *newRecord; 
    ListNode *nxtPtr;
    while(dataPtrs != NULL){
        ioCount++; // count the datablocks accessed. 
        nxtPtr = dataPtrs->next;
        struct Block* block = (struct Block*)(uintptr_t)(dataPtrs->value);
        // retrieve records within range.
        for(int i=0;i<MAX_RECORDS;i++){
            struct Record rec = getRecordFromBlock(block, i);
            if(lowerRange <= rec.FG_PCT_home & rec.FG_PCT_home<= upperRange){
                newRecord = (RecordNode*)malloc(sizeof(RecordNode));
                newRecord->record = rec; 
                newRecord->next = records; 
                records = newRecord;
            }
        }
        free(dataPtrs);
        dataPtrs = nxtPtr;
    }
    endT=clock(); // end time
    return records;
}

/**
 * A function to simulate bruteforce search
 * @param disk object that contains all the data. 
 * @param lowerRange the lower range of the search keys. 
 * @param upperRange the upper range of the search keys. 
 * @return returns the number of blocks accessed. 
*/
int bruteForceSearch(struct Disk *disk, double lowerRange, double upperRange){
    startT=clock();
    int count = runBruteForceSearch(disk,lowerRange,upperRange);
    endT=clock();
    return count; 
}

/**
 * A function to calculate average of the FG3_PCT_home
 * @param records list of records retrieved previously to apply function to. 
 * @return the average value.
*/
double calculateAverage(RecordNode* records){
    double count=0, sum=0;
    RecordNode* nxtRec;
    while(records!=NULL){
        nxtRec = records->next;
        sum += (records->record).FG3_PCT_home;
        count++; 
        free(records);
        records = nxtRec; 
    }
    return (sum/count);
}

/**
 * A function to simulate bruteforce deletion.
 * @param disk object that contains all the data. 
 * @param lowerRange the lower range of the search keys. 
 * @param upperRange the upper range of the search keys. 
 * @return returns the number of blocks accessed. 
*/
int bruteForceDelete(struct Disk *disk, double lowerRange, double upperRange){
    startT = clock();
    int count = runBruteForceDelete(disk,lowerRange, upperRange);
    endT=clock();
    return count;
}

/**
 * A function to delete keys and records in the database 
 * @param tree the tree containing the index
 * @param key the key value we're trying to delete.
*/
void deleteDBKey(BTree *tree, double key){
    startT = clock();
    ioCount = 0; // reset IO count
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
    ListNode* ptrs = getDataBlocksAndDelete(tree, resultPtr);

    // delete records.
    deleteRecords(tree,ptrs,key,key);
    
    free(updateInfo);
    free(deleteInfo);

    endT=clock();
}

/**
 * A function to delete all records within a range of keys. 
 * 
*/
void deleteDBRangeKey(BTree *tree, double startKey, double endKey){
    ioCount=0;
    startT = clock();
    // get the keys within the range.
    ListNode* keys = NULL, *ptrs = NULL, *keyPtrs = NULL; 
    double ptr = searchBTreeRangeKey(tree,startKey); // get first leaf node containing range keys.
    int nodeType = searchPageRecord(tree->page, ptr);
    if (nodeType != 2) {
        printf("No records found. No records deleted.\n");
        return;
    }
    int foundAllKeys = 0; // binary value of whether all range keys have been retrieved.
    LeafNode* lNode = (LeafNode*)(uintptr_t)ptr, *curNode; 
    curNode = lNode; 
    // loop through leaf node to find keys within range. 
    while(curNode != NULL & !foundAllKeys){
        ioCount++;
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
        curNode = curNode->next;
    }

    // retrieve the datablocks
    // creating deletion information
    DeleteNode *deleteInfo = (DeleteNode*)malloc(sizeof(DeleteNode));
    deleteInfo->key = 0;
    deleteInfo->ptr = -2; //representing deleting all records with this key. 
    // creating update Info placeholder.
    UpdateNode *updateInfo = (UpdateNode*) malloc(sizeof(UpdateNode));

    // delete keys from BTree and get all datablocks with records. 
    ListNode *curKey = keys, *nxtKey = NULL, *nxtKeyPtr=NULL; 
    while(curKey != NULL){
        
        nxtKey = curKey->next;
        deleteInfo->key = curKey->value;
        deleteInfo->ptr = -2; 
        deleteBTreeKey(tree,updateInfo,deleteInfo);

        // find datablocks of each key.
        keyPtrs = getDataBlocksAndDelete(tree, resultPtr);

        // add pointers of key into the overall list, ptrs.
        while(keyPtrs != NULL){
            nxtKeyPtr = keyPtrs->next;
            if(findListNodeVal(ptrs,keyPtrs->value) == 0){
                ptrs = insertListNodeVal(ptrs,keyPtrs->value);
            }
            free(keyPtrs);
            keyPtrs = nxtKeyPtr;
        }
        
        free(curKey);
        curKey = nxtKey;
    }
    

    // delete records.
    deleteRecords(tree,ptrs,startKey,endKey);
    
    free(updateInfo);
    free(deleteInfo);
    endT = clock();
}

/**
 * A function to get the datablocks containing data records. 
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
        OverflowNode *curNode = (OverflowNode*)(uintptr_t)ptr;
        while(curNode != NULL){
            // update IO count for accessing the overflow nodes. 
            ioCount++; 
            // add datablocks into the list.
            for(int i=0; i<OVERFLOW_RECS ;i++){
                // if valid datablock and datablock not in list, add into list.
                if(curNode->dataBlocks[i] != -1){ 
                    if(findListNodeVal(ptrs, curNode->dataBlocks[i])==0){
                        ptrs = insertListNodeVal(ptrs,curNode->dataBlocks[i]);
                    }
                }
            }
            curNode=curNode->next;
        }
    }
    // enter lone datablock into list
    else{
        ptrs = insertListNodeVal(ptrs,ptr);
    }


    return ptrs; 
}


/**
 * A function to get the datablocks containing data records to be deleted and delete the overflow nodes.
 * @param tree the tree containing the records. 
 * @param ptr the pointer to the datablocks found in the B+ tree (either a datablock pointer or an overflow node pointer)
 * @return returns the list of datablocks.
*/
ListNode* getDataBlocksAndDelete(BTree *tree, double ptr){
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
                if(curNode->dataBlocks[i] != -1){ 
                    if(findListNodeVal(ptrs, curNode->dataBlocks[i])==0){
                        ptrs = insertListNodeVal(ptrs,curNode->dataBlocks[i]);
                    }
                }
            }
            deleteOverflowNode(tree->page,curNode,curNode); // delete overflow node.
            curNode=nextNode;
        }
    }
    // enter lone datablock into list
    else{
        ptrs = insertListNodeVal(ptrs,ptr);
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
void deleteRecords(BTree *tree, ListNode *ptrs, double startKey, double endKey){
    // delete records from data block. 
    ListNode *curBlock = ptrs, *nxtBlock;
    struct Block* block = NULL; 
    while(curBlock!=NULL){
        ioCount++;
        nxtBlock = curBlock->next;
        // EDIT: STORAGE SIDE NOW: delete record in datablock(curPtr->ptr contains the datablock id)
        block = (struct Block*)(uintptr_t)(curBlock->value);
        for(int i=0;i<MAX_RECORDS;i++){
            struct Record rec = getRecordFromBlock(block, i);
            if(startKey <= rec.FG_PCT_home & rec.FG_PCT_home <= endKey){
                deleteRecord(block,i);
            }
        }
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
        else return findListNodeVal(list->next, findval);
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

double* retrieveAllPointersForKey(BTPage* page, double key, double root) {
    double* pointers = NULL;  // Initialize the pointer array as NULL
    int pointerCount = 0;     // Initialize the pointer count as 0

    double nodePtr = root;  // Start from the root node

    while (nodePtr != 0) {
        int nodeType, index;
        nodeType = searchPageRecord(page, nodePtr);

        // Non-Leaf node
        if (nodeType == 1) {
            NonLeafNode* nlNode = (NonLeafNode*)(uintptr_t)nodePtr;
            index = searchNonLeafNodeKey(nlNode, key);

            // Find the node with the key and the index pointing
            printf("Node: %f, Key: %f, NodeType: %d\n", nodePtr, key, nodeType);  // Debug print
            printf("Index %d\n", index);
            nodePtr = nlNode->ptrs[index];  // Move to the appropriate child node
        }
            // Leaf node
        else {
            LeafNode* lNode = (LeafNode*)(uintptr_t)nodePtr;
            index = searchLeafNodeKey(lNode, key);

            if (index != -1) {

                printf("Node: %f, Key: %f, NodeType: %d\n", nodePtr, key, nodeType);  // Debug print
                double nextPtr = lNode->ptrs[index];
                int resultType = searchPageRecord(page, nextPtr);

                if (resultType == 2) { // result points to a single leafNode
                    struct Record* recordPtr = (struct Record*)(uintptr_t)nextPtr;
                    pointers[pointerCount++] = nextPtr;

                }
                else {
                    printf("HERE\n");
                    OverflowNode* overflowPtr = (OverflowNode*)(uintptr_t)nextPtr;
                    for (pointerCount = 0; pointerCount < OVERFLOW_RECS; pointerCount++) {
                        if (overflowPtr->dataBlocks[pointerCount] != 0) {
                            pointers[pointerCount] = overflowPtr->dataBlocks[pointerCount];
                        }
                    }

                }

                return pointers;

                //printf("Node: %f, Key: %f, NodeType: %d\n", nodePtr, key, nodeType);  // Debug print
                //
                //double nextPtr = (())
                //
                //double recordPtr = ((LeafNode*)(uintptr_t)nodePtr)->ptrs[index];
                //float FG3_PCT_home = ((struct Record*)(uintptr_t)recordPtr)->FG3_PCT_home;
                //printf("Node FG_PCT_3 Value: %f\n", FG3_PCT_home);


                // Found the key in the leaf node or its overflow nodes
                //if (lNode->keys[index] == key) {
                //    // Allocate memory for the pointers
                //    int maxPointers = P_REC_COUNT;
                //    pointers = (double*)malloc(maxPointers * sizeof(double));

                //    /* THROWS ERROR
                //    //// Get pointers from the current leaf node
                //    //while (index < maxPointers && lNode->keys[index] == key) {
                //    //    pointers[pointerCount++] = lNode->ptrs[index];
                //    //    index++;
                //    //}

                //    //// Move to the next leaf node (if any)
                //    //lNode = lNode->next;
                //    //while (lNode != NULL && lNode->keys[0] == key) {
                //    //    for (index = 0; index < maxPointers && lNode->keys[index] == key; index++) {
                //    //        pointers[pointerCount++] = lNode->ptrs[index];
                //    //    }
                //    //    lNode = lNode->next;
                //    //}
                //    */
                //    //while (index < maxPointers && lNode->keys[index] == key) {
                //    //    pointers[pointerCount++] = lNode->ptrs[index];
                //    //    index++;
                //    //}

                //    //while (lNode != NULL) {
                //    //    lNode = lNode->next;
                //    //    if (lNode != NULL && lNode->keys[0] != key) {
                //    //        break;  // Exit if the next leaf node has a different key
                //    //    }

                //    //    for (index = 0; index < maxPointers && lNode->keys[index] == key; index++) {
                //    //        pointers[pointerCount++] = lNode->ptrs[index];
                //    //    }
                //    //}


                //    // Return collected pointers and their count
                //    return pointers;
                //}
            }

            // Key not found in this leaf node, move to the next leaf node
            nodePtr = lNode->ptrs[P_REC_COUNT - 1];
        }
    }

    // Return NULL if key not found
    return pointers;
}

