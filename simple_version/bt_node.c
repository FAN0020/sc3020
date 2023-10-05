#include "bt_page.h"
#include "bt_node.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * function to create non-leaf node 
 * @param ptr1 pointer to the first child. 
 * @param key2 key representing the second child.
 * @param ptr2 pointer to the second child
 * @return returns the nonleaf node created
*/
NonLeafNode* createNonleafNode(double ptr1, float key2, double ptr2){
    NonLeafNode* node = (NonLeafNode*)malloc(sizeof(NonLeafNode));
    int i; 
    for(i=N;i>1;i--){
        node->keys[i-1]=-1;
        node->ptrs[i]=-1;
    }
    node->keys[0] = key2;
    node->ptrs[0] = ptr1; 
    node->ptrs[1] = ptr2; 
    return node; 
}

/**
 * function to search for key of the non-leaf node
 * @param node non-leaf node
 * @param key search key
 * @return returns the "index position" of the pointer of key
*/
int searchNonLeafNodeKey(NonLeafNode* node, float key){
    int i; 
    for(i=0;i<N;i++){
        if(key < node->keys[i] | node->keys[i] == -1){
            break;
        }
    }
    return i; //return index
}

/**
 * function to search for pointer of the key of the non-leaf node
 * @param node non-leaf node
 * @param key search key
 * @return returns the child node pointer.
*/
double searchNonLeafNode(NonLeafNode* node, float key){
    int i; 
    for(i=0;i<=N;i++){
        if(key < node->keys[i] | node->keys[i] == -1){
            break;
        }
    }
    return node->ptrs[i]; //return pointer
}

/**
 * function to insert a child node into a non-leaf node
 * @param node parent node that non-leaf node child node is to be added to. 
 * @param insertNode object containing the lowerbound and pointer of child node. 
 * @return returns binary, 1 if further updates to tree is needed.
*/
int insertNonLeafNodeKey(NonLeafNode* node,InsertNode* insertNode){
    int i, midKey=N/2;
    float lowerboundKey=insertNode->key;
    double lowerboundptr=insertNode->ptr;
    printf("lowerbound key = %.0f \n",lowerboundKey);
    NonLeafNode* newNode; 
    // Node is full
    if(node->keys[N-1] != -1){
        // new key belongs to 2nd half of split. 
        if(node->keys[midKey-1] < insertNode->key){
            if(node->keys[midKey] > insertNode->key){
                newNode = createNonleafNode(insertNode->ptr,node->keys[midKey],node->ptrs[midKey+1]);
            }
            else{
                lowerboundKey = node->keys[midKey];
                newNode = createNonleafNode(node->ptrs[midKey+1],insertNode->key,insertNode->ptr);
            }
            deleteNLNodeKey(node,node->keys[midKey]);
        }
        // new key belongs to first half of split.
        else{
            lowerboundKey = node->keys[midKey-1];
            printf("new lowerbound key = %.0f \n",lowerboundKey);
            newNode = createNonleafNode(node->ptrs[midKey],node->keys[midKey],node->ptrs[midKey+1]);
            deleteNLNodeKey(node, node->keys[midKey-1]);
            deleteNLNodeKey(node, node->keys[midKey-1]);
            // now got space for new node, add inside. 
            insertNLNodeKey(node,insertNode->key, insertNode->ptr);
        }
        // shift remaining half to 2nd half
        while(node->keys[midKey]!=-1){
            insertNLNodeKey(newNode,node->keys[midKey], node->ptrs[midKey+1]);
            deleteNLNodeKey(node,node->keys[midKey]);
        }
        insertNode->key = lowerboundKey;
        insertNode->ptr = (double)(uintptr_t)newNode;
        return 1; 
    }
    // node has space
    else{
        // add key into node. 
        insertNLNodeKey(node,insertNode->key, insertNode->ptr);
        return 0;
    }
    
}

/**
 * function to insert non-leaf node (used only for nodes with space)
 * @param node parent node to insert child node into. 
 * @param key loweer bound key of child node. 
 * @param ptr pointer of the child node. 
*/
void insertNLNodeKey(NonLeafNode* node,float key, double ptr){
    for(int i=N-1; i>0; i--){
        if(node->keys[i-1] != -1){
            if(node->keys[i-1]>key){
                node->keys[i] = node->keys[i-1];
                node->ptrs[i+1] = node->ptrs[i];
            }
            else if (node->keys[i-1] < key){
                node->keys[i] = key;
                node->ptrs[i+1] = ptr;
                return; 
            }
        }
    }
    node->keys[0] = key;
    node->ptrs[1] = ptr;
    for(int j=0;j<N;j++){
        printf("%.0f ",node->ptrs[j]);
        printf("%.0f ",node->keys[j]);
    }
    printf("%.0f \n",node->ptrs[N]);
}

/**
 * function to delete a key/child node from a non-full non-leaf node
 * @param node parent node that a child node is to be deleted from. 
 * @param delKey pointer to the node to be deleted.
 * @return returns binary 0) deleting key has no error 1) invalid node (not enough keys)
*/
int deleteNLNodeKey(NonLeafNode* node, float delKey){
    int i = searchNonLeafNodeKey(node,delKey);
    for(i; i<N; i++){
        node->keys[i-1] = node->keys[i];
        node->ptrs[i] = node->ptrs[i+1];
        if(node->keys[i-1] == -1){
            break; 
        }
    }
    node->keys[N-1] = -1; // in case leaf was previously full, remove last key
    node->ptrs[N] = -1;

    
    for(int j=0;j<N;j++){
        printf("%.0f ",node->ptrs[j]);
        printf("%.0f ",node->keys[j]);
    }
    printf("%.0f \n",node->ptrs[N]);

    if(i < N/2) return 1; 
    return 0;
}

/**
 * function to delete child node of a non-leaf node.
 * @param node current parent node. 
 * @param deleteNode information of nodes to be deleted/updated.
*/
int deleteNonLeafNodeKey(NonLeafNode* node, DeleteNode* deleteNode){
    // delete key first.
    deleteNode->key = deleteNode->oldKey;
    int isInvalid = deleteNLNodeKey(node,deleteNode->key);
    {

    }

}

/** 
 * A function to find the lowerbound key of the nodes. 
 * @param page contains list of nodes and their types. 
 * @param node current node
 * @return returns the lowerbound key of the node.
*/
float retrieveNonLeafLowerboundKey(BTPage* page, double node){
    int nodeType = searchPageRecord(page, node);
    // non-leaf, move deeper down the tree.
    if(nodeType == 1){
        NonLeafNode* nlNode = (NonLeafNode*)(uintptr_t)node; 
        return retrieveNonLeafLowerboundKey(page, nlNode->ptrs[0]);
    }
    // leaf, returns the first key (lowerbound) 
    else if(nodeType==2){
        LeafNode* lNode = (LeafNode*)(uintptr_t)node; 
        return lNode->keys[0];
    }
    return -1; //nothing found.
}


/**
 * function to create leaf Node
 * @return returns the leafnode created
*/
LeafNode* createLeafNode(float key, double ptr){
    LeafNode* node = (LeafNode*)malloc(sizeof(LeafNode));
    int i; 
    for(i=N-1;i>0;i--){
        node->keys[i]=-1;
        node->ptrs[i]=-1;
    }
    node->keys[0]=key;
    node->ptrs[0]=ptr;
    node->next = NULL;
    return node; 
}

/**
 * function to insert child node into a leaf node.
 * @param node the parent leaf node. 
 * @param insertNode the child node to be inserted. 
 * @return returns binary, 1 if further updates to tree is needed.
*/
int insertLeafNodeKey(LeafNode* node,InsertNode* insertNode){
    int i,  midKey=(N+1)/2;
    LeafNode* newNode; 
    // Node is full
    if(node->keys[N-1] != -1){
        newNode = createLeafNode(node->keys[midKey],node->ptrs[midKey]);
        printf("%.0f \n",node->keys[midKey]);
        while(node->keys[midKey]!=-1){
            insertLNodeKey(newNode,node->keys[midKey],node->ptrs[midKey]);
            deleteLNodeKey(node,node->keys[midKey]);
        }
        
        // new key belongs to 2nd half of split. 
        if(node->keys[midKey-1] < insertNode->key){
            insertLNodeKey(newNode,insertNode->key,insertNode->ptr);
        }
        // new key belongs to first half of split.
        else{
            insertLNodeKey(newNode,node->keys[midKey-1],node->ptrs[midKey-1]);
            deleteLNodeKey(node,node->keys[midKey-1]);
            insertLNodeKey(node,insertNode->key,insertNode->ptr);
        }
        insertNode->key = newNode->keys[0];
        printf("split node key %0.f \n",insertNode->key);
        insertNode->ptr = (double)(uintptr_t)newNode;
        newNode->next = node->next; 
        node->next = newNode;
        return 1; 
    }
    // node has space
    else{
        // add key into node. 
        insertLNodeKey(node,insertNode->key, insertNode->ptr);
        return 0;
    }
    
}


/**
 * function to insert leaf node (used only for nodes with space)
 * @param node parent node to insert child node into. 
 * @param key lower bound key of child node. 
 * @param ptr pointer of the child node. 
*/
void insertLNodeKey(LeafNode* node,float key, double ptr){
    for(int i=N-1; i>0; i--){
        if(node->keys[i-1] != -1){
            if(node->keys[i-1]>key){
                node->keys[i] = node->keys[i-1];
                node->ptrs[i] = node->ptrs[i-1];
            }
            else if (node->keys[i-1] < key){
                node->keys[i] = key;
                node->ptrs[i] = ptr;
                return; 
            }
        }
    }
    node->keys[0] = key;
    node->ptrs[0] = ptr;
}


/**
 * function to search for key of the leaf node
 * @param node non-leaf node
 * @param key search key
 * @return returns the "index position" of the key or -1 if key not found
*/
int searchLeafNodeKey(LeafNode* node, float key){
    for(int i=0;i<N;i++){
        if(node->keys[i] == key){
            return i; 
        }
    }
    return -1; // key not found
}

/**
 * function to search for the pointer of the key of the non-leaf node
 * @param node non-leaf node
 * @param key search key
 * @return returns the child node pointer.
*/
double searchLeafNode(LeafNode* node, float key){
    int i; 
    for(i=0;i<N;i++){
        if(key == node->keys[i]){
            return node->ptrs[i];
        }
    }
    return -1; //key not found no pointer. 
}

/**
 * function to delete a key/child node from a non-full leaf node
 * @param node parent node that a child node is to be deleted from. 
 * @param delKey pointer to the node to be deleted.
 * @return returns binary 0) deleting key has no error 1) invalid node (not enough keys)
*/
int deleteLNodeKey(LeafNode* node, float delKey){
    int i = searchLeafNodeKey(node,delKey);
    for(i; i<N-1; i++){
        node->keys[i] = node->keys[i+1];
        node->ptrs[i] = node->ptrs[i+1];
        if(node->keys[i] == -1){
            break; 
        }
    }
    node->keys[N-1] = -1; // in case leaf was previously full, remove last key
    node->ptrs[N-1] = -1; 
    // for(int j=0;j<N;j++){
    //     printf("%.0f ",node->ptrs[j]);
    //     printf("%.0f ",node->keys[j]);
    // }
    // printf("%.0f \n",node->next);

    if(i < (N+1)/2) return 1; 
    return 0;
}

/**
 * function to delete child node of a leaf node.
 * @param node current parent node. 
 * @param deleteNode information of nodes to be deleted/updated.
 * @returns returns binary 1 if updates are to be made to parent nodes. 
*/
int deleteLeafNodeKey(LeafNode* node,double lNode,double rNode, DeleteNode* deleteNode){
    // delete key first.
    deleteNode->oldKey = node->keys[0];
    int isInvalid = deleteLNodeKey(node,deleteNode->key);
    if(isInvalid){
        LeafNode* leftNode, *rightNode, *mergeNode; 
        int toNext = 1; // binary to indicate to move to next case.
        // not enough keys, requires merging. 
        int i, midKey = (N+1)/2;
        // Check left first. 
        if (lNode!=-1){
            leftNode = (LeafNode*)(uintptr_t)lNode; 
            for(i=N-1;i>=0;i--){
                if(leftNode->keys[i] != -1)break;
            }
            if(i>=midKey){
                // take from left node. 
                insertLNodeKey(node,leftNode->keys[i],leftNode->ptrs[i]);
                deleteLNodeKey(leftNode,leftNode->keys[i]);
                deleteNode->newKey = node->keys[0];
                toNext=0; 
            }

        }
        if(toNext & rNode != -1){
            rightNode = (LeafNode*)(uintptr_t)lNode; 
            // check right node. 
            for(i=N-1;i>=0;i--){
                if(node->next->keys[i] != -1)break;
            }
            if(i>=midKey){
                // take from right node. 
                deleteNode->oldKey = node->next->keys[0];
                insertLNodeKey(node,node->next->keys[i],node->next->ptrs[i]);
                deleteLNodeKey(node->next,node->next->keys[i]);
                deleteNode->newKey = node->next->keys[0];
                toNext = 0; 
            }
        }
        if(toNext){
            deleteNode->key = deleteNode->oldKey;
            if(lNode != -1){
                mergeNode = leftNode;
                deleteNode->oldKey = -1;
                deleteNode->newKey = -1;
            }
            else {
                mergeNode = rightNode;
                deleteNode->oldKey = rightNode->keys[0];
                deleteNode->newKey = node->keys[0];
            }
            // merge node. 
            while(node->keys[0] != 1){
                insertLNodeKey(mergeNode,node->keys[0],node->ptrs[0]);
                deleteLNodeKey(node,node->keys[0]);
            }
            deleteNode->ptr = deleteLeafNode(node);
        }
        return 1; 
    } 
    else{
        // update key if necessary.
        // lowerbound of node has been changed.
        if(deleteNode->oldKey != node->keys[0]){
            deleteNode->newKey = node->keys[0];
            return 1;
        }
        // no updates needed.
        return 0;
    }
    
}

/**
 * Function to delete leaf node.
 * @param node leaf node to be deleted.
 * @return pointer of the deleted node. 
*/
double deleteLeafNode(LeafNode* node){
    double ptr = (double)(uintptr_t)node; 
    free(node); 
    return ptr; 
}

/**
 * function to create non-leaf node
 * @return returns the nonleaf node created
*/
OverflowNode* createOverflowNode(double dataPtr){
    OverflowNode* node = (OverflowNode*)malloc(sizeof(OverflowNode));
    int i; 
    for(i=OVERFLOW_RECS-1;i>0;i--){
        node->dataBlocks[i]=-1;
    }
    node->dataBlocks[0] = dataPtr;
    node->next = NULL;
    return node; 
}

/**
 * function to insert child into overflow node
 * @param node the overflow node.
 * @param insertNode the datablock info to be added.
 * @return returns binary 1 if there are things to be updated.
*/
int insertOverflowRecord(OverflowNode* node, InsertNode* insertNode){
    for(int i=0;i<OVERFLOW_RECS;i++){
        if(node->dataBlocks[i] == insertNode->ptr){
            //data block already indexed, stop. 
            return 0; 
        }
        if(node->dataBlocks[i] == -1){
            // empty slot found insert datablock pointer.
            node->dataBlocks[i] = insertNode->ptr;
            return 0;
        }
    }
    // node has no space, look add into next node;
    // no more node, create node to insert.
    if(node->next == NULL){
        OverflowNode *newNode = createOverflowNode(insertNode->ptr);
        node->next = newNode;
        insertNode->ptr = (double)(uintptr_t)newNode; 
        return 1; 
    }
    // search in other node.
    else{
        return insertOverflowRecord(node->next, insertNode);
    }

}

/**
 * A function to get the last overflow node.
 * @param node current node being traversed.
 * @param return the last node.
*/
OverflowNode* lastOverflowNode(OverflowNode *node){
    if(node->next == NULL){
        return node;
    }
    return lastOverflowNode(node->next);
}

/**
 * A function to check the records in an overflow node and returns pointer if changes to tree are needed.
 * @param node current overflow node.
 * @return returns the pointer of the record if 1 record left, pointer of empty node, otherwise -1.
*/
double CheckOverflowNode(OverflowNode* node){
    int count=0; 
    double pointer; 
    for(int i =0; i<OVERFLOW_RECS;i++){
        // data record found, not empty
        if(node->dataBlocks[i] != -1){
            count++; 
            pointer = node->dataBlocks[i];
        } 
    }
    if(count == 0){
        return (double)(uintptr_t)node;
    }
    else if(count == 1){
        return pointer;  
    }
    return -1;
}


/**
 * A function to delete overflow record. 
 * @param node overflow node where the record is stored.
 * @param deleteNode information of the record to be deleted.
 * @return returns binary, 1 if any overflow node is deleted.
*/
int deleteOverflowRecord(OverflowNode* node, DeleteNode* deleteNode){
    for(int i = 0; i < OVERFLOW_RECS;i++){
        // delete record. 
        if(node->dataBlocks[i] == deleteNode->ptr){
            node->dataBlocks[i] == -1;
            //move last node of overflow forward if node not last node.
            OverflowNode* lastNode = lastOverflowNode(node);
            if(lastNode != node){
                for(int j = OVERFLOW_RECS-1;j>=0;j--){
                    if(lastNode->dataBlocks[j] != -1){
                        node->dataBlocks[i] = lastNode->dataBlocks[j];
                        lastNode->dataBlocks[j] = -1;
                    }
                }
            }
            // delete empty overflow node when necessary. 
            double pointer = CheckOverflowNode(lastNode);
            deleteNode->ptr = pointer;
            if(pointer == -1){
                return 0;
            }
            else{
                free(lastNode);
                return 1;
            }
        }
    }
    //record not found, move to next node. 
    if(node->next == NULL){
        return 0; 
    }
    else{
        return deleteOverflowRecord(node->next, deleteNode);
    }
    
}

/**
 * main function to insert a new datarecord into the B+ Tree
 * @param page page that records the different types of nodes, used to differentiate the node type
 * @param nodePtr pointer of the current node. 
 * @param key key of the datarecord being added. 
 * @returns returns binary, 1 if parent node needs to be updated 
*/
int insertKey(BTPage *page, double nodePtr,InsertNode* insertNode){
    // identify type of node, 1) non-leaf, 2) leaf, 3) overflow. 
    int index, toUpdate = 0;
    double childPtr; 
    int nodeType = searchPageRecord(page, nodePtr);
    // non-leaf node
    if(nodeType == 1){
        printf("%s %.0f %.0f \n","NonLeaf",insertNode->key, insertNode->ptr);
        NonLeafNode *nlNode = (NonLeafNode*)(uintptr_t)nodePtr; 
        // go deeper into the tree
        childPtr = searchNonLeafNode(nlNode,insertNode->key); 
        toUpdate = insertKey(page, childPtr, insertNode);
        printf("non-leaf inserting key = %.0f \n",insertNode->key);
        // update the current node.
        if(toUpdate){
            toUpdate = insertNonLeafNodeKey(nlNode,insertNode); 
            // requires parent node to be updated as new node was created.
            if(toUpdate){
                // record new node
                insertPageRecord(page,insertNode->ptr,1);
            }
        }
    }
    else if(nodeType ==2){
        printf("%s %.0f %.0f \n","Leaf",insertNode->key, insertNode->ptr);;
        LeafNode *lNode = (LeafNode*)(uintptr_t)nodePtr; 
        // go deeper into the tree
        childPtr = searchLeafNode(lNode,insertNode->key); 
        index = searchLeafNodeKey(lNode,insertNode->key);
        // update the current node.
        if(index == -1){
            printf("%s \n","inserting node...");
            toUpdate = insertLeafNodeKey(lNode,insertNode); 
            // requires parent node to be updated as new node was created.
            if(toUpdate){
                // record new node
                insertPageRecord(page,insertNode->ptr,2);
            }
        }
        else{
            // update pointer to overflow pointer. 
            if(insertKey(page, childPtr, insertNode)){
                lNode->ptrs[index] = insertNode->ptr;
            }
        }
    }
    else if(nodeType ==3){
        printf("%s %.0f %.0f \n","Overflow",insertNode->key, insertNode->ptr);
        OverflowNode *oNode = (OverflowNode*)(uintptr_t)nodePtr; 
        toUpdate = insertOverflowRecord(oNode,insertNode);
        // new node created
        if(toUpdate){
            insertPageRecord(page, insertNode->ptr,3);
            toUpdate=0;
        }
    }
    else{
        printf("%s %.0f %.0f \n","empty",insertNode->key,insertNode->ptr);
        // if empty pointer
        if(nodePtr==-1){
            // create new leaf node with record.
            LeafNode *lNode = createLeafNode(insertNode->key,insertNode->ptr);
            insertPageRecord(page, (double)(uintptr_t)lNode,2);
            insertNode->key = lNode->keys[0];
            insertNode->ptr = (double)(uintptr_t)lNode; 
            toUpdate = 1; 
        }
        else if(nodePtr != insertNode->ptr){
            // data record occupying space. 
            // new datablock of same index, create overflow node.
            OverflowNode *oNode = createOverflowNode(nodePtr);
            insertOverflowRecord(oNode,insertNode);
            insertPageRecord(page, (double)(uintptr_t)oNode,3);
            insertNode->ptr = (double)(uintptr_t)oNode;
            toUpdate=1; 
        }
    }
    return toUpdate; 
}

int deleteKey(BTPage *page,double root, double nodePtr,DeleteNode* deleteNode, UpdateNode* nodeInfo){
    // identify type of node, 1) non-leaf, 2) leaf, 3) overflow. 
    int index, toUpdate = 0;
    double childPtr; 
    int nodeType = searchPageRecord(page, nodePtr);
    // non-leaf node
    if(nodeType == 1){
        printf("%s %.0f %.0f \n","NonLeaf",deleteNode->key, deleteNode->ptr);
        NonLeafNode *nlNode = (NonLeafNode*)(uintptr_t)nodePtr; 
        index = searchNonLeafNodeKey(nlNode,deleteNode->key);
        toUpdate = deleteKey(page,root,nlNode->ptrs[index],deleteNode,nodeInfo);
        if(toUpdate){
            // update key
            //int
            // deleteKey
            // getNodes(page, root, deleteNode->key,nodeInfo);
            // int toUpdate = deleteNonLeafNodeKey(); // EDITTTTTTTTTTTTTTT
            if(toUpdate){
                deletePageRecord(page, deleteNode->ptr);
            }
        }

        
    }
    else if(nodeType ==2){
        printf("%s %.0f %.0f \n","Leaf",deleteNode->key, deleteNode->ptr);
        LeafNode *lNode = (LeafNode*)(uintptr_t)nodePtr; 
        // go deeper into the tree 
        index = searchLeafNodeKey(lNode,deleteNode->key);
        toUpdate = deleteKey(page,root,lNode->ptrs[index],deleteNode,nodeInfo);
        if(toUpdate){
            // datablock removed, no pointers, delete key. 
            if(deleteNode->ptr == -1){
                nodeInfo->cNode = (double)(uintptr_t)lNode; 
                // get the left and right node.
                getNodes(page, root,deleteNode->key,nodeInfo);
                toUpdate = deleteLeafNodeKey(lNode, nodeInfo->lNode,nodeInfo->rNode,deleteNode);
                if(toUpdate){
                    // update the lowerbound of keys if necessary.
                    if(deleteNode->oldKey != deleteNode->newKey){
                        UpdateLowerboundKeys(page, root,deleteNode->oldKey, deleteNode->newKey);
                    }
                    // delete page record.
                    deletePageRecord(page, deleteNode->ptr);
                }
            }
            // update from overflow node to single data block.
            else{
                lNode->ptrs[index] = deleteNode->ptr;
                toUpdate = 0; 
            }
        }
        
    }
    else if(nodeType ==3){
        printf("%s %.0f %.0f \n","Overflow",deleteNode->key, deleteNode->ptr);
        OverflowNode *oNode = (OverflowNode*)(uintptr_t)nodePtr; 
        toUpdate = deleteOverflowRecord(oNode,deleteNode);
        // new node created
        if(toUpdate){
            // delete empty record.
            deletePageRecord(page, deleteNode->ptr);
            if(deleteNode->ptr != (double)(uintptr_t)oNode){
                toUpdate = 1;
            }
            else{
                toUpdate = 0;
            }
        }
    }
    else{
        // datablock pointer. 
        deleteNode->ptr = -1; 
        toUpdate = 1;
    }
    return toUpdate; 
}

/**
 * A function to update the current node, the left node and the right node. 
 * @param page object storing all the nodes and their types. 
 * @param node current node we are searching through. 
 * @param key key we're searching for.
 * @param nodeInfo object containing the node we're searching for and it's neighbouring nodes.
*/
int getNodes(BTPage* page, double node,float key,UpdateNode* nodeInfo){
    int depth = 0;
    double currNode;
    NonLeafNode* nlNode = (NonLeafNode*)(uintptr_t)node; 
    int index = searchNonLeafNodeKey(nlNode,key);
    if(nlNode->ptrs[index] == nodeInfo->cNode){
        if(index>0) nodeInfo->lNode = nlNode->ptrs[index-1];
        if(index<N) nodeInfo->rNode = nlNode->ptrs[index+1];
        return 1; 
    }
    else{
        depth = getNodes(page, nlNode->ptrs[index],key,nodeInfo);
        if(nodeInfo->lNode == -1){
            if(index>0){
                nlNode=(NonLeafNode*)(uintptr_t)(nlNode->ptrs[index-1]);
                currNode = nlNode->ptrs[N];
                for(int i = 1; i<depth;i++ ){
                    nlNode=(NonLeafNode*)(uintptr_t)currNode;
                    currNode = nlNode->ptrs[N];
                }
            }
            nodeInfo->lNode = currNode;
        }
        if(nodeInfo->rNode == -1){
            if(nodeInfo->rNode == -1){
            if(index<N){
                nlNode=(NonLeafNode*)(uintptr_t)(nlNode->ptrs[index+1]);
                currNode = nlNode->ptrs[0];
                for(int i = 1; i<depth;i++ ){
                    nlNode=(NonLeafNode*)(uintptr_t)currNode;
                    currNode = nlNode->ptrs[0];
                }
            }
            nodeInfo->rNode = currNode;
        }
        }
        return depth+1; 
    }
}


/**
 * A function to update the non-leaf nodes when the lowerbounds of leaf nodes have been update. 
 * @param page object containings all the node and the node type.
 * @param node current node
 * @param oldKey the old lowerbound key of the leaf node 
 * @param newKey the new lowerbound key of the leaf node
*/
void UpdateLowerboundKeys(BTPage *page, double node, float oldKey, float newKey){
    NonLeafNode* nlNode = (NonLeafNode*)(uintptr_t)node;
    int index = searchNonLeafNodeKey(nlNode,oldKey); 
    if(index>0 & nlNode->keys[index-1]==oldKey){
        nlNode->keys[index-1]=newKey;
    }
    else{
        UpdateLowerboundKeys(page, nlNode->ptrs[index],oldKey,newKey);
    }
} 





