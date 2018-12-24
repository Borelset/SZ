//
// Created by borelset on 2018/12/24.
//

#ifndef SZ_MULTILEVELCACHETABLEWIDEINTERVAL_H
#define SZ_MULTILEVELCACHETABLEWIDEINTERVAL_H

#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include "stdio.h"

struct SubLevelTableWideInterval{
    uint64_t baseIndex;
    uint64_t topIndex;
    uint32_t* table;
    uint16_t expoIndex;
};

struct TopLevelTableWideInterval{
    uint16_t bits;
    uint16_t baseIndex;
    uint16_t topIndex;
    struct SubLevelTableWideInterval* subTables;
    double bottomBoundary;
    double topBoundary;
};

uint16_t MLCTWI_GetExpoIndex(double value){
    uint64_t* ptr = (uint64_t*)&value;
    return (*ptr) >> 52;
}

uint16_t MLCTWI_GetRequiredBits(double precision){
    uint64_t* ptr = (uint64_t*)&precision;
    return -(((*ptr) >> 52) - 1023);
}

uint64_t MLCTWI_GetMantiIndex(double value, int bits){
    uint64_t* ptr = (uint64_t*)&value;
    (*ptr) = (*ptr) << 12 >> 12;
    int shift = 64 - 12 - bits;
    if(shift > 0){
        return (*ptr) >> shift;
    }else{
        return (*ptr);
    }
}

double MLTCWI_RebuildDouble(uint16_t expo, uint64_t manti, int bits){
    double result;
    uint64_t *ptr = (uint64_t*)&result;
    *ptr = expo;
    (*ptr) = (*ptr) << 52;
    (*ptr) |= (manti << (52-bits));
    return result;
}

void MultiLevelCacheTableWideIntervalBuild(struct TopLevelTableWideInterval* topTable, double* precisionTable, int count, double precision){
    uint16_t bits = MLCTWI_GetRequiredBits(precision);
    topTable->bits = bits;
    topTable->bottomBoundary = precisionTable[1]/(1+precision);
    topTable->topBoundary = precisionTable[count-1]/(1-precision);
    topTable->baseIndex = MLCTWI_GetExpoIndex(topTable->bottomBoundary);
    topTable->topIndex = MLCTWI_GetExpoIndex(topTable->topBoundary);
    int subTableCount = topTable->topIndex - topTable->baseIndex + 1;
    topTable->subTables = (struct SubLevelTableWideInterval*)malloc(sizeof(struct SubLevelTableWideInterval) * subTableCount);
    memset(topTable->subTables, 0, sizeof(struct SubLevelTableWideInterval) * subTableCount);

    uint32_t expoBoundary[subTableCount];
    uint16_t lastExpo = 0xffff;
    uint16_t lastIndex = 0;
    for(int i=0; i<count; i++){
        uint16_t expo = MLCTWI_GetExpoIndex(precisionTable[i]);
        if(expo != lastExpo){
            expoBoundary[lastIndex] = i;
            lastExpo = expo;
            lastIndex++;
        }
    }

    for(int i=topTable->topIndex-topTable->baseIndex; i>=0; i--){
        struct SubLevelTableWideInterval* processingSubTable = &topTable->subTables[i];
        if(i == topTable->topIndex - topTable->baseIndex &&
           MLCTWI_GetExpoIndex(topTable->topBoundary) == MLCTWI_GetExpoIndex(precisionTable[count-1])){
            processingSubTable->topIndex = MLCTWI_GetMantiIndex(topTable->topBoundary, bits);
        }else{
            uint32_t maxIndex = 0;
            for(int j=0; j<bits; j++){
                maxIndex += 1 << j;
            }
            processingSubTable->topIndex = maxIndex;
        }
        if(i == 0 && MLCTWI_GetExpoIndex(topTable->topBoundary) == MLCTWI_GetExpoIndex(precisionTable[count-1])){
            processingSubTable->baseIndex = MLCTWI_GetMantiIndex(topTable->bottomBoundary, bits);
        }else{
            processingSubTable->baseIndex = 0;
        }

        uint64_t subTableLength = processingSubTable->topIndex - processingSubTable-> baseIndex+ 1;
        processingSubTable->table = (uint32_t*)malloc(sizeof(uint32_t) * subTableLength);
        memset(processingSubTable->table, 0, sizeof(uint32_t) * subTableLength);
        processingSubTable->expoIndex = topTable->baseIndex + i;
    }


    uint32_t index = 0;
    for(uint16_t i = 0; i<=topTable->topIndex-topTable->baseIndex; i++){
        struct SubLevelTableWideInterval* processingSubTable = &topTable->subTables[i];
        uint16_t expoIndex = i+topTable->baseIndex;
        for(uint32_t j = 0; j<=processingSubTable->topIndex - processingSubTable->baseIndex; j++){
            uint64_t mantiIndex = j + processingSubTable->baseIndex;
            double sample = MLTCWI_RebuildDouble(expoIndex, mantiIndex, topTable->bits);
            double bottomBoundary = precisionTable[index] / (1+precision);
            double topBoundary = precisionTable[index] / (1-precision);
            if(sample < topBoundary && sample > bottomBoundary){
                processingSubTable->table[j] = index;
            }else{
                index++;
                processingSubTable->table[j] = index;
            }
        }
    }

}

uint32_t MultiLevelCacheTableWideIntervalGetIndex(double value, struct TopLevelTableWideInterval* topLevelTable){
    uint16_t expoIndex = MLCTWI_GetExpoIndex(value);
    if(expoIndex <= topLevelTable->topIndex && expoIndex >= topLevelTable->baseIndex){
        struct SubLevelTableWideInterval* subLevelTable = &topLevelTable->subTables[expoIndex-topLevelTable->baseIndex];
        uint64_t mantiIndex = MLCTWI_GetMantiIndex(value, topLevelTable->bits);
        double rebuild = MLTCWI_RebuildDouble(expoIndex, mantiIndex, topLevelTable->bits);
        if(mantiIndex >= subLevelTable->baseIndex && mantiIndex <= subLevelTable->topIndex)
            return subLevelTable->table[mantiIndex - subLevelTable->baseIndex];
    }
    return 0;
}

void MultiLevelCacheTableWideIntervalFree(struct TopLevelTableWideInterval* table){
    for(int i=0; i<table->topIndex - table->baseIndex + 1; i++){
        free(table->subTables[i].table);
    }
    free(table->subTables);
}

#endif //SZ_MULTILEVELCACHETABLEWIDEINTERVAL_H
