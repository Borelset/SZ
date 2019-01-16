//
// Created by borelset on 2019/1/16.
//
#ifndef SZ_TASKDISPATCHER_H
#define SZ_TASKDISPATCHER_H

#include "ConcurrentController.h"
#include "LineCache.h"
#include "List.h"

struct Task{
    int lineIndex;
    int lineLength;
    struct ConcurrentController* concurrentController;
    struct LineCache* lineCache;
    struct ConcurrentController* prevConcurrentController;
    struct LineCache* prevLineCache;
};

struct RegisterTableEntry{
    int lineIndex;
    struct ConcurrentController* concurrentController;
    struct LineCache* lineCache;
};

struct TaskDispatcher{
    int taskCount;
    int lastest;
    int lineLength;
    pthread_mutex_t mutex;
    struct RegisterTableEntry* registerTable;
    struct CCSourcePool ccSourcePool;
    struct LCSourcePool lcSourcePool;
};

void TaskDispatcherInit(struct TaskDispatcher* taskDispatcher, int count, struct LineCache* firstLineCache, int lineLength){
    pthread_mutex_init(&taskDispatcher->mutex, NULL);
    taskDispatcher->lastest = 1;
    taskDispatcher->lineLength = lineLength;
    taskDispatcher->taskCount = count;
    taskDispatcher->registerTable = (struct RegisterTableEntry*)malloc(count * sizeof(struct TaskDispatcher));
    taskDispatcher->registerTable[0].lineCache = LCSourcePoolGet(&taskDispatcher->lcSourcePool);
    taskDispatcher->registerTable[0].concurrentController = CCSourcePoolGet(&taskDispatcher->ccSourcePool);
    taskDispatcher->registerTable[0].concurrentController->pos = lineLength;
}

struct Task TaskDispatcherGet(struct TaskDispatcher* taskDispatcher){
    struct Task result;
    pthread_mutex_lock(&taskDispatcher->mutex);

    if(taskDispatcher->lastest >= taskDispatcher->taskCount){
        result.lineIndex = 0;
        pthread_mutex_unlock(&taskDispatcher->mutex);
        return result;
    }

    result.lineIndex = taskDispatcher->lastest;
    taskDispatcher->lastest++;

    result.prevConcurrentController = taskDispatcher->registerTable[ result.lineIndex-1 ].concurrentController;
    result.prevLineCache = taskDispatcher->registerTable[ result.lineIndex-1 ].lineCache;
    result.concurrentController = CCSourcePoolGet(&taskDispatcher->ccSourcePool);
    result.lineCache = LCSourcePoolGet(&taskDispatcher->lcSourcePool);
    result.lineLength = taskDispatcher->lineLength;

    taskDispatcher->registerTable[ result.lineIndex ].concurrentController = result.concurrentController;
    taskDispatcher->registerTable[ result.lineIndex ].lineCache = result.lineCache;

    pthread_mutex_unlock(&taskDispatcher->mutex);

    return result;
}

void TaskDispatcherFinish(struct TaskDispatcher* taskDispatcher, Task* task){
    pthread_mutex_lock(&taskDispatcher->mutex);

    CCSourcePoolAdd(&taskDispatcher->ccSourcePool, task->prevConcurrentController);
    LCSourcePoolAdd(&taskDispatcher->lcSourcePool, task->prevLineCache);

    pthread_mutex_unlock(&taskDispatcher->mutex);
}





#endif //SZ_TASKDISPATCHER_H
