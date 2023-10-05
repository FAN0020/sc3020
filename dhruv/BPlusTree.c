//
// Created by CapnDrove on 5/10/2023.
//
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "BPlusNode.h"

typedef struct BPlusTree
{
    BPlusNode* root;
    int minSize;
}BPlusTree;

BPlusTree * createTree(int minNodeSize){
    BPlusTree * tree = malloc(sizeof(BPlusTree));
    tree->root = NULL;
    tree->minSize = minNodeSize;
    return tree;
}

void insertNodeToTree(BPlusTree* tree, float key_value);


void insertNodeToTree(BPlusTree* tree, float key_value){
    if (tree->root == NULL){
        tree->root = createNode(tree->minSize, true);
        tree->root->key_list[0] = key_value;
        tree->root->num_keys = 1;
    }
    else{
        if (tree->root->num_keys == 2 * tree->minSize - 1){
            BPlusNode* s = createNode(tree->minSize, false);
            s->children[0] = tree->root;
            splitChild(s, 0, tree->root);
            int i = 0;
            if (s->key_list[0] < key_value)
                i++;
            insertNonFull(s->children[i], key_value);
            tree->root = s;
        }
        else {
            insertNonFull(tree->root, key_value);
        }
    }
}

void tree_traverse(BPlusTree * tree){
    if (tree->root) { traverse(tree->root); }
}


BPlusNode* tree_search(BPlusTree* tree, float search_key){
    return (tree->root == NULL) ? NULL : search(tree->root, search_key);
}

void tree_remove(BPlusTree* tree, float key_value){
    if (!tree->root){
        printf("Tree is empty!!\n");
        return;
    }
    removeNode(tree->root, key_value);

    if (tree->root->num_keys == 0){
        BPlusNode* tmp = tree->root;
        if (tree->root->leaf)
            tree->root = NULL;
        else
            tree->root = tree->root->children[0];
        free( tmp);
    }
}


int main()
{
    BPlusTree * t = createTree(3); // A B-Tree with minimum degree 3

    insertNodeToTree(t, 1);
    insertNodeToTree(t, 7);
    insertNodeToTree(t, 3);
    insertNodeToTree(t, 1);
    insertNodeToTree(t, 10);
    insertNodeToTree(t, 11);
    insertNodeToTree(t, 13);
    insertNodeToTree(t, 14);
    insertNodeToTree(t, 15);
    insertNodeToTree(t, 18);
    insertNodeToTree(t, 16);
    insertNodeToTree(t, 19);

    printf("Traversal of tree\n");
    tree_traverse(t);
    printf("\n");

    tree_remove(t, 6);
    printf("Traversal of tree after removing 6\n");
    tree_traverse(t);
    printf("\n");

    tree_remove(t, 13);
    printf("Traversal of tree after removing 13\n");
    tree_traverse(t);
    printf("\n");

    return 0;
}

