//
// Created by borelset on 2019/1/16.
//

#ifndef SZ_WORKER_H
#define SZ_WORKER_H

#include <stdio.h>
#include <syscall.h>
#include <zconf.h>
#include "TaskDispatcher.h"
#include "math.h"
#include "CompressElement.h"
#include "dataCompression.h"

#define CheckInterval 180

#define WorkerType2D 1
#define WorkerType3D 2

void* WorkerLoop1(void* args){
    struct TaskDispatcher* taskDispatcher = (struct TaskDispatcher*)args;
    struct Task task;
    long tid = syscall(SYS_gettid);
    //printf("Worker==> tid:%ld, start\n", tid);

    double realPrecision = taskDispatcher->szParams.realPrecision;
    int intvRadius = taskDispatcher->szParams.intvRadius;
    int intvCapacity = intvRadius*2;
    float medianValue = taskDispatcher->szParams.medianValue;
    int reqLength = taskDispatcher->szParams.reqLength;
    int reqBytesLength = taskDispatcher->szParams.reqBytesLength;
    int resiBitsLength = taskDispatcher->szParams.resiBitsLength;

    float pred, cur, diff;
    int type, itvNum;
    unsigned char preDataBytes[4];

    FloatValueCompressElement *vce = (FloatValueCompressElement*)malloc(sizeof(FloatValueCompressElement));
    LossyCompressionElement *lce = (LossyCompressionElement*)malloc(sizeof(LossyCompressionElement));

    while((task = TaskDispatcherGet(taskDispatcher), task.lineIndex)){
        struct LineCache* prevLineCache = task.prevLineCache;
        struct LineCache* lineCache = task.lineCache;
        struct ConcurrentController* prevConcurrentController = task.prevConcurrentController;
        struct ConcurrentController* concurrentController = task.concurrentController;
        //printf("Worker==> tid:%ld, get task:%d\n", tid, task.lineIndex);

        DynamicIntArray *exactLeadNumArray = task.exactLeadNumArray;
        DynamicByteArray *exactMidByteArray = task.exactMidByteArray;
        DynamicIntArray *resiBitArray = task.resiBitArray;

        for(int i=0; i<task.lineLength; i++){
            if(i % CheckInterval == 0){
                pthread_mutex_lock(&prevConcurrentController->mutex);
                if(i+CheckInterval > prevConcurrentController->pos){
                    //printf("Worker==> tid:%ld, check fail and wait for:%d, pos:%d, prev_pos:%d\n", tid, prevConcurrentController->index, i, prevConcurrentController->pos);
                    pthread_cond_wait(&prevConcurrentController->cond, &prevConcurrentController->mutex);
                    //printf("Worker==> tid:%ld, end wait, pos:%d\n", tid, i);
                }
                pthread_mutex_unlock(&prevConcurrentController->mutex);
            }
            if(i == 0){
                pred = prevLineCache->readCache[i];
                cur = lineCache->dataCache[i];
                diff = cur-pred;
                itvNum = fabs(diff)/realPrecision + 1;
                if (itvNum < intvCapacity)
                {
                    if (diff < 0) itvNum = -itvNum;
                    type = itvNum/2;
                    lineCache->readCache[i] = pred + realPrecision * 2 * type;
                    lineCache->typeCache[i] = type + intvRadius;
                }else{
                    lineCache->typeCache[i] = 0;
                    compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength, resiBitsLength);
                    updateLossyCompElement_Float_first(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                    memcpy(preDataBytes,vce->curBytes,4);
                    addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                    lineCache->readCache[i] = vce->data;
                }
            }else{
                pred = lineCache->readCache[i-1] + prevLineCache->readCache[i] - prevLineCache->readCache[i-1];
                cur = lineCache->dataCache[i];
                diff = cur-pred;
                itvNum = fabs(diff)/realPrecision + 1;
                if (itvNum < intvCapacity)
                {
                    if (diff < 0) itvNum = -itvNum;
                    type = itvNum/2;
                    lineCache->readCache[i] = pred + realPrecision * 2 * type;
                    lineCache->typeCache[i] = type + intvRadius;
                }else{
                    lineCache->typeCache[i] = 0;
                    compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength, resiBitsLength);
                    updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                    memcpy(preDataBytes,vce->curBytes,4);
                    addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                    lineCache->readCache[i] = vce->data;
                }
            }

            if(i && i % CheckInterval == 0){
                pthread_mutex_lock(&concurrentController->mutex);
                concurrentController->pos += CheckInterval;
                pthread_cond_signal(&concurrentController->cond);
                pthread_mutex_unlock(&concurrentController->mutex);
            }
        }

        pthread_mutex_lock(&concurrentController->mutex);
        concurrentController->pos += CheckInterval*3; // prevent task.lineLength / CheckInterval is not a integer.
        pthread_cond_signal(&concurrentController->cond);
        pthread_mutex_unlock(&concurrentController->mutex);

        //printf("Worker==> tid:%ld, pcc:%d, cc:%d, plc:%d, lc:%d\n", tid, prevConcurrentController->index, concurrentController->index, prevLineCache->index, lineCache->index);

        //printf("Worker==> tid:%ld, done, pos:%d\n", tid, concurrentController->pos);
        TaskDispatcherFinish(taskDispatcher, &task);
    }
    //printf("Worker==> tid:%ld, exit\n", tid);
}

void* WorkerLoop2(void* args){
    struct TaskDispatcher* taskDispatcher = (struct TaskDispatcher*)args;
    struct Task task;
    long tid = syscall(SYS_gettid);
    printf("Worker==> tid:%ld, start\n", tid);
    while((task = TaskDispatcherGet(taskDispatcher), task.lineIndex)){
        struct LineCache* prevLineCache = task.prevLineCache;
        struct LineCache* lineCache = task.lineCache;
        struct ConcurrentController* prevConcurrentController = task.prevConcurrentController;
        struct ConcurrentController* concurrentController = task.concurrentController;
        printf("Worker==> tid:%ld, get task:%d\n", tid, task.lineIndex);

        for(int i=0; i<task.lineLength; i++){
            if(i % CheckInterval == 0){
                pthread_mutex_lock(&prevConcurrentController->mutex);
                while(i-CheckInterval > prevConcurrentController->pos){
                    printf("Worker==> tid:%ld, check fail and wait\n", tid);
                    pthread_cond_wait(&prevConcurrentController->cond, &prevConcurrentController->mutex);
                    printf("Worker==> tid:%ld, end wait\n", tid);
                }
                pthread_mutex_unlock(&prevConcurrentController->mutex);
            }
            if(i == 0){
                float pred = prevLineCache->readCache[i];
                float cur = lineCache->dataCache[i];
                int type = (cur - pred)/0.003;
                lineCache->typeCache[i] = type;
                lineCache->readCache[i] = pred + 0.003*type;
            }else{
                float pred = lineCache->readCache[i-1] + prevLineCache->readCache[i] - prevLineCache->readCache[i-1];
                float cur = lineCache->dataCache[i];
                int type = (cur - pred)/0.003;
                lineCache->typeCache[i] = type;
                lineCache->readCache[i] = pred + 0.003*type;
            }

            if(i &&i % CheckInterval == 0){
                concurrentController->pos += CheckInterval;
                pthread_cond_signal(&concurrentController->cond);
            }
        }

        concurrentController->pos += CheckInterval; // prevent task.lineLength / CheckInterval is not a integer.
        printf("Worker==> tid:%ld, pcc:%d, cc:%d, plc:%d, lc:%d\n", tid, prevConcurrentController->index, concurrentController->index, prevLineCache->index, lineCache->index);

        printf("Worker==> tid:%ld, done\n", tid);
        TaskDispatcherFinish(taskDispatcher, &task);
    }
    printf("Worker==> tid:%ld, exit\n", tid);
}

struct Worker{
    pthread_t pid;
};

void WorkerInit(struct Worker* worker, struct TaskDispatcher* taskDispatcher, int mode){
    if(mode == WorkerType2D){
        pthread_create(&worker->pid, NULL, WorkerLoop1, taskDispatcher);
    }else if (mode == WorkerType3D){
        pthread_create(&worker->pid, NULL, WorkerLoop2, taskDispatcher);
    }

}

void WorkerWait(struct Worker* worker){
    pthread_join(worker->pid, NULL);
}

#endif //SZ_WORKER_H
