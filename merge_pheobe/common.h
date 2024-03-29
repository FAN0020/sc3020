#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"
#include "b+tree/bt_page.h"
#include "b+tree/bt_node.h"
#include "b+tree/bt_mgr.h"

#define LEAF_NODE_TYPE 2

// Function declarations for the functions defined in common.c
int countNodes(double nodePtr, BTPage *page);
int countLevels(double nodePtr, BTPage *page);

int countNodes(double nodePtr, BTPage *page) {
    if (nodePtr == -1) return 0;

    int type = searchPageRecord(page, nodePtr);
    if (type == LEAF_NODE_TYPE) {
        return 1;
    } else {
        NonLeafNode *node = (NonLeafNode*)searchPageRecord(page, nodePtr);
        int count = 1;
        for (int i = 0; i <= N && node->ptrs[i] != 0; i++) {
            count += countNodes(node->ptrs[i], page);
        }
        return count;
    }
}

int countLevels(double nodePtr, BTPage *page) {
    if (nodePtr == -1) return 0;

    int type = searchPageRecord(page, nodePtr);
    if (type == LEAF_NODE_TYPE) {
        return 1;
    } else {
        NonLeafNode *node = (NonLeafNode*)searchPageRecord(page, nodePtr);
        return 1 + countLevels(node->ptrs[0], page);  // assuming balanced tree
    }
}

#endif // COMMON_H
