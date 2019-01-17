//
// Created by xdnzx on 2019/1/16.
//

#ifndef SZ_LIST_H
#define SZ_LIST_H

#include <malloc.h>
#include "pthread.h"

struct ListNode{
    struct ListNode* next;
    void* ptr;
};

struct List{
    struct ListNode* head;
    int count;
};

void ListInit(struct List* list){
    list->count = 0;
}

void ListDestroy(struct List* list){
    struct ListNode* ptr = list->head;
    struct ListNode* next;
    for(int i=0; i<list->count; i++){
        next = ptr->next;
        free(ptr);
    }
}

void* ListGet(struct List* list){
    struct ListNode* p;
    void* result;

    //get head node from list
    p = list->head;
    list->head = p->next;
    list->count--;
    result = p->ptr;
    free(p);

    return result;
}

void ListPut(struct List* list, void* ptr){
    struct ListNode* node = (struct ListNode*)malloc(sizeof(struct ListNode));

    //setting
    node->ptr = ptr;
    node->next = list->head;

    //add node to list;
    list->head = node;
    list->count++;
}

#endif //SZ_LIST_H
