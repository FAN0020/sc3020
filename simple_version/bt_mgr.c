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
    int nodeType = searchPageRecord(tree->page,tree->root);
    if(nodeType == 1){
        NonLeafNode* nlNode = (NonLeafNode*)(uintptr_t)(tree->root);
        if(nlNode->keys[0] == -1){
            deletePageRecord(tree->page,tree->root);
            tree->root = nlNode->ptrs[0];
            deleteNonLeafNode(nlNode);
        }
    }
    else if(nodeType == 2){
        LeafNode* lNode = (LeafNode*)(uintptr_t)(tree->root);
        if(lNode->keys[0] == -1){
            tree->root = -1;
            deleteLeafNode(lNode);
        }
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

/**
 * A function to print out the keys of the root node. 
 * @param tree the B+ Tree
*/
void printRootKeys(BTree *tree){
    int type = searchPageRecord(tree->page, tree->root);
    if(type == 1){
        NonLeafNode* nlNode = (NonLeafNode*)(uintptr_t)(tree->root);
        for(int i=0;i<N;i++){
            if(nlNode->keys[i] == -1) break; 
            printf("%f ",nlNode->keys[i]);
        }
    }
    else if(type == 2){
        LeafNode* lNode = (LeafNode*)(uintptr_t)(tree->root);
        for(int i=0;i<N;i++){
            if(lNode->keys[i] == -1) break; 
            printf("%f ",lNode->keys[i]);
        }
    }
    else{
        printf("Empty tree, there is no root.");
    }
    printf("\n");
}

/**
 * A function to count to total number of nodes in a B+ Tree (non-leaf and leaf)
 * @param tree objecting containing the tree which nodes we are counting.
 * @return returns the total number of nodes in the tree.
*/
int countNode(BTree *tree){
    int count=0, i;
    BTPage* curPage = tree->page;
    // loop through pages and count non-leaf nodes and leaf nodes.
    while(curPage!=NULL){
        for(i=0; i<P_REC_COUNT;i++){
            // if non-leaf/leaf, add counter.
            if(curPage->types[i] == 1 | curPage->types[i] == 2){
                count++; 
            }
        }
        curPage = curPage->next;
    }
    return count;
}

/**
 * A function to count the number of node levels in a B+ Tree. 
 * @param page object containing all the nodes and their types, used to identify node. 
 * @param node pointer of the current node. 
 * @return returns the number of levels in a tree.
*/
int countLevel(BTPage *page, double node){
    int nodeType = searchPageRecord(page, node);
    // leaf node found, count leaf node and end recursive.
    if(nodeType == 2){
        return 1; 
    }
    // non-leaf node found, go deeper into tree and count non-leaf node.
    else if(nodeType == 1){
        NonLeafNode* nlNode = (NonLeafNode*)(uintptr_t)(node);
        return 1 + countLevel(page, nlNode->ptrs[0]);
    }
    // no node found.
    else{
        return 0;
    }

}

