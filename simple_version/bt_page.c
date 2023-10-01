#include "bt_page.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * function to create a new page
 * @return the page that is created. 
*/
BTPage* createPage(){
    BTPage* page = (BTPage*)malloc(sizeof(BTPage));
    // set all types to 0, signify empty slot, there is no node.
    memset(page->types,0,sizeof(page->types));
    page->next = NULL; 
    return page; 
}

/**
 * function to delete a page, used for deleting empty pages. 
 * @param page parent of the page to be deleted or the head of the pages 
*/
void deletePage(BTPage *page){
    if(page->next == NULL){
        BTPage *emptyPage = page; 
        page = NULL; 
        free(emptyPage);
    }
    else{
        BTPage* emptyPage = page->next;
        page->next = emptyPage->next;
        free(emptyPage);
    }
}

/**
 * function to insert a page record, recording a new BT node
 * @param page current page
 * @param pointer pointer reference of the newly added BT node
 * @param type int value representing the type of the node. 1) non-leaf, 2) leaf, 3) overflow  
*/
void insertPageRecord(BTPage *page, double pointer, int type){
    // search for empty slot then insert. 
    for(int i = 0; i<P_REC_COUNT;i++){
        if (page->types[i] != 0){
            page->types[i] = type; 
            page->pointers[i] = pointer; 
            return;
        }
    }
    // no empty slot found, move to next page. 
    // no next page, create a new page
    if (page->next == NULL){
        BTPage* newPage = createPage();
        page->next = newPage;
    }
    // attempt to insert in the next page
    insertPageRecord(page->next,pointer,type);
}


/**
 * function to delete a page record, to delete the record of a BT node
 * @param page current page
 * @param pointer the reference of the BT node to be removed from records. 
 * @return returns an int value indicating if the page is empty.
*/
int deletePageRecord(BTPage *page, double pointer){
    //exit case: 
    if (page==NULL) return 0; 

    int recordCount = 0;
    int i; 
    // finding the record containing the BT node, free the slot. 
    for (i = 0; i<P_REC_COUNT;i++){
        if(page->pointers[i] == pointer & page->types[i]!=0){
            page->types[i]=0;
            // check if page is empty. empty page = all type 0 = total count 0
            for(i=0;i<P_REC_COUNT;i++){
                recordCount += page->types[i];
            }
            // if page is empty, return 1, else 0. 
            if (recordCount == 0) return 1;
            else return 0; 
        }
    }
    // if next page is empty, delete page. 
    if(deletePageRecord(page->next, pointer)){
        deletePage(page);
    }
    return 0; 
}

/**
 * Function to search for node type in page.
 * @param page current page to search for node. 
 * @param pointer pointer of the node we're searching for. 
 * @return returns the type of the node. 
*/
int searchPageRecord(BTPage *page, double pointer){
    //exit case: no page, nothing found, return 0
    if(page == NULL){
        return 0; 
    }
    for(int i = 0; i<P_REC_COUNT;i++){
        // node found, return node type
        if(page->pointers[i]==pointer & page->types!=0){
            return page->types[i];
        }
    }
    // not found in current page, search other pages
    return searchPageRecord(page->next,pointer);
}
