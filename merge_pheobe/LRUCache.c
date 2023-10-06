#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "LRUCache.h"

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
void moveToEnd(struct LRUCache* cache, struct Node* node)
 {
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