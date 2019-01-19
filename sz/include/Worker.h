//
// Created by borelset on 2019/1/16.
//

#ifndef SZ_WORKER_H
#define SZ_WORKER_H

#include <stdio.h>
#include <syscall.h>
#include <zconf.h>
#include <stdbool.h>
#include "TaskDispatcher.h"
#include "math.h"
#include "CompressElement.h"
#include "dataCompression.h"

#define WorkerType2D 1
#define WorkerType3D 2

struct Worker{
    pthread_t pid;
};

void* WorkerLoop2D(void *args){
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
    int checkInterval = taskDispatcher->szParams.parallelismCheckInterval;

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
            if(i % checkInterval == 0){
                pthread_mutex_lock(&prevConcurrentController->mutex);
                if(i+checkInterval > prevConcurrentController->pos){
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

            if(i && i % checkInterval == 0){
                pthread_mutex_lock(&concurrentController->mutex);
                concurrentController->pos += checkInterval;
                pthread_cond_signal(&concurrentController->cond);
                pthread_mutex_unlock(&concurrentController->mutex);
            }
        }

        pthread_mutex_lock(&concurrentController->mutex);
        concurrentController->pos += checkInterval*3; // prevent task.lineLength / checkInterval is not a integer.
        pthread_cond_signal(&concurrentController->cond);
        pthread_mutex_unlock(&concurrentController->mutex);

        //printf("Worker==> tid:%ld, pcc:%d, cc:%d, plc:%d, lc:%d\n", tid, prevConcurrentController->index, concurrentController->index, prevLineCache->index, lineCache->index);

        //printf("Worker==> tid:%ld, done, pos:%d\n", tid, concurrentController->pos);
        TaskDispatcherFinish(taskDispatcher, &task);
    }
    //printf("Worker==> tid:%ld, exit\n", tid);
}

void* WorkerLoop3D(void *args) {
    struct TaskDispatcher *taskDispatcher = (struct TaskDispatcher *) args;
    struct Task task;
    long tid = syscall(SYS_gettid);
    //printf("Worker==> tid:%ld, start\n", tid);

    double realPrecision = taskDispatcher->szParams.realPrecision;
    int intvRadius = taskDispatcher->szParams.intvRadius;
    int intvCapacity = intvRadius * 2;
    float medianValue = taskDispatcher->szParams.medianValue;
    int reqLength = taskDispatcher->szParams.reqLength;
    int reqBytesLength = taskDispatcher->szParams.reqBytesLength;
    int resiBitsLength = taskDispatcher->szParams.resiBitsLength;
    int checkInterval = taskDispatcher->szParams.parallelismCheckInterval;

    float pred, cur, diff;
    int type;
    double itvNum;
    unsigned char preDataBytes[4];

    FloatValueCompressElement *vce = (FloatValueCompressElement *) malloc(sizeof(FloatValueCompressElement));
    LossyCompressionElement *lce = (LossyCompressionElement *) malloc(sizeof(LossyCompressionElement));
    float *thisLayerPred;
    float *prevLayerPred;
    float *curData;
    int *curType;

    while ((task = TaskDispatcherGet(taskDispatcher), task.lineIndex)) {
        struct LineCache *prevLineCache = task.prevLineCache;
        struct LineCache *lineCache = task.lineCache;
        struct ConcurrentController *prevConcurrentController = task.prevConcurrentController;
        struct ConcurrentController *concurrentController = task.concurrentController;
        //printf("Worker==> tid:%ld, get task:%d\n", tid, task.lineIndex);

        DynamicIntArray *exactLeadNumArray = task.exactLeadNumArray;
        DynamicByteArray *exactMidByteArray = task.exactMidByteArray;
        DynamicIntArray *resiBitArray = task.resiBitArray;

        thisLayerPred = lineCache->readCache;
        prevLayerPred = prevLineCache->readCache;
        curData = lineCache->dataCache;
        curType = lineCache->typeCache;

        //==================================第一行=========================================================================
        pthread_mutex_lock(&prevConcurrentController->mutex);
        if (0 + checkInterval > prevConcurrentController->pos) {
            //printf("Worker==> tid:%ld, check fail and wait for:%d, pos:%d, prev_pos:%d\n", tid, prevConcurrentController->index, 0, prevConcurrentController->pos);
            pthread_cond_wait(&prevConcurrentController->cond, &prevConcurrentController->mutex);
            //printf("Worker==> tid:%ld, end wait, pos:%d\n", tid, 0);
        }
        pthread_mutex_unlock(&prevConcurrentController->mutex);

        pred = prevLayerPred[0];
        cur = curData[0];
        diff = cur - pred;
        itvNum = fabs(diff) / realPrecision + 1;
        if (itvNum < intvCapacity) {
            if (diff < 0) itvNum = -itvNum;
            type = itvNum / 2;
            thisLayerPred[0] = pred + realPrecision * 2 * type;
            curType[0] = type + intvRadius;

            if(fabs(cur-thisLayerPred[0])>realPrecision)
            {
                curType[0] = 0;
                compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength, resiBitsLength);
                updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                memcpy(preDataBytes,vce->curBytes,4);
                addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                thisLayerPred[0] = vce->data;
            }

        } else {
            curType[0] = 0;
            compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength, resiBitsLength);
            updateLossyCompElement_Float_first(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
            memcpy(preDataBytes, vce->curBytes, 4);
            addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
            thisLayerPred[0] = vce->data;
        }

        for (int i = 1; i < task.width_3D; i++) {
            pred = thisLayerPred[i - 1] + prevLayerPred[i] - prevLayerPred[i - 1];
            cur = curData[i];
            diff = cur - pred;
            itvNum = fabs(diff) / realPrecision + 1;
            if (itvNum < intvCapacity) {
                if (diff < 0) itvNum = -itvNum;
                type = itvNum / 2;
                thisLayerPred[i] = pred + realPrecision * 2 * type;
                curType[i] = type + intvRadius;

                if(fabs(cur-thisLayerPred[i])>realPrecision)
                {
                    curType[i] = 0;
                    compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength, resiBitsLength);
                    updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                    memcpy(preDataBytes,vce->curBytes,4);
                    addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                    thisLayerPred[i] = vce->data;
                }

            } else {
                curType[i] = 0;
                compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength,
                                         resiBitsLength);
                updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                memcpy(preDataBytes, vce->curBytes, 4);
                addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                thisLayerPred[i] = vce->data;
            }
        }
        pthread_mutex_lock(&concurrentController->mutex);
        concurrentController->pos = task.width_3D;
        pthread_cond_signal(&concurrentController->cond);
        pthread_mutex_unlock(&concurrentController->mutex);

        pthread_mutex_lock(&prevConcurrentController->mutex);
        if (task.width_3D + checkInterval > prevConcurrentController->pos) {
            //printf("Worker==> tid:%ld, check fail and wait for:%d, pos:%d, prev_pos:%d\n", tid, prevConcurrentController->index, task.width_3D, prevConcurrentController->pos);
            pthread_cond_wait(&prevConcurrentController->cond, &prevConcurrentController->mutex);
            //printf("Worker==> tid:%ld, end wait, pos:%d\n", tid, task.width_3D);
        }
        pthread_mutex_unlock(&prevConcurrentController->mutex);
        //===================================================================================================================

        for (int i = task.width_3D; i < task.lineLength; i++) {
            if (i % checkInterval == 0) {
                pthread_mutex_lock(&prevConcurrentController->mutex);
                if (i + checkInterval > prevConcurrentController->pos) {
                    //printf("Worker==> tid:%ld, check fail and wait for:%d, pos:%d, prev_pos:%d\n", tid, prevConcurrentController->index, i, prevConcurrentController->pos);
                    pthread_cond_wait(&prevConcurrentController->cond, &prevConcurrentController->mutex);
                    //printf("Worker==> tid:%ld, end wait, pos:%d\n", tid, i);
                }
                pthread_mutex_unlock(&prevConcurrentController->mutex);
            }
            if (i % task.width_3D) {
                pred = thisLayerPred[i - 1] + thisLayerPred[i - task.width_3D] + prevLayerPred[i] -
                       thisLayerPred[i - task.width_3D - 1] - prevLayerPred[i - task.width_3D] - prevLayerPred[i - 1] +
                       prevLayerPred[i - task.width_3D - 1];
                cur = curData[i];
                diff = cur - pred;
                itvNum = fabs(diff) / realPrecision + 1;
                if (itvNum < intvCapacity) {
                    if (diff < 0) itvNum = -itvNum;
                    type = itvNum / 2;
                    thisLayerPred[i] = pred + realPrecision * 2 * type;
                    curType[i] = type + intvRadius;

                    if(fabs(cur-thisLayerPred[i])>realPrecision)
                    {
                        curType[i] = 0;
                        compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength, resiBitsLength);
                        updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                        memcpy(preDataBytes,vce->curBytes,4);
                        addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                        thisLayerPred[i] = vce->data;
                    }

                } else {
                    curType[i] = 0;
                    compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength,
                                             resiBitsLength);
                    updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                    memcpy(preDataBytes, vce->curBytes, 4);
                    addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                    thisLayerPred[i] = vce->data;
                }
            } else {
                pred = thisLayerPred[i - task.width_3D] + prevLayerPred[i] - prevLayerPred[i - task.width_3D];
                cur = curData[i];
                diff = cur - pred;
                itvNum = fabs(diff) / realPrecision + 1;
                if (itvNum < intvCapacity) {
                    if (diff < 0) itvNum = -itvNum;
                    type = itvNum / 2;
                    thisLayerPred[i] = pred + realPrecision * 2 * type;
                    curType[i] = type + intvRadius;

                    if(fabs(cur-thisLayerPred[i])>realPrecision)
                    {
                        curType[i] = 0;
                        compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength, resiBitsLength);
                        updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                        memcpy(preDataBytes,vce->curBytes,4);
                        addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                        thisLayerPred[i] = vce->data;
                    }

                } else {
                    curType[i] = 0;
                    compressSingleFloatValue(vce, cur, realPrecision, medianValue, reqLength, reqBytesLength,
                                             resiBitsLength);
                    updateLossyCompElement_Float(vce->curBytes, preDataBytes, reqBytesLength, resiBitsLength, lce);
                    memcpy(preDataBytes, vce->curBytes, 4);
                    addExactData(exactMidByteArray, exactLeadNumArray, resiBitArray, lce);
                    thisLayerPred[i] = vce->data;
                }
            }
            if (i % checkInterval == 0) {
                pthread_mutex_lock(&concurrentController->mutex);
                concurrentController->pos = i;
                pthread_cond_signal(&concurrentController->cond);
                pthread_mutex_unlock(&concurrentController->mutex);
            }
        }
        pthread_mutex_lock(&concurrentController->mutex);
        concurrentController->pos += checkInterval * 3; // prevent task.lineLength / checkInterval is not a integer.
        pthread_cond_signal(&concurrentController->cond);
        pthread_mutex_unlock(&concurrentController->mutex);

        //printf("Worker==> tid:%ld, pcc:%d, cc:%d, plc:%d, lc:%d\n", tid, prevConcurrentController->index, concurrentController->index, prevLineCache->index, lineCache->index);

        //printf("Worker==> tid:%ld, done, pos:%d\n", tid, concurrentController->pos);
        TaskDispatcherFinish(taskDispatcher, &task);
    }
    //printf("Worker==> tid:%ld, exit\n", tid);
}

void WorkerInit(struct Worker* worker, struct TaskDispatcher* taskDispatcher, int mode){
    if(mode == WorkerType2D){
        pthread_create(&worker->pid, NULL, WorkerLoop2D, taskDispatcher);
    }else if (mode == WorkerType3D){
        pthread_create(&worker->pid, NULL, WorkerLoop3D, taskDispatcher);
    }
}

void WorkerWait(struct Worker* worker){
    pthread_join(worker->pid, NULL);
}

#endif //SZ_WORKER_H
