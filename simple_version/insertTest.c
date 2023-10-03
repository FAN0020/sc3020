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
    BTPage* page = createPage();
    BTree* testTree = createTree(field);
    Node* insertNode = (Node*) malloc(sizeof(Node));
    NonLeafNode* root;

    int value = 36*16+1;
    for(int i=1; i<=value;i++){
        printf("i = %d \n",i);
        insertNode->key = value-i+1;
        insertNode->ptr = value-i+1; 
        insertBTreeKey(page,testTree,insertNode);
        if(searchPageRecord(page,testTree->root) == 2){
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
    }
    
    NonLeafNode *child1 = (NonLeafNode*) (uintptr_t) (root->ptrs[1]);
    for(int j=0;j<N;j++){
        printf("%.0f ",child1->ptrs[j]);
        printf("%.0f ",child1->keys[j]);
    }
    printf("%.0f \n",child1->ptrs[N]);

    LeafNode *child = (LeafNode*)(uintptr_t) (child1->ptrs[0]);
    for(int j=0;j<N;j++){
        printf("%.0f ",child->ptrs[j]);
        printf("%.0f ",child->keys[j]);
    }
    printf("%.0f \n",child->next);

}