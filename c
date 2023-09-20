#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants for block size and disk capacity
#define BLOCK_SIZE 400
#define DISK_CAPACITY_MB 100

// Data structure for the disk storage
typedef struct DiskStorage {
    char** blocks;  // Array to store blocks
    int totalBlocks;  // Total number of blocks in the disk
} DiskStorage;

static DiskStorage* disk = NULL;  // Global variable to simulate disk storage

/************************************************************
 *                    handle data structures                *
 ************************************************************/
typedef struct SM_FileHandle {
    char* fileName;
    int totalNumPages;
    int curPagePos;
    void* mgmtInfo;
} SM_FileHandle;

typedef char* SM_PageHandle;

/************************************************************
 *                    interface                             *
 ************************************************************/
/* Function to initialize the storage manager */
extern void initStorageManager(void) {
    if (disk == NULL) {
        // Initialize disk storage
        disk = (DiskStorage*)malloc(sizeof(DiskStorage));
        disk->totalBlocks = DISK_CAPACITY_MB * (1024 * 1024) / BLOCK_SIZE; // Calculate total blocks
        disk->blocks = (char**)malloc(disk->totalBlocks * sizeof(char*));

        // Initialize each block
        for (int i = 0; i < disk->totalBlocks; i++) {
            disk->blocks[i] = (char*)malloc(BLOCK_SIZE);
            memset(disk->blocks[i], '\0', BLOCK_SIZE); // Initialize with null bytes
        }
    }
}

/* Function to create a new page file */
extern RC createPageFile(char* fileName) {
    FILE* fp;

    // Check if the file already exists
    if (access(fileName, F_OK) != -1) {
        return RC_FILE_ALREADY_EXISTS;
    }

    // Create the file
    fp = fopen(fileName, "wb");
    if (fp == NULL) {
        return RC_FILE_CREATION_FAILED;
    }

    // Write an empty page to the file
    char* emptyPage = (char*)malloc(BLOCK_SIZE);
    memset(emptyPage, '\0', BLOCK_SIZE);
    fwrite(emptyPage, 1, BLOCK_SIZE, fp);
    fclose(fp);

    // Free the empty page buffer
    free(emptyPage);

    return RC_OK;
}

/* Function to open a page file */
extern RC openPageFile(char* fileName, SM_FileHandle* fHandle) {
    // Check if the file exists
    if (access(fileName, F_OK) == -1) {
        return RC_FILE_NOT_FOUND;
    }

    // Initialize file handle
    fHandle->fileName = strdup(fileName);
    fHandle->totalNumPages = DISK_CAPACITY_MB * (1024 * 1024) / BLOCK_SIZE;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = NULL; // We don't need to store additional information for this simulation

    return RC_OK;
}

/* Function to close a page file */
extern RC closePageFile(SM_FileHandle* fHandle) {
    // Free resources and reset file handle
    if (fHandle->fileName != NULL) {
        free(fHandle->fileName);
    }
    fHandle->fileName = NULL;
    fHandle->totalNumPages = 0;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = NULL;

    return RC_OK;
}

/* Function to destroy a page file */
extern RC destroyPageFile(char* fileName) {
    // Check if the file exists
    if (access(fileName, F_OK) == -1) {
        return RC_FILE_NOT_FOUND;
    }

    // Delete the file
    if (remove(fileName) != 0) {
        return RC_DELETE_FAILED;
    }

    return RC_OK;
}

/* Function to read a block from disk */
extern RC readBlock(int pageNum, SM_FileHandle* fHandle, SM_PageHandle memPage) {
    // Check if the page number is valid
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Copy the block data from disk to memory
    memcpy(memPage, disk->blocks[pageNum], BLOCK_SIZE);
    fHandle->curPagePos = pageNum;

    return RC_OK;
}

/* Function to get the current block position */
extern int getBlockPos(SM_FileHandle* fHandle) {
    return fHandle->curPagePos;
}

/* Function to read the first block */
extern RC readFirstBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

/* Function to read the previous block */
extern RC readPreviousBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int prevPage = fHandle->curPagePos - 1;
    if (prevPage < 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(prevPage, fHandle, memPage);
}

/* Function to read the current block */
extern RC readCurrentBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

/* Function to read the next block */
extern RC readNextBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int nextPage = fHandle->curPagePos + 1;
    if (nextPage >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(nextPage, fHandle, memPage);
}

/* Function to read the last block */
extern RC readLastBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

/* Function to write a block to disk */
extern RC writeBlock(int pageNum, SM_FileHandle* fHandle, SM_PageHandle memPage) {
    // Check if the page number is valid
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_WRITE_OUT_OF_BOUND;
    }

    // Copy the block data from memory to disk
    memcpy(disk->blocks[pageNum], memPage, BLOCK_SIZE);
    fHandle->curPagePos = pageNum;

    return RC_OK;
}

/* Function to write the current block to disk */
extern RC writeCurrentBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

/* Function to append an empty block to the end of the file */
extern RC appendEmptyBlock(SM_FileHandle* fHandle) {
    // Check if there is enough space to append a block
    if (fHandle->totalNumPages >= disk->totalBlocks) {
        return RC_DISK_FULL;
    }

    // Initialize the new block with null bytes
    char* emptyBlock = (char*)malloc(BLOCK_SIZE);
    memset(emptyBlock, '\0', BLOCK_SIZE);

    // Add the new block to the end of the file
    int newPageNum = fHandle->totalNumPages;
    writeBlock(newPageNum, fHandle, emptyBlock);

    // Update file handle information
    fHandle->totalNumPages++;
    fHandle->curPagePos = newPageNum;

    // Free the empty block buffer
    free(emptyBlock);

    return RC_OK;
}

/* Function to ensure the capacity of the file */
extern RC ensureCapacity(int numberOfPages, SM_FileHandle* fHandle) {
    // Calculate the number of additional pages needed
    int additionalPages = numberOfPages - fHandle->totalNumPages;

    // Check if additional pages are needed
    if (additionalPages <= 0) {
        return RC_OK;
    }

    // Append empty blocks to meet the required capacity
    for (int i = 0; i < additionalPages; i++) {
        RC result = appendEmptyBlock(fHandle);
        if (result != RC_OK) {
            return result;
        }
    }

    return RC_OK;
}

#define FILE_NAME "test_pagefile.bin"
void initStorageManager(void) {
    initStorageManager(); // Call the initialization function to set up disk storage.
}
