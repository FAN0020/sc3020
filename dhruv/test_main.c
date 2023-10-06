#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define ORDER 4 // Adjust the order of the B+ tree as needed
// Define B+ tree node structure
typedef struct BPlusNode {
    int keys[ORDER - 1];
    struct BPlusNode* children[ORDER];
    bool is_leaf;
    int current_number_of_keys;
    struct BPlusNode* next; // Pointer to the next leaf node (used for range queries)
} BPlusNode;

// Define B+ tree class
typedef struct BPlusTree {
    BPlusNode* root;
} BPlusTree;

BPlusNode* createNode();
BPlusTree* initializeTree();
void insertInNode(BPlusNode* node, int key);
void splitNode(BPlusTree* tree, BPlusNode* parent, BPlusNode* child, int index);
void insert(BPlusTree* tree, int key);
int findMin(BPlusNode* node);
void removeFromLeaf(BPlusNode* node, int key);
void removeFromNonLeaf(BPlusTree* tree, BPlusNode* node, int key, int index);
void mergeNodes(BPlusTree* tree, BPlusNode* parent, BPlusNode* child, BPlusNode* sibling, int index);
void removeFromTree(BPlusTree* tree, BPlusNode* node, int key);
void borrowFromPrev(BPlusNode* node, int index);
void borrowFromNext(BPlusNode* node, int index);
void deleteKey(BPlusTree* tree, int key);

BPlusNode * search(BPlusTree* tree, int key);
void displayTree(BPlusNode* node);
void inOrderTraversal(BPlusNode* node);
BPlusNode * tree_search(BPlusNode * node, int K);

// Create a new B+ tree node
BPlusNode* createNode() {
    BPlusNode* newNode = (BPlusNode*)malloc(sizeof(BPlusNode));
    newNode->is_leaf = true;
    newNode->current_number_of_keys = 0;
    newNode->next = NULL;
    for (int i = 0; i < ORDER; i++) {
        newNode->children[i] = NULL;
    }
    return newNode;
}

// Initialize a new B+ tree
BPlusTree* initializeTree() {
    BPlusTree* tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    tree->root = createNode();
    return tree;
}

// Helper function to insert a key into a node
void insertInNode(BPlusNode* node, int key) {
    int i = node->current_number_of_keys - 1;
    while (i >= 0 && key < node->keys[i]) {
        node->keys[i + 1] = node->keys[i];
        i--;
    }
    node->keys[i + 1] = key;
    node->current_number_of_keys++;
}

// Helper function to split a node
void splitNode(BPlusTree* tree, BPlusNode* parent, BPlusNode* child, int index) {
    BPlusNode* newNode = createNode();
    newNode->is_leaf = child->is_leaf;
    newNode->next = child->next;
    child->next = newNode;

    int split_point = (ORDER - 1) / 2;  // Updated split point
    for (int i = split_point, j = 0; i < ORDER - 1; i++, j++) {
        newNode->keys[j] = child->keys[i];
        child->keys[i] = 0;
        newNode->children[j] = child->children[i];
        child->children[i] = NULL;
        newNode->current_number_of_keys++;
        child->current_number_of_keys--;
    }

    insertInNode(parent, child->keys[split_point]);

    for (int i = index + 1; i < parent->current_number_of_keys; i++) {
        parent->children[i + 1] = parent->children[i];
    }

    parent->children[index + 1] = newNode;
}

// Insert a key into the B+ tree
void insert(BPlusTree* tree, int key) {
    BPlusNode* root = tree->root;

    if (root->current_number_of_keys == ORDER - 1) {
        BPlusNode* newNode = createNode();
        newNode->children[0] = root;
        tree->root = newNode;  // Update the root node here

        // Split the old root
        splitNode(tree, newNode, root, 0);
        insertInNode(newNode, key);
    } else {
        BPlusNode* current = root;
        while (!current->is_leaf) {
            int i = 0;
            while (i < current->current_number_of_keys && key >= current->keys[i]) {
                i++;
            }
            if (current->children[i]->current_number_of_keys == ORDER - 1) {
                splitNode(tree, current, current->children[i], i);
                if (key >= current->keys[i]) {
                    i++;
                }
            }
            current = current->children[i];
        }
        insertInNode(current, key);
    }
}

// Helper function to find the minimum key in a B+ tree
int findMin(BPlusNode* node) {
    if (node == NULL) {
        return -1; // Return a sentinel value if the tree is empty
    }
    while (!node->is_leaf) {
        node = node->children[0];
    }
    return node->keys[0];
}

// Helper function to remove a key from a leaf node
void removeFromLeaf(BPlusNode* node, int key) {
    int i = 0;
    while (i < node->current_number_of_keys && key != node->keys[i]) {
        i++;
    }
    if (i < node->current_number_of_keys) {
        for (int j = i + 1; j < node->current_number_of_keys; j++) {
            node->keys[j - 1] = node->keys[j];
        }
        node->current_number_of_keys--;
    }
}

// Helper function to remove a key from a non-leaf node
void removeFromNonLeaf(BPlusTree* tree, BPlusNode* node, int key, int index) {
    int k = node->keys[index];
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = (index == 0) ? node->children[index + 1] : node->children[index - 1];

    if (child->current_number_of_keys >= (ORDER + 1) / 2) {
        int pred = findMin(child);
        removeFromTree(tree, child, pred);
        node->keys[index] = pred;
    } else if (sibling->current_number_of_keys >= (ORDER + 1) / 2) {
        int succ = findMin(sibling);
        removeFromTree(tree, sibling, succ);
        node->keys[index] = succ;
    } else {
        mergeNodes(tree, node, child, sibling, index);
        removeFromTree(tree, child, k);
    }
}

// Helper function to merge two nodes
void mergeNodes(BPlusTree* tree, BPlusNode* parent, BPlusNode* child, BPlusNode* sibling, int index) {
    int merge_index = parent->keys[index];
    child->keys[child->current_number_of_keys] = merge_index;

    for (int i = 0, j = child->current_number_of_keys + 1; i < sibling->current_number_of_keys; i++, j++) {
        child->keys[j] = sibling->keys[i];
    }

    if (!child->is_leaf) {
        for (int i = 0, j = child->current_number_of_keys + 1; i <= sibling->current_number_of_keys; i++, j++) {
            child->children[j] = sibling->children[i];
        }
    }

    for (int i = index + 1; i < parent->current_number_of_keys; i++) {
        parent->keys[i - 1] = parent->keys[i];
        parent->children[i] = parent->children[i + 1];
    }

    child->current_number_of_keys += sibling->current_number_of_keys + 1;
    parent->current_number_of_keys--;
    free(sibling);
}

// Helper function to remove a key from the B+ tree
void removeFromTree(BPlusTree* tree, BPlusNode* node, int key) {
    int index = 0;
    while (index < node->current_number_of_keys && key > node->keys[index]) {
        index++;
    }

    if (node->is_leaf) {
        if (index < node->current_number_of_keys && key == node->keys[index]) {
            removeFromLeaf(node, key);
        }
    } else {
        if (index < node->current_number_of_keys && key == node->keys[index]) {
            removeFromNonLeaf(tree, node, key, index);
        } else {
            BPlusNode* child = node->children[index];
            if (child->current_number_of_keys >= (ORDER + 1) / 2) {
                removeFromTree(tree, child, key);
            } else {
                if (index != 0 && node->children[index - 1]->current_number_of_keys >= (ORDER + 1) / 2) {
                    borrowFromPrev(node, index);
                } else if (index != node->current_number_of_keys && node->children[index + 1]->current_number_of_keys >= (ORDER + 1) / 2) {
                    borrowFromNext(node, index);
                } else {
                    if (index == node->current_number_of_keys) {
                        index--;
                    }
                    mergeNodes(tree, node, child, node->children[index + 1], index);
                    removeFromTree(tree, child, key);
                }
            }
        }
    }
}

// Helper function to borrow a key from the previous sibling
void borrowFromPrev(BPlusNode* node, int index) {
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = node->children[index - 1];

    for (int i = child->current_number_of_keys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }

    if (!child->is_leaf) {
        for (int i = child->current_number_of_keys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    child->keys[0] = node->keys[index - 1];

    if (!child->is_leaf) {
        child->children[0] = sibling->children[sibling->current_number_of_keys];
    }

    node->keys[index - 1] = sibling->keys[sibling->current_number_of_keys - 1];
    child->current_number_of_keys++;
    sibling->current_number_of_keys--;
}

// Helper function to borrow a key from the next sibling
void borrowFromNext(BPlusNode* node, int index) {
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = node->children[index + 1];

    child->keys[child->current_number_of_keys] = node->keys[index];

    if (!child->is_leaf) {
        child->children[child->current_number_of_keys + 1] = sibling->children[0];
    }

    node->keys[index] = sibling->keys[0];

    for (int i = 1; i < sibling->current_number_of_keys; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    if (!sibling->is_leaf) {
        for (int i = 1; i <= sibling->current_number_of_keys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->current_number_of_keys++;
    sibling->current_number_of_keys--;
}

// Delete a key from the B+ tree
void deleteKey(BPlusTree* tree, int key) {
    BPlusNode* root = tree->root;

    if (!root) {
        return; // Tree is empty
    }

    removeFromTree(tree, root, key);

    // If the root has no keys left, update the root
    if (root->current_number_of_keys == 0) {
        BPlusNode* newRoot = root->is_leaf ? NULL : root->children[0];
        free(root);
        tree->root = newRoot;
    }
}

// Search for a key in the B+ tree
BPlusNode * search(BPlusTree* tree, int key) {
    return tree_search(tree->root, key);
}

BPlusNode * tree_search(BPlusNode * node, int K) {
    if (!node) {
        return NULL;
    }
    int i = 0;
    while (i < node->current_number_of_keys && K > node->keys[i]) {
        i++;
    }
    if (i < node->current_number_of_keys && K == node->keys[i]) {
        return node;
    }
    if (node->is_leaf) {
        return NULL;
    }
    return tree_search(node->children[i], K);
}

// Display the B+ tree (for debugging)
void displayTree(BPlusNode* node) {
    if (node) {
        for (int i = 0; i < node->current_number_of_keys; i++) {
            printf("%d ", node->keys[i]);
        }
        printf("| ");
    }
}

// In-order traversal of the B+ tree
void inOrderTraversal(BPlusNode* node) {
    if (node) {
        if (node->is_leaf) {
            displayTree(node);
        } else {
            for (int i = 0; i < node->current_number_of_keys; i++) {
                inOrderTraversal(node->children[i]);
                printf("%d ", node->keys[i]);
            }
            inOrderTraversal(node->children[node->current_number_of_keys]);
        }
    }
}

int main() {
    BPlusTree* tree = initializeTree();

    int keys[] = {3, 7, 1, 5, 9, 4, 2, 8, 6};
    int num_keys = sizeof(keys) / sizeof(keys[0]);

    // Insert keys into the B+ tree
    for (int i = 0; i < num_keys; i++) {
        insert(tree, keys[i]);
    }

    printf("In-order traversal of the B+ tree: ");
    inOrderTraversal(tree->root);
    printf("\n");

    int search_key = 12;
    BPlusNode* result = search(tree, search_key);
    if (result != NULL) {
        printf("%d found in the B+ tree.\n", search_key);
    } else {
        printf("%d not found in the B+ tree.\n", search_key);
    }

    search_key = 4;
    result = search(tree, search_key);
    if (result != NULL) {
        printf("%d found in the B+ tree.\n", search_key);
    } else {
        printf("%d not found in the B+ tree.\n", search_key);
    }

    int delete_key = 4;
    deleteKey(tree, delete_key);

    search_key = 4;
    result = search(tree, search_key);
    if (result != NULL) {
        printf("%d found in the B+ tree.\n", search_key);
    } else {
        printf("%d not found in the B+ tree.\n", search_key);
    }

    // Clean up and free memory
    // ...

    return 0;
}
