#include "bt_page.h"
#include "bt_node.h"
#include "bt_mgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * function to create a B+ Tree
 * @param field the field the records are indexed by. 
 * @return returns the B+ Tree created. 
*/
BTree* createTree(char field[20]){
    BTree *newTree = (BTree*)malloc(sizeof(BTree));
    strcpy(newTree->field,field);
    newTree->root = -1; // signifies no node created in tree yet.
    return newTree;
}


void insertBTreeKey(BTPage* page,BTree* tree,Node* insertNode){
    int toUpdate = insertKey(page,tree->root,insertNode);
    if(toUpdate==1){
        // empty tree, make node the root. 
        if(tree->root == -1){
            tree->root = insertNode->ptr;
        }
        // adding new root node.
        else{
            NonLeafNode* newRoot = createNonleafNode(tree->root,insertNode->key,insertNode->ptr);
            insertPageRecord(page,(double)(uintptr_t)newRoot,1);
            tree->root = (double)(uintptr_t)newRoot;
        }
    }
}