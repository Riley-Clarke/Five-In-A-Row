/* File name:   LinkedList.c
   Descirption: the implementation of LinkedList
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinkedList.h"

List *ListCreate() {
   List *list = (List*) malloc(sizeof(List));
   list->first = NULL;
   list->last = NULL;
   list->size = 0;

   list->cursor = NULL;
   return list;
}

//NOTE: this function may cause memory leaks
//      if the item in a node is linked to any data on the heap
//      the data would not freed by this function.
void ListDestroy(List *this) {
   Node *node = this->first;
   while (node != NULL) {
      printf("item@%p, node@%p to be freed\n", node->item, node);
      free(node->item);
      free(node);
      node = node->next;
   }
   free(this);
}


Node *NodeCreate(void *item) {
   Node *node = (Node*) malloc(sizeof(Node));
   node->item = item;
   node->next = NULL;

   return node;
}

void ListInsert(List *this, void* item) {
   Node *node = NodeCreate(item);

   node->next = this->first;
   this->first = node;
   if (this->size == 0) {
      this->last = node;
   }
   this->size++;
}

void ListFirst(List *this) {
   this->cursor = this->first;
}

void *ListGetNext(List *this) {
   void *result = NULL;
   if (this->cursor != NULL) {
      result = this->cursor->item;
      this->cursor = this->cursor->next;
   }
   return result;
}

void ListAppend(List *this, void* item){
    if(this->first!=NULL){
        Node *temp=NodeCreate(item);
        this->last->next= temp;
        this->last=temp;
        this->size++;
    }else{
        Node *temp=NodeCreate(item);
        this->first=temp;
        this->last=temp;
        this->size++;
    }

}

void *ListDelete(List *this){
    Node *temp=this->first;
    if(temp==NULL){
        return NULL;
    }
    free(this->first->item);
    free(this->first);
    this->first=temp->next;
    this->size--;
    return temp->item;
}

void *ListGetAt(List *this, int index){
    Node *temp=this->first;
    int count=0;
    while(temp!=NULL){
        if(count==index){
            return temp->item;
        }
        temp=temp->next;
        count++;
    }
    return NULL;
}
