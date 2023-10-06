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

    int value = 36*16+1;;
    for(int i=1; i<=value;i++){
        printf("i = %d \n",i);
        insertNode->key = value-i+1;
        insertNode->ptr = value-i+1; 
        insertBTreeKey(testTree,insertNode);
    }

    printf("%s \n","Print Tree:");
    if(searchPageRecord(testTree->page,testTree->root) == 2){
        LeafNode *child = (LeafNode*)(uintptr_t)testTree->root;
        for(int j=0;j<(int)N;j++){
            printf("%.0f ",child->ptrs[j]);
            printf("%.0f ",child->keys[j]);
        }
        // printf("%.0f \n",child->next);
    }
    else{
        root = (NonLeafNode*) (uintptr_t) testTree->root;
        for(int j=0;j<(int)N;j++){
            printf("%.0f ",root->ptrs[j]);
            printf("%.0f ",root->keys[j]);
        }
        printf("%.0f \n",root->ptrs[N]);
    }

    UpdateNode *update = (UpdateNode*) malloc(sizeof(UpdateNode));
    DeleteNode *deleteNode = (DeleteNode*)malloc(sizeof(DeleteNode)); 

    for(int j = 343; j<365; j++){
        
        deleteNode->key = j; 
        deleteNode->ptr = j; 

    
        deleteBTreeKey(testTree,update,deleteNode);
    }

    for(int j = 50; j<75; j++){
        
        deleteNode->key = j; 
        deleteNode->ptr = j; 

    
        deleteBTreeKey(testTree,update,deleteNode);
    }
    
    
    printf("%s \n","Print Tree:");
    if(searchPageRecord(testTree->page,testTree->root) == 2){
        LeafNode *child = (LeafNode*)(uintptr_t)testTree->root;
        for(int j=0;j<(int)N;j++){
            printf("%.0f ",child->ptrs[j]);
            printf("%.0f ",child->keys[j]);
        }
        // printf("%.0f \n",child->next);
    }
    else{
        root = (NonLeafNode*) (uintptr_t) testTree->root;
        for(int j=0;j<(int)N;j++){
            printf("%.0f ",root->ptrs[j]);
            printf("%.0f ",root->keys[j]);
        }
        printf("%.0f \n",root->ptrs[N]);
    }
    // int k;
    // for(k=N-1;k>=0;k--){
    //     if (root->keys[k] != 1) break;
    // }

    // for(int l=0;l<=k;l++){
    //     printf("root pointer %d \n", l);
    //     NonLeafNode *child1 = (NonLeafNode*) (uintptr_t) (root->ptrs[l]);
    //     for(int j=0;j<(int)N;j++){
    //         printf("%.0f ",child1->ptrs[j]);
    //         printf("%.0f ",child1->keys[j]);
    //     }
    //     printf("%.0f \n",child1->ptrs[N]);
    // }
    

    // LeafNode *child = (LeafNode*)(uintptr_t) (child1->ptrs[16]);
    // for(int j=0;j<(int)N;j++){
    //     printf("%.0f ",child->ptrs[j]);
    //     printf("%.0f ",child->keys[j]);
    // }
    // printf("%.0f \n",child->next);

}