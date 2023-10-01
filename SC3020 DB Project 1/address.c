#include <stdio.h>

// To find the address of the block where the record is stored. 

struct Address {
    int blockID; 
    int offset;  
};

struct Address createAddress(int blockID, int offset) {
    struct Address address;
    address.blockID = blockID;
    address.offset = offset;
    return address;
}

int getBlockId(struct Address address) {
    return address.blockID;
}

int getOffset(struct Address address) {
    return address.offset;
}

void toString(struct Address address) {
    printf("blk %d offset %d\n", address.blockID, address.offset);
}

int main() {
    struct Address address = createAddress(1, 42);
    printf("Block ID: %d\n", getBlockId(address));
    printf("Offset: %d\n", getOffset(address));
    toString(address);
    return 0;
}
