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
    memset(node->keys,-1,sizeof(node->keys));
    memset(node->ptrs,-1,sizeof(node->ptrs));
    node->keys[0] = key2;
    node->ptrs[0] = ptr1; 
    node->ptrs[1] = ptr2; 
    return node; 
}

/**
 * function to search for key of the non-leaf node
 * @param node non-leaf node
 * @param key search key
 * @return returns the "index position" of the key or -1 if key not found
*/
int searchNonLeafNodeKey(NonLeafNode* node, float key){
    for(int i=0;i<N;i++){
        if(node->keys[i] == key){
            return i; 
        }
    }
    return -1; // key not found
}

/**
 * function to search for key of the non-leaf node
 * @param node non-leaf node
 * @param key search key
 * @return returns the child node pointer.
*/
double searchNonLeafNode(NonLeafNode* node, float key){
    int i; 
    for(i=0;i<N;i++){
        if(key < node->keys[i]){
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
int insertNonLeafNodeKey(NonLeafNode* node,Node* insertNode){
    int i, lowerboundKey=insertNode->key, lowerboundptr=insertNode->ptr, midKey=N/2;
    NonLeafNode* newNode; 
    // Node is full
    if(node->keys[N-1] != -1){
        // new key belongs to 2nd half of split. 
        if(node->keys[midKey-1] > insertNode->key){
            if(node->keys[midKey] > insertNode->key){
                newNode = createNonleafNode(insertNode->ptr,node->keys[midKey],node->ptrs[midKey+1]);
            }
            else{
                lowerboundKey = node->keys[midKey];
                lowerboundptr = node->ptrs[midKey+1];
                newNode = createNonleafNode(node->ptrs[midKey+1],insertNode->key,insertNode->ptr);
            }
            deleteLeafNodeKey(node,node->keys[N/2]);
        }
        // new key belongs to first half of split.
        else{
            lowerboundKey = node->keys[midKey];
            lowerboundptr = node->ptrs[midKey+1];
            createNonleafNode(node->ptrs[midKey],node->keys[midKey],node->ptrs[midKey+1]);
            deleteLeafNodeKey(node, node->keys[midKey-1]);
            deleteLeafNodeKey(node, node->keys[midKey-1]);
            // now got space for new node, add inside. 
            insertNLNodeKey(node,insertNode->key, insertNode->ptr);
        }
        // shift remaining half to 2nd half
        while(node->keys[midKey]!=-1){
            insertNLNodeKey(newNode,node->keys[midKey], node->ptrs[midKey+1]);
            deleteNonLeafNodeKey(node,node->keys[midKey-1]);
        }
        insertNode->key = lowerboundKey;
        insertNode->ptr = lowerboundptr;
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
                break; 
            }
        }
    }
}

/**
 * function to delete a key/child node from a non-leaf node
 * @param node parent node that a child node is to be deleted from. 
 * @param delKey pointer to the node to be deleted.
 * @return returns binary 0) deleting key has no error 1) invalid node (not enough keys)
*/
int deleteNonLeafNodeKey(NonLeafNode* node, float delKey){
    int i = searchNonLeafNodeKey(node,delKey);
    for(i; i<N; i++){
        node->keys[i] = node->keys[i+1];
        node->ptrs[i+1] = node->ptrs[i+2];
        if(node->keys[i] == -1){
            break; 
        }
    }
    if(i < N/2) return 1; 
    return 0;
}


/**
 * function to create leaf Node
 * @return returns the leafnode created
*/
LeafNode* createLeafNode(float key, double ptr){
    LeafNode* node = (LeafNode*)malloc(sizeof(LeafNode));
    memset(node->keys,-1,sizeof(node->keys));
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
int insertLeafNodeKey(LeafNode* node,Node* insertNode){
    int i,  midKey=(N+1)/2;
    LeafNode* newNode; 
    // Node is full
    if(node->keys[N-1] != -1){
        newNode = createLeafNode(node->keys[midKey],node->ptrs[midKey]);
        while(node->keys[midKey]!=-1){
            insertLNodeKey(newNode,node->keys[midKey],node->ptrs[midKey]);
            deleteLeafNodeKey(node,node->keys[midKey]);
        }
        // new key belongs to 2nd half of split. 
        if(node->keys[midKey-1] > insertNode->key){
            insertLNodeKey(newNode,insertNode->key,insertNode->ptr);
        }
        // new key belongs to first half of split.
        else{
            insertLNodeKey(newNode,node->keys[midKey-1],node->ptrs[midKey-1]);
            deleteLeafNodeKey(node,node->keys[midKey-1]);
            insertLNodeKey(node,insertNode->key,insertNode->ptr);
        }
        insertNode->key = newNode->keys[0];
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
                break; 
            }
        }
    }
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
 * function to search for key of the non-leaf node
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
 * function to create non-leaf node
 * @return returns the nonleaf node created
*/
OverflowNode* createOverflowNode(double dataPtr){
    OverflowNode* node = (OverflowNode*)malloc(sizeof(OverflowNode));
    memset(node->dataBlocks,-1,sizeof(node->dataBlocks));
    node->dataBlocks[0] = dataPtr;
    node->next = NULL;
    return node; 
}

/**
 * function to insert record
*/
int insertOverflowRecord(OverflowNode* node, double dataPtr){
    for(int i=0;i<OVERFLOW_RECS;i++){
        if(node->dataBlocks[i] == dataPtr){
            //data block already indexed, stop. 
        }
        if(node->dataBlocks[i] == -1){
            // empty slot found insert datablock pointer.
            node->dataBlocks[i] = dataPtr;
            return;
        }
    }
    // node has no space, look add into next node; 
    insertOverflowRecord(node->next, dataPtr);

}


