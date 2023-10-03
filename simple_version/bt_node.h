#ifndef BT_NODE_H
#define BT_NODE_H

#include "storage.h"
#include "bt_page.h"
#include <string.h>

// defining fixed variables and global variables
#define NODE_SIZE 400
#define N (400-sizeof(double))/(sizeof(float)+sizeof(double))
#define OVERFLOW_RECS (400-sizeof(double))/sizeof(double)

// declare objects 

/**
 * Object representing the non-leaf nodes, containing n keys and n+1 pointers to next node. 
*/
typedef struct _nonleafnode{
    float keys[N];
    double ptrs[N + 1];
} NonLeafNode;

/**
 * Object representing the leaf nodes, containing n keys and n+1 pointers. 
*/
typedef struct _leafnode{
    float keys[N];
    double ptrs[N];
    struct _leafnode *next;
} LeafNode;

/**
 * Object representing the overflow nodes, containing data blocks. 
*/
typedef struct _overflownode{
    double dataBlocks[OVERFLOW_RECS];
    struct _overflownode *next;
} OverflowNode;

/**
 * Object containing information of node to be inserted/updated during insertion. 
*/
typedef struct _insertnode{
    float key; 
    double ptr; 
}InsertNode; 


/**
 * object containing information of nodes to be deleted/updated during deletion. 
*/
typedef struct _deletenode{
    float key;
    double ptr; 
    float oldKey;
    float newKey; 
}DeleteNode; 

// declare functions
NonLeafNode* createNonleafNode(double ptr1, float key2, double ptr2);
int insertNonLeafNodeKey(NonLeafNode* node, InsertNode* insertNode);
void insertNLNodeKey(NonLeafNode* node,float key, double ptr);
int searchNonLeafNodeKey(NonLeafNode* node, float key);
double searchNonLeafNode(NonLeafNode* node, float key);
int deleteNLNodeKey(NonLeafNode* node, float key);
int deleteNonLeafNodeKey(NonLeafNode* node, DeleteNode* deleteNode);
int deleteNonleafNode(NonLeafNode* node, DeleteNode* deleteNode);
float retrieveNonLeafLowerboundKey(BTPage* page, double node);

LeafNode* createLeafNode(float key, double ptr);
int insertLeafNodeKey(LeafNode* node, InsertNode* insertNode);
void insertLNodeKey(LeafNode* node,float key, double ptr);
int searchLeafNodeKey(LeafNode* node, float key);
double searchLeafNode(LeafNode* node, float key);
int deleteLNodeKey(LeafNode* node, float key);
int deleteLeafNodeKey(LeafNode* leftNode,LeafNode* node, DeleteNode* deleteNode);
double deleteLeafNode(LeafNode* node);

OverflowNode* createOverflowNode(double dataPtr);
int insertOverflowRecord(OverflowNode* node, InsertNode* insertNode);
int deleteOverflowRecord(OverflowNode* node, DeleteNode* deleteNode);
OverflowNode* lastOverflowNode(OverflowNode *node);
double CheckOverflowNode(OverflowNode* node);

int insertKey(BTPage *page, double nodePtr, InsertNode* insertNode);
int deleteKey(BTPage* page, double leftPtr, double nodePtr, double rightPtr, InsertNode* insertNode);






#endif //BT_NODE_H