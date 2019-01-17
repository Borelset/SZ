//
// Created by borelset on 2019/1/16.
//
#ifndef SZ_TASKDISPATCHER_H
#define SZ_TASKDISPATCHER_H

#include "ConcurrentController.h"
#include "LineCache.h"
#include "List.h"
#include "DynamicIntArray.h"
#include "DynamicByteArray.h"
#include "sz.h"

struct Task{
    int lineIndex;
    int lineLength;
    struct ConcurrentController* concurrentController;
    struct LineCache* lineCache;
    struct ConcurrentController* prevConcurrentController;
    struct LineCache* prevLineCache;

    DynamicIntArray *exactLeadNumArray;
    DynamicByteArray *exactMidByteArray;
    DynamicIntArray *resiBitArray;
};

struct RegisterTableEntry{
    int lineIndex;
    struct ConcurrentController* concurrentController;
    struct LineCache* lineCache;
};

struct SZParams{
    double realPrecision;
    int intvRadius;
    float medianValue;
    int reqLength;
    int reqBytesLength;
    int resiBitsLength;
};

struct TaskDispatcher{
    int taskCount;
    int lastest;
    int lineLength;
    pthread_mutex_t mutex;
    struct RegisterTableEntry* registerTable;
    struct CCSourcePool ccSourcePool;
    struct LCSourcePool lcSourcePool;
    int* typeArray;
    float* floatArray;
    struct SZParams szParams;
    DynamicIntArray** leadArray;
    DynamicByteArray** byteArray;
    DynamicIntArray** resiArray;
};

void TaskDispatcherInit(struct TaskDispatcher* taskDispatcher, int count, float* firstLine, int lineLength, int * typeArray, float *floatArray, struct SZParams szParams){
    pthread_mutex_init(&taskDispatcher->mutex, NULL);
    LCSourcePoolInit(&taskDispatcher->lcSourcePool, lineLength);
    CCSourcePoolInit(&taskDispatcher->ccSourcePool);
    taskDispatcher->lastest = 1;
    taskDispatcher->lineLength = lineLength;
    taskDispatcher->taskCount = count;
    taskDispatcher->registerTable = (struct RegisterTableEntry*)malloc(count * sizeof(struct TaskDispatcher));

    taskDispatcher->typeArray = typeArray;
    taskDispatcher->floatArray = floatArray;
    taskDispatcher->szParams = szParams;

    taskDispatcher->leadArray = (DynamicIntArray**)malloc(sizeof(DynamicIntArray*) * (count-1));
    taskDispatcher->byteArray = (DynamicByteArray**)malloc(sizeof(DynamicByteArray*) * (count-1));
    taskDispatcher->resiArray = (DynamicIntArray**)malloc(sizeof(DynamicIntArray*) * (count-1));
    for(int i=0; i<count-1; i++){
        new_DIA(&taskDispatcher->leadArray[i], 1024);
        new_DBA(&taskDispatcher->byteArray[i], 1024);
        new_DIA(&taskDispatcher->resiArray[i], 1024);
    }

    taskDispatcher->registerTable[0].lineCache = LCSourcePoolGet(&taskDispatcher->lcSourcePool);
    memcpy(taskDispatcher->registerTable[0].lineCache->readCache, firstLine, sizeof(float) * taskDispatcher->lineLength);
    taskDispatcher->registerTable[0].concurrentController = CCSourcePoolGet(&taskDispatcher->ccSourcePool);
    taskDispatcher->registerTable[0].concurrentController->pos = lineLength*2;
}

void TaskDispatcherDestroy(struct TaskDispatcher* taskDispatcher){
    for(int i=0; i<taskDispatcher->taskCount-1; i++){
        free_DIA(taskDispatcher->leadArray[i]);
        free_DBA(taskDispatcher->byteArray[i]);
        free(taskDispatcher->resiArray[i]);
    }
    free(taskDispatcher->leadArray);
    free(taskDispatcher->byteArray);
    free(taskDispatcher->resiArray);
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
    result.lineCache->typeCache = &taskDispatcher->typeArray[ taskDispatcher->lineLength * result.lineIndex ];
    result.lineCache->dataCache = &taskDispatcher->floatArray[ taskDispatcher->lineLength * result.lineIndex ];
    result.lineLength = taskDispatcher->lineLength;

    taskDispatcher->registerTable[ result.lineIndex ].concurrentController = result.concurrentController;
    taskDispatcher->registerTable[ result.lineIndex ].lineCache = result.lineCache;

    result.exactLeadNumArray = taskDispatcher->leadArray[ result.lineIndex-1 ];
    result.exactMidByteArray = taskDispatcher->byteArray[ result.lineIndex-1 ];
    result.resiBitArray = taskDispatcher->resiArray[ result.lineIndex-1 ];

    pthread_mutex_unlock(&taskDispatcher->mutex);

    return result;
}

void TaskDispatcherFinish(struct TaskDispatcher* taskDispatcher, struct Task* task){
    pthread_mutex_lock(&taskDispatcher->mutex);

    CCSourcePoolAdd(&taskDispatcher->ccSourcePool, task->prevConcurrentController);
    LCSourcePoolAdd(&taskDispatcher->lcSourcePool, task->prevLineCache);

    pthread_mutex_unlock(&taskDispatcher->mutex);
}





#endif //SZ_TASKDISPATCHER_H
