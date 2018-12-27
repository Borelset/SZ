//
// Created by borelset on 2018/12/17.
//

#ifndef SZ_MASTER_TIMEDURATION_H
#define SZ_MASTER_TIMEDURATION_H

#include <sys/time.h>

struct ClockPoint{
    struct timeval t0, t1;
    char* namePtr;
};

static void TimeDurationStart(char* name, struct ClockPoint* clockPoint){
    gettimeofday(&clockPoint->t0, NULL);
    clockPoint->namePtr = (char*)malloc(sizeof(name));
    memcpy(clockPoint->namePtr, name, sizeof(name));
}

static void TimeDurationEnd(struct ClockPoint* clockPoint){
    gettimeofday(&clockPoint->t1, NULL);
    printf("%s:duration:%ldus\n", clockPoint->namePtr, (clockPoint->t1.tv_sec-clockPoint->t0.tv_sec)*1000000 + (clockPoint->t1.tv_usec - clockPoint->t0.tv_usec));
    free(clockPoint->namePtr);
}

#endif //SZ_MASTER_TIMEDURATION_H
