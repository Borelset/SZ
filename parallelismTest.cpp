//
// Created by xdnzx on 2019/1/16.
//

#include "Worker.h"

int main(){
    TaskDispatcher taskDispatcher;
    LineCache lineCache;
    lineCache.index = -1;
    TaskDispatcherInit(&taskDispatcher, 100, &lineCache, 100);
    Worker worker1;
    Worker worker2;
    WorkerInit(&worker1, &taskDispatcher);
    WorkerInit(&worker2, &taskDispatcher);

    WorkerWait(&worker1);
    WorkerWait(&worker2);
}
