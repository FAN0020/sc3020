#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"
#include "b+tree/bt_page.h"
#include "b+tree/bt_node.h"
#include "b+tree/bt_mgr.h"

#define LEAF_NODE_TYPE 2

// Function declarations for the functions defined in common.c
int countNodes(double nodePtr, BTPage *page);
int countLevels(double nodePtr, BTPage *page);

#endif // COMMON_H
