#include "bt_page.h"
#include "bt_node.h"
#include "bt_mgr.h"

#include "bt_page.c"
#include "bt_node.c"
#include "bt_mgr.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// functions 
int main(){
    char field[20] = "test";
    BTree* testTree = createTree(field);
    InsertNode* insertNode = (InsertNode*) malloc(sizeof(InsertNode));
    NonLeafNode* root;
    LeafNode *child;

    int value = OVERFLOW_RECS+6;
    for(int i=1; i<=value;i++){
        printf("i = %d \n",i);
        insertNode->key = 3;
        insertNode->ptr = i; 
        insertBTreeKey(testTree,insertNode);
        if(searchPageRecord(testTree->page,testTree->root) == 2){
            child = (LeafNode*)(uintptr_t)testTree->root;
            for(int j=0;j<N;j++){
                printf("%.0f ",child->ptrs[j]);
                printf("%.0f ",child->keys[j]);
            }
            printf("%.0f \n",child->next);
        }
        else{
            root = (NonLeafNode*) (uintptr_t) testTree->root;
            for(int j=0;j<N;j++){
                printf("%.0f ",root->ptrs[j]);
                printf("%.0f ",root->keys[j]);
            }
            printf("%.0f \n",root->ptrs[N]);
        }
    }
    
    OverflowNode *overflow = (OverflowNode*)(uintptr_t)(child->ptrs[0]);
    for(int j=0;j<OVERFLOW_RECS;j++){
        printf("%.0f ",overflow->dataBlocks[j]);
    }

    overflow = overflow->next;
    for(int j=0;j<OVERFLOW_RECS;j++){
        printf("%.0f ",overflow->dataBlocks[j]);
    }

}