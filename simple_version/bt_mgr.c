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
    newTree->page = createPage();
    return newTree;
}


void insertBTreeKey(BTree* tree,InsertNode* insertNode){
    int toUpdate = insertKey(tree->page,tree->root,insertNode);
    if(toUpdate==1){
        // empty tree, make node the root. 
        if(tree->root == -1){
            tree->root = insertNode->ptr;
        }
        // adding new root node.
        else{
            NonLeafNode* newRoot = createNonleafNode(tree->root,insertNode->key,insertNode->ptr);
            insertPageRecord(tree->page,(double)(uintptr_t)newRoot,1);
            tree->root = (double)(uintptr_t)newRoot;
        }
    }
}

void deleteBTreeKey(BTree* tree, UpdateNode* updateNode, DeleteNode* deleteInfo){
    // delete!
    int toUpdate = deleteKey(tree->page, tree->root,tree->root, deleteInfo, updateNode); 
    NonLeafNode* root = (NonLeafNode*)(uintptr_t)(tree->root);
    if(root->keys[0] == -1){
        tree->root = root->ptrs[0];
    }
}

/**
 * A function to delete the B+ Tree
 * @param tree the tree to be deleted.
*/
void deleteBTree(BTree* tree){
    // delete page.
    BTPage *newPage, *page = tree->page;
    int nodeType;
    while(page != NULL){
        newPage = page->next;
        for(int i =0;i<P_REC_COUNT;i++){
            nodeType = page->types[i];
            if(nodeType == 1){
                deleteNonLeafNode((NonLeafNode*)(uintptr_t)(page->pointers[i]));
            }
            else if(nodeType == 2){
                deleteLeafNode((LeafNode*)(uintptr_t)(page->pointers[i]));
            }
            else if(nodeType == 3){
                free((OverflowNode*)(uintptr_t)(page->pointers[i]));
            }
        }
        deletePage(page);
        page = newPage;
    }
    // delete tree.
    free(tree);
}

/**
 * A function to search key and return datablock/overflow node containing location of records.
 * @param tree object containing details of the tree
 * @param key key we're searching for in the tree.
 * @return returns the datablock/overflow node containing the records. (returns -1 if key is not found.)
*/
double searchBTreeKey(BTree *tree, float key){
    return searchKey(tree->page,tree->root,key);
}

/**
 * A function to search key and return leaf node containing start of the range.
 * @param tree object containing details of the tree
 * @param key starting range key we're searching for in the tree.
 * @return returns the leaf node containing start of the range.
*/
double searchBTreeRangeKey(BTree *tree, float key){
    return searchRangeKey(tree->page,tree->root,key);
}
