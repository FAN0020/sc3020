//
// Created by CapnDrove on 6/10/2023.
//
#include "bt_page.h"
#include "bt_node.h"
#include "bt_mgr.h"

#include "bt_page.c"
#include "bt_node.c"
#include "bt_mgr.c"

#include <stdio.h>
#include <stdint.h>
int main() {
    char field[20] = "test";
    BTree *testTree = createTree(field);
    InsertNode *insertNode = (InsertNode *) malloc(sizeof(InsertNode));
    NonLeafNode *root;

    int value = 36 * 16 + 1;;
    for (int i = 1; i <= value; i++) {
        printf("i = %d \n", i);
        insertNode->key = value - i + 1;
        insertNode->ptr = (double)(uintptr_t)&insertNode->key;
        insertBTreeKey(testTree, insertNode);
    }
    UpdateNode *update = (UpdateNode *) malloc(sizeof(UpdateNode));
    DeleteNode *delete = (DeleteNode *) malloc(sizeof(DeleteNode));
    for (int k = 0; k < 50; k++){
        double a = searchBTreeKey(testTree, k);
        printf("%f", a);
    }
}