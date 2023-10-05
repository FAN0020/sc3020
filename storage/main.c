#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "block.h"
#include "LRUCache.h"

#include "disk.c"
#include "block.c"
#include "LRUCache.c"

int main() {
    struct Disk* disk = createDisk(500 * 1024 * 1024, MAX_RECORDS, 100); // Example values, you can modify as per your requirements.
    load_data_to_disk(disk, "/Users/fanyupei/Codes/sc3020/game.txt");
    // Add other functionalities or clean-up as needed...
    return 0;
}

