#include "bt_page.h"
#include "bt_node.h"
#include "bt_mgr.h"
#include "database.h"

#include "bt_page.c"
#include "bt_node.c"
#include "bt_mgr.c"
#include "database.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// functions 
int main(){
    char field[20] = "test";
    BTree* testTree = createTree(field);
    InsertNode* insertNode = (InsertNode*) malloc(sizeof(InsertNode));
    NonLeafNode* root;
    LeafNode *child;
    OverflowNode *overflow;

    int value = OVERFLOW_RECS*12;
    for(int i=1; i<=value;i++){
        printf("i = %d \n",i);
        insertNode->key = i%3;
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
    

    for(int i=0; i<3; i++){
        overflow = (OverflowNode*)(uintptr_t)(child->ptrs[i]);
        for(int j=0;j<OVERFLOW_RECS;j++){
            printf("%.0f ",overflow->dataBlocks[j]);
        }
        for(int k=1; k<4;k++){
            overflow = overflow->next;
            for(int j=0;j<OVERFLOW_RECS;j++){
                printf("%.0f ",overflow->dataBlocks[j]);
            }
        }
        printf("\n");
        printf("\n");
    }


    // OverflowNode *overflow = (OverflowNode*)(uintptr_t)(child->ptrs[0]);
    // for(int j=0;j<OVERFLOW_RECS;j++){
    //     printf("%.0f ",overflow->dataBlocks[j]);
    // }
    // printf("\n");

    // overflow = (OverflowNode*)(uintptr_t)(child->ptrs[1]);
    // for(int j=0;j<OVERFLOW_RECS;j++){
    //     printf("%.0f ",overflow->dataBlocks[j]);
    // }
    // printf("\n");

    // overflow = (OverflowNode*)(uintptr_t)(child->ptrs[2]);
    // for(int j=0;j<OVERFLOW_RECS;j++){
    //     printf("%.0f ",overflow->dataBlocks[j]);
    // }
    // printf("\n");

    printf("The number of nodes in the tree = %d\n", countNode(testTree));

    // overflow = overflow->next;
    // for(int j=0;j<OVERFLOW_RECS;j++){
    //     printf("%.0f ",overflow->dataBlocks[j]);
    // }

    printRootKeys(testTree);

    UpdateNode *update = (UpdateNode*) malloc(sizeof(UpdateNode));
    DeleteNode *delete = (DeleteNode*)malloc(sizeof(DeleteNode)); 

    printf("DELETE\n");
    // for(int i=30; i<37;i++)
    // {
        // delete->key = 1; 
        // delete->ptr = -2; 
        //deleteBTreeKey(testTree,update,delete);
        ioCount = 0; 
        deleteDBKey(testTree, 1);

        printRootKeys(testTree);
        printf("%f \n",resultPtr);

        overflow = (OverflowNode*)(uintptr_t)(child->ptrs[0]);
        for(int j=0;j<OVERFLOW_RECS;j++){
            printf("%.0f ",overflow->dataBlocks[j]);
        } 
        printf("\n");
        overflow = (OverflowNode*)(uintptr_t)(child->ptrs[1]);
        for(int j=0;j<OVERFLOW_RECS;j++){
            printf("%.0f ",overflow->dataBlocks[j]);
        } 
        printf("\n");

        if(overflow->next == NULL) printf("Null block\n");
    // }
    printRootKeys(testTree);
    printf("The number of nodes in the tree = %d\n", countNode(testTree));
    printf("IO count = %d\n",ioCount);

    for(int j= 0;j<5;j++){
        printf("%d ", testTree->page->types[j]);
    }
    
    

}

