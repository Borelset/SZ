//
// Created by xdnzx on 2019/1/17.
//

#ifndef SZ_WORKERLOG_H
#define SZ_WORKERLOG_H

#include <stdbool.h>
#include <pthread.h>

struct WorkerLog{
    pthread_t pid;
};


struct LogParams{
    float* input;
    float* output;
    unsigned char* signs;
    int start;
    int end;
    float zeroValue;
    double realPrecision;
};

struct LogResult{
    float maxLog;
    float minLog;
    bool positive;
};

void* WorkerLoopLog(void* args){
    struct LogParams* logParams = (struct LogParams*)args;
    struct LogResult* logResult = (struct LogResult*)malloc(sizeof(struct LogResult));
    int start = logParams->start;
    int end = logParams->end;
    float* input = logParams->input;
    float* output = logParams->output;
    unsigned char* signs = logParams->signs;
    float minLog = 200, maxLog = -200;
    for(int i=start; i<=end; i++){
        if(input[i] < 0){
            output[i] = -input[i];
            signs[i] = 1;
            logResult->positive = false;
        }else{
            output[i] = input[i];
        }
        if(output[i] > 0){
            output[i] = log2(output[i]);
            if(output[i] > maxLog) maxLog = output[i];
            if(output[i] < minLog) minLog = output[i];
        }
    }
    logResult->maxLog = maxLog;
    logResult->minLog = minLog;
    return logResult;
}

void* WorkerLoopTransZero(void* args){
    struct LogParams* logParams = (struct LogParams*)args;
    float zeroValue = logParams->zeroValue;
    int start = logParams->start;
    int end = logParams->end;
    float* input = logParams->input;
    float* output = logParams->output;
    double realPrecision = logParams->realPrecision;
    for(int i=start; i<=end; i++){
        if(input[i] == 0){
            output[i] = zeroValue - 2.0001*realPrecision;
        }
    }
}


void WorkerInitLog(struct WorkerLog* worker, struct LogParams* logParams){
    pthread_create(&worker->pid, NULL, WorkerLoopLog, logParams);
}

void WorkerWaitLog(struct WorkerLog* worker, struct LogResult** pLogResult){
    pthread_join(worker->pid, pLogResult);
}

void WorkerInitTransZero(struct WorkerLog* worker, struct LogParams* logParams){
    pthread_create(&worker->pid, NULL, WorkerLoopTransZero, logParams);
}

void WorkerWaitTransZero(struct WorkerLog* worker){
    pthread_join(worker->pid, NULL);
}

#endif //SZ_WORKERLOG_H
