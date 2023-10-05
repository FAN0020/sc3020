#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
// #include "block.h"
#include "LRUCache.h"

// struct Node {
//     int key;
//     struct Block* value;
//     struct Node* next;
//     struct Node* prev;
// };

// struct LinkedList {
//     struct Node* head;
//     struct Node* tail;
// };

// struct LRUCache {
//     int capacity;
//     struct LinkedList* list;
//     struct Node** cacheMap;
// };

struct Node* createNode(int key, struct Block* value) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->key = key;
    node->value = value;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

struct LinkedList* createLinkedList() {
    struct LinkedList* list = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

struct LRUCache* createLRUCache(int capacity) {
    struct LRUCache* cache = (struct LRUCache*)malloc(sizeof(struct LRUCache));
    cache->capacity = capacity;
    cache->list = createLinkedList();
    cache->cacheMap = (struct Node**)malloc(sizeof(struct Node*) * capacity);
    for (int i = 0; i < capacity; i++) {
        cache->cacheMap[i] = NULL;
    }
    return cache;
}

// move a node to the end of the linked list
void moveToEnd(struct LRUCache* cache, struct Node* node) {
    if (node == cache->list->tail) {
        return; // Node is already at the end
    }

    if (node == cache->list->head) {
        cache->list->head = node->next;
        cache->list->head->prev = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    node->prev = cache->list->tail;
    node->next = NULL;
    cache->list->tail->next = node;
    cache->list->tail = node;
}

struct Block* get(struct LRUCache* cache, int key) {
    struct Node* node = cache->cacheMap[key % cache->capacity];
    while (node != NULL) {
        if (node->key == key) {
            // Move the accessed node to the end
            moveToEnd(cache, node);
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

void put(struct LRUCache* cache, int key, struct Block* value) {
    struct Node* newNode = createNode(key, value);
    if (cache->list->head == NULL) {
        cache->list->head = newNode;
        cache->list->tail = newNode;
    } else {
        newNode->prev = cache->list->tail;
        cache->list->tail->next = newNode;
        cache->list->tail = newNode;
    }

    cache->cacheMap[key % cache->capacity] = newNode;

    if (cache->capacity <= cache->list->tail->key) {
        // If the cache is full, remove the least recently used node
        struct Node* toRemove = cache->list->head;
        cache->list->head = toRemove->next;
        if (cache->list->head != NULL) {
            cache->list->head->prev = NULL;
        }
        cache->cacheMap[toRemove->key % cache->capacity] = NULL;
        free(toRemove);
    }
}

void freeLRUCache(struct LRUCache* cache) {
    struct Node* current = cache->list->head;
    while (current != NULL) {
        struct Node* next = current->next;
        free(current);
        current = next;
    }
    free(cache->list);
    free(cache->cacheMap);
    free(cache);
}

/*int main() {
    struct LRUCache* cache = createLRUCache(3);

    struct Block block1;
    block1.blockID = 1;

    struct Block block2;
    block2.blockID = 2;

    struct Block block3;
    block3.blockID = 3;

    struct Block block4;
    block4.blockID = 4;

    put(cache, 1, block1);
    put(cache, 2, block2);
    put(cache, 3, block3);
    put(cache, 4, block4); 

    struct Block* result = get(cache, 1); 

    if (result == NULL) {
        printf("Block 1 is not in the cache.\n");
    }

    result = get(cache, 4);

    if (result != NULL) {
        printf("Block 4 is in the cache. Block ID: %d\n", result->blockID);
    }

    freeLRUCache(cache);

    return 0;
}
*/