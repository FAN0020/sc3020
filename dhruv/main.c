#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define ORDER 3 // Adjust the order of the B+ tree as needed
// Define B+ tree node structure
typedef struct BPlusNode {
    int keys[ORDER - 1];
    struct BPlusNode* children[ORDER];
    bool is_leaf;
    int num_keys;
    struct BPlusNode* next; // Pointer to next leaf node (used for range queries)
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
bool search(BPlusTree* tree, int key);
void displayTree(BPlusNode* node);
void inOrderTraversal(BPlusNode* node);




// Create a new B+ tree node
BPlusNode* createNode() {
    BPlusNode* newNode = (BPlusNode*)malloc(sizeof(BPlusNode));
    newNode->is_leaf = true;
    newNode->num_keys = 0;
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
    int i = node->num_keys - 1;
    while (i >= 0 && key < node->keys[i]) {
        node->keys[i + 1] = node->keys[i];
        i--;
    }
    node->keys[i + 1] = key;
    node->num_keys++;
}

// Helper function to split a node
void splitNode(BPlusTree* tree, BPlusNode* parent, BPlusNode* child, int index) {
    BPlusNode* newNode = createNode();
    newNode->is_leaf = child->is_leaf;
    newNode->next = child->next;
    child->next = newNode;

    int split_point = (ORDER - 1) / 2;
    for (int i = split_point + 1, j = 0; i < ORDER; i++, j++) {
        newNode->keys[j] = child->keys[i];
        child->keys[i] = 0;
        newNode->children[j] = child->children[i];
        child->children[i] = NULL;
        newNode->num_keys++;
        child->num_keys--;
    }

    insertInNode(parent, child->keys[split_point]);

    for (int i = index + 1; i < parent->num_keys; i++) {
        parent->children[i + 1] = parent->children[i];
    }

    parent->children[index + 1] = newNode;
}

// Insert a key into the B+ tree
void insert(BPlusTree* tree, int key) {
    BPlusNode* root = tree->root;

    if (root->num_keys == ORDER - 1) {
        BPlusNode* newNode = createNode();
        newNode->children[0] = root;
        splitNode(tree, newNode, root, 0);
        tree->root = newNode;
        insertInNode(newNode, key);
    } else {
        BPlusNode* current = root;
        while (!current->is_leaf) {
            int i = current->num_keys - 1;
            while (i >= 0 && key < current->keys[i]) {
                i--;
            }
            i++;
            if (current->children[i]->num_keys == ORDER - 1) {
                splitNode(tree, current, current->children[i], i);
                if (key > current->keys[i]) {
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
    while (i < node->num_keys && key != node->keys[i]) {
        i++;
    }
    if (i < node->num_keys) {
        for (int j = i + 1; j < node->num_keys; j++) {
            node->keys[j - 1] = node->keys[j];
        }
        node->num_keys--;
    }
}

// Helper function to remove a key from a non-leaf node
void removeFromNonLeaf(BPlusTree* tree, BPlusNode* node, int key, int index) {
    int k = node->keys[index];
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = (index == 0) ? node->children[index + 1] : node->children[index - 1];

    if (child->num_keys >= (ORDER + 1) / 2) {
        int pred = findMin(child);
        removeFromTree(tree, child, pred);
        node->keys[index] = pred;
    } else if (sibling->num_keys >= (ORDER + 1) / 2) {
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
    child->keys[child->num_keys] = merge_index;

    for (int i = 0, j = child->num_keys + 1; i < sibling->num_keys; i++, j++) {
        child->keys[j] = sibling->keys[i];
    }

    if (!child->is_leaf) {
        for (int i = 0, j = child->num_keys + 1; i <= sibling->num_keys; i++, j++) {
            child->children[j] = sibling->children[i];
        }
    }

    for (int i = index + 1; i < parent->num_keys; i++) {
        parent->keys[i - 1] = parent->keys[i];
        parent->children[i] = parent->children[i + 1];
    }

    child->num_keys += sibling->num_keys + 1;
    parent->num_keys--;
    free(sibling);
}

// Helper function to remove a key from the B+ tree
void removeFromTree(BPlusTree* tree, BPlusNode* node, int key) {
    int index = 0;
    while (index < node->num_keys && key > node->keys[index]) {
        index++;
    }

    if (node->is_leaf) {
        if (index < node->num_keys && key == node->keys[index]) {
            removeFromLeaf(node, key);
        }
    } else {
        if (index < node->num_keys && key == node->keys[index]) {
            removeFromNonLeaf(tree, node, key, index);
        } else {
            BPlusNode* child = node->children[index];
            if (child->num_keys >= (ORDER + 1) / 2) {
                removeFromTree(tree, child, key);
            } else {
                if (index != 0 && node->children[index - 1]->num_keys >= (ORDER + 1) / 2) {
                    borrowFromPrev(node, index);
                } else if (index != node->num_keys && node->children[index + 1]->num_keys >= (ORDER + 1) / 2) {
                    borrowFromNext(node, index);
                } else {
                    if (index == node->num_keys) {
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

    for (int i = child->num_keys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }

    if (!child->is_leaf) {
        for (int i = child->num_keys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    child->keys[0] = node->keys[index - 1];

    if (!child->is_leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
    }

    node->keys[index - 1] = sibling->keys[sibling->num_keys - 1];
    child->num_keys++;
    sibling->num_keys--;
}

// Helper function to borrow a key from the next sibling
void borrowFromNext(BPlusNode* node, int index) {
    BPlusNode* child = node->children[index];
    BPlusNode* sibling = node->children[index + 1];

    child->keys[child->num_keys] = node->keys[index];

    if (!child->is_leaf) {
        child->children[child->num_keys + 1] = sibling->children[0];
    }

    node->keys[index] = sibling->keys[0];

    for (int i = 1; i < sibling->num_keys; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    if (!sibling->is_leaf) {
        for (int i = 1; i <= sibling->num_keys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->num_keys++;
    sibling->num_keys--;
}

// Delete a key from the B+ tree
void deleteKey(BPlusTree* tree, int key) {
    BPlusNode* root = tree->root;

    if (!root) {
        return; // Tree is empty
    }

    removeFromTree(tree, root, key);

    // If the root has no keys left, update the root
    if (root->num_keys == 0) {
        BPlusNode* newRoot = root->is_leaf ? NULL : root->children[0];
        free(root);
        tree->root = newRoot;
    }
}

// Search for a key in the B+ tree
bool search(BPlusTree* tree, int key) {
    BPlusNode* root = tree->root;
    BPlusNode* current = root;

    while (current) {
        int i = 0;
        while (i < current->num_keys && key >= current->keys[i]) {
            if (key == current->keys[i]) {
                return true; // Key found
            }
            i++;
        }

        if (!current->is_leaf) {
            current = current->children[i];
        } else {
            break; // Key not found in leaf node
        }
    }

    return false; // Key not found
}

// Display the B+ tree (for debugging)
void displayTree(BPlusNode* node) {
    if (node) {
        for (int i = 0; i < node->num_keys; i++) {
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
            for (int i = 0; i < node->num_keys; i++) {
                inOrderTraversal(node->children[i]);
                printf("%d ", node->keys[i]);
            }
            inOrderTraversal(node->children[node->num_keys]);
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

    int search_key = 5;
    if (search(tree, search_key)) {
        printf("%d found in the B+ tree.\n", search_key);
    } else {
        printf("%d not found in the B+ tree.\n", search_key);
    }

    int delete_key = 4;
    deleteKey(tree, delete_key);
    printf("In-order traversal after deleting %d: ", delete_key);
    inOrderTraversal(tree->root);
    printf("\n");

    // Clean up and free memory
    // ...

    return 0;
}
