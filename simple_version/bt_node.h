#ifndef BT_NODE_H
#define BT_NODE_H

#include "storage.h"
#include "storage.c"
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

typedef struct _node{
    float key; 
    double ptr; 
}Node; 

// declare functions
NonLeafNode* createNonleafNode(double ptr1, float key2, double ptr2);
int insertNonLeafNodeKey(NonLeafNode* node, Node* insertNode);
void insertNLNodeKey(NonLeafNode* node,float key, double ptr);
int searchNonLeafNodeKey(NonLeafNode* node, float key);
double searchNonLeafNode(NonLeafNode* node, float key);
int deleteNonLeafNodeKey(NonLeafNode* node, float key);

LeafNode* createLeafNode(float key, double ptr);
int insertLeafNodeKey(LeafNode* node, Node* insertNode);
void insertLNodeKey(LeafNode* node,float key, double ptr);
int searchLeafNodeKey(LeafNode* node, float key);
double searchLeafNode(LeafNode* node, float key);
int deleteLeafNodeKey(LeafNode* node, float key);

OverflowNode* createOverflowNode(double dataPtr);
int insertOverflowRecord(OverflowNode* node, double dataPtr);








#endif //BT_NODE_H