#ifndef BT_NODE_H
#define BT_NODE_H

#include "bt_page.h"
#include <string.h>
#include <stdint.h>
// defining fixed variables and global variables
#define NODE_SIZE 400
#define N (400-sizeof(double))/(sizeof(double)+sizeof(double))
#define OVERFLOW_RECS (400-sizeof(double))/sizeof(double)

// declare global variables and objects 
int ioCount = 0;
double resultPtr = -1;


/**
 * Object representing the non-leaf nodes, containing n keys and n+1 pointers to next node. 
*/
typedef struct _nonleafnode{
    double keys[N];
    double ptrs[N + 1];
} NonLeafNode;

/**
 * Object representing the leaf nodes, containing n keys and n+1 pointers. 
*/
typedef struct _leafnode{
    double keys[N];
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
    double key; 
    double ptr; 
}InsertNode; 


/**
 * object containing information of nodes to be deleted/updated during deletion. 
*/
typedef struct _deletenode{
    double key;
    double ptr; 
    double oldKey;
    double newKey; 
}DeleteNode; 

typedef struct _updateNode{
    double root; 
    double lNode; 
    double cNode;
    double rNode; 
}UpdateNode;

// declare functions
NonLeafNode* createNonleafNode(double ptr1, double key2, double ptr2);
int insertNonLeafNodeKey(NonLeafNode* node, InsertNode* insertNode);
void insertNLNodeKey(NonLeafNode* node,double key, double ptr,int isLowerbound);
int searchNonLeafNodeKey(NonLeafNode* node, double key);
double searchNonLeafNode(NonLeafNode* node, double key);
int deleteNLNodeKey(NonLeafNode* node, double key);
int deleteNonLeafNodeKey(BTPage *page, double root,UpdateNode* updateInfo ,DeleteNode* deleteNode);
double deleteNonLeafNode(NonLeafNode* node);
double retrieveNonLeafLowerboundKey(BTPage* page, double node);

LeafNode* createLeafNode(double key, double ptr);
int insertLeafNodeKey(LeafNode* node, InsertNode* insertNode);
void insertLNodeKey(LeafNode* node,double key, double ptr);
int searchLeafNodeKey(LeafNode* node, double key);
double searchLeafNode(LeafNode* node, double key);
int deleteLNodeKey(LeafNode* node, double key);
int deleteLeafNodeKey(double root, UpdateNode* updateInfo, DeleteNode* deleteNode);
double deleteLeafNode(LeafNode* node);

OverflowNode* createOverflowNode(double dataPtr);
int insertOverflowRecord(OverflowNode* node, InsertNode* insertNode);
OverflowNode* lastOverflowNode(OverflowNode *node);
double CheckOverflowNode(OverflowNode* node);
int deleteOverflowRecord(BTPage *page,OverflowNode* node, DeleteNode* deleteNode);
double deleteOverflowNode(BTPage *page,OverflowNode* node, OverflowNode* delNode);

int insertKey(BTPage *page, double nodePtr, InsertNode* insertNode);
int deleteKey(BTPage *page,double root, double nodePtr,DeleteNode* deleteNode, UpdateNode* nodeInfo);
int getNodes(BTPage* page, double node,double key,double ptr,UpdateNode* nodeInfo);
void UpdateLowerboundKeys(double node, double oldKey, double newKey);

double searchKey(BTPage *page, double node, double key);
double searchRangeKey(BTPage *page, double node, double key);


#endif //BT_NODE_H