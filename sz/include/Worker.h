//
// Created by xdnzx on 2019/1/16.
//

#ifndef SZ_WORKER_H
#define SZ_WORKER_H

#include <cstdio>
#include <syscall.h>
#include <zconf.h>
#include "TaskDispatcher.h"

#define CheckInterval 10

void* WorkerLoop(void* args){
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



            if(i % CheckInterval == 0){
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

void WorkerInit(struct Worker* worker, struct TaskDispatcher* taskDispatcher){
    pthread_create(&worker->pid, NULL, WorkerLoop, taskDispatcher);
}

void WorkerWait(Worker* worker){
    pthread_join(worker->pid, NULL);
}

#endif //SZ_WORKER_H
