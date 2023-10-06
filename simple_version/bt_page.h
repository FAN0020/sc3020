#ifndef BT_PAGE_H
#define BT_PAGE_H

// defining fixed variables
#define NODE_SIZE 400
#define P_REC_COUNT (NODE_SIZE-8)/(4+8)


/**
 * Object containing a list of BT blocks and its type.
 * var types type of the bt node. 0) nothing 1) non-leaf, 2) leaf, 3) overflow
 * var pointers corresponding pointer referencing the bt node. 
*/
typedef struct _btpage{
    int types[P_REC_COUNT];
    double pointers[P_REC_COUNT];
    struct _btpage *next; 
} BTPage; 

// declare functions
BTPage* createPage(); // function to create a page
void deletePage(BTPage *page); //delete page.
void insertPageRecord(BTPage *page, double pointer, int type); // function to add a new node to the page
int deletePageRecord(BTPage *page, double pointer); // function to remove a page record. 
int searchPageRecord(BTPage *page, double pointer); // look for BT node within the pages, and return the type of node. 


#endif //BT_PAGE_H