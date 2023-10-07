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

int countManualNodes(double node, BTPage *page){
    int count=0, type = searchPageRecord(page, node);
    if(type != 1){
        return 0; 
    }
    else{
        NonLeafNode *nlNode = (NonLeafNode*)(uintptr_t)node;
        count = 1 + countManualNodes(nlNode->ptrs[0],page);
        for(int i = 0; i<N; i++){
            if(nlNode->keys[i] != -1){
                count += 1 + countManualNodes(nlNode->ptrs[i+1],page);
            }
        }
        return count;
    }
}


int main(){
    char field[20] = "test";
    BTree* testTree = createTree(field);
    InsertNode* insertNode = (InsertNode*) malloc(sizeof(InsertNode));
    NonLeafNode* root;

    int value = 26651;
    for(int i=1; i<=value;i++){
        printf("i = %d \n",i);
        insertNode->key = value-i+1;
        insertNode->ptr = value-i+1; 
        insertBTreeKey(testTree,insertNode);
    }

    printf("%s \n","Print Tree:");
    if(searchPageRecord(testTree->page,testTree->root) == 2){
        LeafNode *child = (LeafNode*)(uintptr_t)testTree->root;
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

    UpdateNode *update = (UpdateNode*) malloc(sizeof(UpdateNode));
    DeleteNode *delete = (DeleteNode*)malloc(sizeof(DeleteNode)); 

    printf("The number of nodes in the tree = %d\n", countNode(testTree));
    printf("The number of levels in the tree = %d\n", countLevel(testTree->page,testTree->root));

    printf("The number of nodes in the tree = %d\n", countManualNodes(testTree->root,testTree->page)+1);
    
    for(int j = 343; j<365; j++){
        
        delete->key = j; 
        delete->ptr = j; 

    
        deleteBTreeKey(testTree,update,delete);
    }

    for(int j = 50; j<75; j++){
        
        delete->key = j; 
        delete->ptr = j; 

    
        deleteBTreeKey(testTree,update,delete);
    }
    
    
    printf("%s \n","Print Tree:");
    if(searchPageRecord(testTree->page,testTree->root) == 2){
        LeafNode *child = (LeafNode*)(uintptr_t)testTree->root;
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
    // int k;
    // for(k=N-1;k>=0;k--){
    //     if (root->keys[k] != 1) break;
    // }

    // for(int l=0;l<=k;l++){
    //     printf("root pointer %d \n", l);
    //     NonLeafNode *child1 = (NonLeafNode*) (uintptr_t) (root->ptrs[l]);
    //     for(int j=0;j<N;j++){
    //         printf("%.0f ",child1->ptrs[j]);
    //         printf("%.0f ",child1->keys[j]);
    //     }
    //     printf("%.0f \n",child1->ptrs[N]);
    // }
    

    // LeafNode *child = (LeafNode*)(uintptr_t) (child1->ptrs[16]);
    // for(int j=0;j<N;j++){
    //     printf("%.0f ",child->ptrs[j]);
    //     printf("%.0f ",child->keys[j]);
    // }
    // printf("%.0f \n",child->next);

    // Count the non-list node
    
}