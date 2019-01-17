//
// Created by borelset on 2019/1/16.
//

#include "Worker.h"

int main(){
    float array[20] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0 ,13.0, 14.0, 15.0, 16.0, 17.0, 18.0,19.0, 20.0};
    int typeArray[20];

    TaskDispatcher taskDispatcher;
    LineCache lineCache;
    lineCache.index = -1;
    float readCache[] = {1.0, 1.999, 2.998, 4.0, 4.999};
    typeArray[0] = 0;
    typeArray[1] = 333;
    typeArray[2] = 333;
    typeArray[3] = 334;
    typeArray[4] = 333;
    lineCache.readCache = readCache;
    //TaskDispatcherInit(&taskDispatcher, 4, &lineCache, 5, typeArray, array, 0.003);

    Worker worker1;
    Worker worker2;
    WorkerInit(&worker1, &taskDispatcher, 1);
    WorkerInit(&worker2, &taskDispatcher, 1);

    WorkerWait(&worker1);
    WorkerWait(&worker2);

    printf("typeArray\n");
    for(int i=0; i<20; i++){
        printf("%d ", typeArray[i]);
    }
    printf("\n");
}
