//
// Created by borelset on 2019/1/16.
//

#ifndef SZ_CONCURRENTCONTROLLER_H
#define SZ_CONCURRENTCONTROLLER_H

#include "List.h"

struct ConcurrentController{
    int index;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    volatile int pos;
};

void ConcurrentControllerInit(struct ConcurrentController* concurrentController){
    pthread_mutex_init(&concurrentController->mutex, NULL);
    pthread_cond_init(&concurrentController->cond, NULL);
}

void ConcurrentControllerDestroy(struct ConcurrentController* concurrentController){
    pthread_mutex_destroy(&concurrentController->mutex);
    pthread_cond_destroy(&concurrentController->cond);
}

struct CCSourcePool{
    struct List idle;
    int counter;
};

void CCSourcePoolInit(struct CCSourcePool* ccSourcePool){
    ListInit(&ccSourcePool->idle);
    ccSourcePool->counter = 0;
}


void CCSourcePoolDestroy(struct CCSourcePool* ccSourcePool){
    struct ConcurrentController* concurrentController;
    for(int i=0; i<ccSourcePool->idle.count; i++){
        concurrentController = (struct ConcurrentController*)ListGet(&ccSourcePool->idle);
        ConcurrentControllerDestroy(concurrentController);
        free(concurrentController);
    }
    ListDestroy(&ccSourcePool->idle);
}

struct ConcurrentController* CCSourcePoolGet(struct CCSourcePool* ccSourcePool){
    struct ConcurrentController* ptr;
    if(ccSourcePool->idle.count){
        ptr = (struct ConcurrentController*)ListGet(&ccSourcePool->idle);
    }else{
        ptr = (struct ConcurrentController*)malloc(sizeof(struct ConcurrentController));
        ptr->index = ccSourcePool->counter++;
        ConcurrentControllerInit(ptr);
    }
    ptr->pos = 0;
    return ptr;
}

void CCSourcePoolAdd(struct CCSourcePool* ccSourcePool, struct ConcurrentController* concurrentController){
    ListPut(&ccSourcePool->idle, concurrentController);
}

#endif //SZ_CONCURRENTCONTROLLER_H
