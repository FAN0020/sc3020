//
// Created by CapnDrove on 5/10/2023.
//

#ifndef UNTITLED1_BPLUSNODE_H
#define UNTITLED1_BPLUSNODE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define ORDER 3


typedef struct BPlusNode{
    float * key_list;
    int min_size;
    int num_keys;
    struct BPlusNode* children[ORDER+1];
    bool leaf;
} BPlusNode;

BPlusNode* createNode(int min_size, bool is_leaf);

int findKey(BPlusNode* node, float search_key);

float getPrevious(BPlusNode* node, int idx);
float getNext(BPlusNode* node, int idx);

void borrowFromPrev(BPlusNode* node, int idx);
void borrowFromNext(BPlusNode * node, int idx);

void mergeNode(BPlusNode * node, int idx);
void fillNode(BPlusNode* node, int idx);


void splitChild(BPlusNode* node, int on_index, BPlusNode* child_node);

void insertNonFull(BPlusNode* node, float key_value);

void removeNode(BPlusNode* node, float key_value);
void removeNodeFromNonLeaf(BPlusNode * node, int idx);
void removeNodeFromLeaf(BPlusNode* node, int idx);

void traverse(BPlusNode* node);
BPlusNode* search(BPlusNode* node, float search_key);
#endif //UNTITLED1_BPLUSNODE_H
