#ifndef BT_MGR_H
#define BT_MGR_H

#include "bt_node.h"
#include "bt_page.h"
#include <string.h>

/**
 * Object containing the tree. 
 * It contains the field the records are indexed by and the pointer ot the root of the tree.
*/
typedef struct _btree{
    char field[20];
    double root;
    BTPage* page;
}BTree;

// declare functions
BTree* createTree(char field[20]);
void insertBTreeKey(BTree* tree,InsertNode* insertNode);
void deleteBTreeKey(BTree* tree, UpdateNode* updateNode, DeleteNode* deleteInfo);
void deleteBTree(BTree* tree);
double searchBTreeKey(BTree *tree, float key);
double searchBTreeRangeKey(BTree *tree, float key);
void printRootKeys(BTree *tree);
int countNode(BTree *tree);
int countLevel(BTPage *page, double node);

#endif //BT_MGR_H