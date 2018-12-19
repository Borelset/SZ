//
// Created by xdnzx on 2018/12/17.
//

#ifndef SZ_MASTER_TIMEDURATION_H
#define SZ_MASTER_TIMEDURATION_H

static struct timeval t0, t1;
static char* namePtr = NULL;

static void TimeDurationStart(char* name){
    gettimeofday(&t0, NULL);
    namePtr = name;
}

static void TimeDurationEnd(){
    gettimeofday(&t1, NULL);
    printf("%s:duration:%ldus\n", namePtr, (t1.tv_sec-t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec));
}

#endif //SZ_MASTER_TIMEDURATION_H
