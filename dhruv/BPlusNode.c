#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "BPlusNode.h"

#define ORDER 3

// A utility function that returns the index of the first key that is
// greater than or equal to k
BPlusNode* createNode(int min_size, bool is_leaf){
    BPlusNode * node = malloc(sizeof (BPlusNode));
    node->min_size = min_size;
    node->leaf = is_leaf;
    node->key_list = malloc(sizeof(float) * 2 * (node->min_size - 1)) ;
    node->num_keys = 0;
    return node;
}

int findKey(BPlusNode* node, float search_key)
{
    int idx = 0;
    while (idx < node->num_keys && node->key_list[idx] < search_key) {
        ++idx;
    }
    return idx;
}


// A function to get predecessor of key_list[idx]
float getPrevious(BPlusNode* node, int idx){
    // Keep moving to the right most node until we reach a leaf
    BPlusNode* cur = node->children[idx];
    while (!cur->leaf)
        cur = cur->children[cur->num_keys];

    // Return the last key of the leaf
    return cur->key_list[cur->num_keys - 1];
}

// A function to get successor of key_list[idx]
float getNext(BPlusNode* node, int idx)
{

    // Keep moving the left most node starting from C[idx+1] until we reach a leaf
    BPlusNode* cur = node->children[idx + 1];
    while (!cur->leaf)
        cur = cur->children[0];

    // Return the first key of the leaf
    return cur->key_list[0];
}

// A function to borrow a key from C[idx-1] and insertNodeToTree it into C[idx]
void borrowFromPrev(BPlusNode* node, int idx)
{

    BPlusNode* child = node->children[idx];
    BPlusNode* sibling = node->children[idx - 1];

    // The last key from C[idx-1] goes up to the parent and key[idx-1]
    // from parent is inserted as the first key in C[idx]. Thus, the loses
    // sibling one key and child gains one key

    // Moving all key in C[idx] one step ahead
    for (int i = child->num_keys - 1; i >= 0; --i)
        child->key_list[i + 1] = child->key_list[i];

    // If C[idx] is not a leaf, move all its child pointers one step ahead
    if (!child->leaf)
    {
        for (int i = child->num_keys; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    // Setting child's first key equal to key_list[idx-1] from the current node
    child->key_list[0] = node->key_list[idx - 1];

    // Moving sibling's last child as C[idx]'s first child
    if (!child->leaf)
        child->children[0] = sibling->children[sibling->num_keys];

    // Moving the key from the sibling to the parent
    // This reduces the number of key_list in the sibling
    node->key_list[idx - 1] = sibling->key_list[sibling->num_keys - 1];

    child->num_keys += 1;
    sibling->num_keys -= 1;
}

// A function to borrow a key from the C[idx+1] and place
// it in C[idx]
void borrowFromNext(BPlusNode * node, int idx){

    BPlusNode* child = node->children[idx];
    BPlusNode* sibling = node->children[idx + 1];

    // key_list[idx] is inserted as the last key in C[idx]
    child->key_list[(child->num_keys)] = node->key_list[idx];

    // Sibling's first child is inserted as the last child
    // into C[idx]
    if (!(child->leaf))
        child->children[(child->num_keys) + 1] = sibling->children[0];

    //The first key from sibling is inserted into key_list[idx]
    node->key_list[idx] = sibling->key_list[0];

    // Moving all key_list in sibling one step behind
    for (int i = 1; i < sibling->num_keys; ++i)
        sibling->key_list[i - 1] = sibling->key_list[i];

    // Moving the child pointers one step behind
    if (!sibling->leaf)
    {
        for (int i = 1; i <= sibling->num_keys; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }

    // Increasing and decreasing the key count of C[idx] and C[idx+1]
    // respectively
    child->num_keys += 1;
    sibling->num_keys -= 1;
}

// A function to mergeNode C[idx] with C[idx+1]
// C[idx+1] is freed after merging
void mergeNode(BPlusNode * node, int idx){
    BPlusNode* child = node->children[idx];
    BPlusNode* sibling = node->children[idx + 1];

    // Pulling a key from the current node and inserting it into (minSize-1)th
    // position of C[idx]
    child->key_list[node->min_size - 1] = node->key_list[idx];

    // Copying the key_list from C[idx+1] to C[idx] at the end
    for (int i = 0; i < sibling->num_keys; ++i)
        child->key_list[i + node->min_size] = sibling->key_list[i];

    // Copying the child pointers from C[idx+1] to C[idx]
    if (!child->leaf)
    {
        for (int i = 0; i <= sibling->num_keys; ++i)
            child->children[i + node->min_size] = sibling->children[i];
    }

    // Moving all key_list after idx in the current node one step before -
    // to fillNode the gap created by moving key_list[idx] to C[idx]
    for (int i = idx + 1; i < node->num_keys; ++i)
        node->key_list[i - 1] = node->key_list[i];

    // Moving the child pointers after (idx+1) in the current node one
    // step before
    for (int i = idx + 2; i <= node->num_keys; ++i)
        node->children[i - 1] = node->children[i];

    // Updating the key count of child and the current node
    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    // Freeing the memory occupied by sibling
    free(sibling);
}

// A function to fillNode child C[idx] which has less than minSize-1 key_list
void fillNode(BPlusNode* node, int idx)
{

    // If the previous child(C[idx-1]) has more than minSize-1 key_list, borrow a key
    // from that child
    if (idx != 0 && node->children[idx - 1]->num_keys >= node->min_size)
        borrowFromPrev(node, idx);

        // If the next child(C[idx+1]) has more than minSize-1 key_list, borrow a key
        // from that child
    else if (idx != node->num_keys && node->children[idx + 1]->num_keys >= node->min_size)
        borrowFromNext(node, idx);

        // Merge C[idx] with its sibling
        // If C[idx] is the last child, mergeNode it with its previous sibling
        // Otherwise mergeNode it with its next sibling
    else
    {
        if (idx != node->num_keys)
            mergeNode(node, idx);
        else
            mergeNode(node, idx - 1);
    }
}

// A utility function to insertNodeToTree a new key in this node
// The assumption is, the node must be non-full when this
// function is called
void insertNonFull(BPlusNode* node, float key_value){
    // Initialize index as index of rightmost element
    int i = node->num_keys - 1;

    // If this is a leaf node
    if (node->leaf)
    {
        // The following loop does two things
        // a) Finds the location of new key to be inserted
        // b) Moves all greater key_list to one place ahead
        while (i >= 0 && node->key_list[i] > key_value)
        {
            node->key_list[i + 1] = node->key_list[i];
            i--;
        }

        // Insert the new key at found location
        node->key_list[i + 1] = key_value;
        node->num_keys++;
    }
    else // If this node is not leaf
    {
        // Find the child which is going to have the new key
        while (i >= 0 && node->key_list[i] > key_value)
            i--;

        // See if the found child is full
        if (node->children[i + 1]->num_keys == 2 * node->min_size - 1)
        {
            // If the child is full, then split it
            splitChild(node, i + 1, node->children[i + 1]);

            // After split, the middle key of C[i] goes up and
            // C[i] is split into two. See which of the two
            // is going to have the new key
            if (node->key_list[i + 1] < key_value)
                i++;
        }
        insertNonFull(node->children[i + 1], key_value);
    }
}


void removeNodeFromLeaf(BPlusNode* node, int idx)
{

    // Move all the key_list after the idx-th pos one place backward
    for (int i = idx + 1; i < node->num_keys; ++i) {
        node->key_list[i - 1] = node->key_list[i];
    }
    // Reduce the count of key_list
    node->num_keys--;
}


void removeNodeFromNonLeaf(BPlusNode * node, int idx)
{

    float k = node->key_list[idx];
    if (node->children[idx]->num_keys >= node->min_size)
    {
        float pred = getPrevious(node, idx);
        node->key_list[idx] = pred;
        removeNode(node->children[idx], pred);
    }
    else if (node->children[idx + 1]->num_keys >= node->min_size)
    {
        float succ = getNext(node, idx);
        node->key_list[idx] = succ;
        removeNode(node->children[idx + 1], succ);
    }
    else
    {
        mergeNode(node, idx);
        removeNode(node->children[idx], k);
    }
}

// A function to remove the key k from the subtree rooted with this node
void removeNode(BPlusNode* node, float key_value)
{
    int idx = findKey(node, key_value);
    if (idx < node->num_keys && node->key_list[idx] == key_value)
    {
        if (node->leaf) {
            removeNodeFromLeaf(node, idx);
        }
        else {
            removeNodeFromNonLeaf(node, idx);
        }
    }
    else
    {
        if (node->leaf){
            return;
        }
        bool flag = (idx == node->num_keys) != 0;
        if (node->children[idx]->num_keys < node->min_size)
            fillNode(node, idx);
        if (flag && idx > node->num_keys)
            removeNode(node->children[idx - 1], key_value);
        else
            removeNode(node->children[idx], key_value);
    }
}


void splitChild(BPlusNode* node, int on_index, BPlusNode* child_node)
{
    BPlusNode* z = createNode(child_node->min_size, child_node->leaf);
    z->num_keys = node->min_size - 1;

    for (int j = 0; j < node->min_size - 1; j++) {
        z->key_list[j] = child_node->key_list[j + node->min_size];
    }

    if (!child_node->leaf)
    {
        for (int j = 0; j < node->min_size; j++) {
            z->children[j] = child_node->children[j + node->min_size];
        }
    }

    child_node->num_keys = node->min_size - 1;
    for (int j = node->num_keys; j >= on_index + 1; j--) {
        node->children[j + 1] = node->children[j];
    }
    node->children[on_index + 1] = z;
    for (int j = node->num_keys - 1; j >= on_index; j--)
        node->key_list[j + 1] = node->key_list[j];

    node->key_list[on_index] = child_node->key_list[node->min_size - 1];

    node->num_keys++;
}


void traverse(BPlusNode* node){

    int i;
    for (i = 0; i < node->num_keys; i++)
    {
        if (!node->leaf) {
            traverse(node->children[i]);
        }
        printf("%f\n", node->key_list[i]);
    }
    if (!node->leaf) {
        traverse(node->children[i]);
    }
}


BPlusNode* search(BPlusNode* node, float search_key)
{
    int i = 0;
    while (i < node->num_keys && search_key > node->key_list[i]) {
        i++;
        if (node->key_list[i] == search_key) {
            return node;
        }
        if (node->leaf) {
            return NULL;
        }
    }
    return search(node->children[i], search_key);
}

