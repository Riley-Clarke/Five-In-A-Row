/* File name:   LinkedList.h
   Description: the header file for LinkedList.
                #include "LinkedList.h" in your program.
*/

// define the struct of nodes
struct NODE {
   void *item;
   struct NODE *next;
};
// give "struct NODE" a short name (Node)
typedef struct NODE  Node;

// define a struct and give it a short name
typedef struct LIST {
   Node *first;
   Node *last;
   int size;

   Node *cursor;
} List;

List *ListCreate();

void ListDestroy(List *this);

Node *NodeCreate(void *item);

void ListInsert(List *this, void* item);

void ListFirst(List *this);

void *ListGetNext(List *this);

// addis item to the end of the list
void ListAppend(List *this, void* item);

// deletes the first node if exists & returns its item
// if list is empty, returns NULL
void *ListDelete(List *this);

// returns the item at index if exists, otherwise NULL;
// the first item in the list has the index of 0.
void *ListGetAt(List *this, int index);
