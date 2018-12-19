//
// Created by xdnzx on 2018/12/18.
//

#ifndef SZ_MASTER_CACHETABLE_H
#define SZ_MASTER_CACHETABLE_H

#include "stdio.h"
#include "stdint.h"
#include <math.h>

double* g_CacheTable;
uint32_t* g_InverseTable;
uint64_t baseIndex;
uint64_t topIndex;
int bits;

int doubleGetExpo(double d){
    long* ptr = &d;
    *ptr = ((*ptr) >> 52) - 1023;
    return *ptr;
}

int CacheTableGetRequiredBits(double precision, int quantization_intervals){
    double min_distance = pow((1+precision), -(quantization_intervals>>1)) * precision;
    return -(doubleGetExpo(min_distance));
}

uint32_t CacheTableGetIndex(float value, int bits){
    uint32_t* ptr = (uint32_t*)&value;
    int shift = 32 - 8 - bits;
    if(shift>0){
        return (*ptr) >> shift;
    }else{
        return 0;
    }
}

uint64_t CacheTableGetIndexDouble(double value, int bits){
    uint64_t* ptr = (uint64_t*)&value;
    int shift = 64 - 12 - bits;
    if(shift>0){
        return (*ptr) >> shift;
    }else{
        return 0;
    }
}

static int CacheTableIsInBoundary(float value){
    int index = CacheTableGetIndex(value, bits);
    if(index <= topIndex && index > baseIndex){
        return 1;
    }else{
        return 0;
    }
}

void CacheTableBuild(double * table, int count, double smallest, double largest, double precision, int quantization_intervals){
    bits = CacheTableGetRequiredBits(precision, quantization_intervals);
    baseIndex = CacheTableGetIndex((float)smallest, bits);
    topIndex = CacheTableGetIndex((float)largest, bits);
    uint32_t range = topIndex - baseIndex;
    g_InverseTable = (uint32_t *)malloc(sizeof(uint32_t) * range);
    uint32_t fillInPos = 0;
    for(int i=0; i<count; i++){
        if(i == 0){
            continue;
        }
        uint32_t index = CacheTableGetIndex((float)table[i], bits) - baseIndex;
        g_InverseTable[index] = i;
        if(index > fillInPos){
            for(int j=fillInPos; j<index; j++){
                g_InverseTable[j] = g_InverseTable[index];
            }
        }
        fillInPos = index + 1;
    }
}

uint32_t CacheTableFind(double predRatio){
    uint32_t index = CacheTableGetIndex(predRatio, bits);
    return g_InverseTable[index-baseIndex];
}

void CacheTableFree(){
    free(g_InverseTable);
}

#endif //SZ_MASTER_CACHETABLE_H
