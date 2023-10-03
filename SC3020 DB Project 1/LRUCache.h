#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "block.h"

struct Node {
    int key;
    struct Block value;
    struct Node* next;
    struct Node* prev;
};

struct LinkedList {
    struct Node* head;
    struct Node* tail;
};

struct LRUCache {
    int capacity;
    struct LinkedList* list;
    struct Node** cacheMap;
};

// Function prototypes
struct LRUCache* createLRUCache(int capacity);
void freeLRUCache(struct LRUCache* cache);
struct Block* get(struct LRUCache* cache, int key);
void put(struct LRUCache* cache, int key, struct Block value);

#endif /* LRUCACHE_H */
