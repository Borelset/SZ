//
// Created by xdnzx on 2018/12/22.
//

#ifndef SZ_MULTILEVELCACHETABLE_H
#define SZ_MULTILEVELCACHETABLE_H

#include <stdint.h>
#include <memory.h>
#include "stdio.h"

struct SubLevelTable{
    uint32_t baseIndex;
    uint32_t topIndex;
    uint32_t* table;
};

struct TopLevelTable{
    uint8_t bits;
    uint8_t baseIndex;
    uint8_t topIndex;
    struct SubLevelTable* subTables;
    float bottomBoundary;
    float topBoundary;
};

uint8_t MLCT_GetExpoIndex(float value){
    uint32_t* ptr = (uint32_t*)&value;
    return (*ptr) >> 23;
}

uint8_t MLCT_GetRequiredBits(float precision){
    int32_t* ptr = (int32_t*)&precision;
    return -(((*ptr) >> 23) - 127);
}

uint32_t MLCT_GetMantiIndex(float value, int bits){
    uint32_t* ptr = (uint32_t*)&value;
    (*ptr) = (*ptr) << 9 >> 9;
    int shift = 32 - 9 - bits;
    if(shift > 0){
        return (*ptr) >> shift;
    }else{
        return (*ptr);
    }
}

void MultiLevelCacheTableBuild(struct TopLevelTable* topTable, float* precisionTable, int count, int quantization_intervals, float precision){
    uint8_t bits = MLCT_GetRequiredBits(precision);
    topTable->bits = bits;
    topTable->bottomBoundary = precisionTable[1]/(1+precision);
    topTable->topBoundary = precisionTable[count-1]*(1+precision);
    topTable->baseIndex = MLCT_GetExpoIndex(topTable->bottomBoundary);
    topTable->topIndex = MLCT_GetExpoIndex(topTable->topBoundary);
    int subTableCount = topTable->topIndex - topTable->baseIndex + 1;
    topTable->subTables = (struct SubLevelTable*)malloc(sizeof(struct SubLevelTable) * subTableCount);
    memset(topTable->subTables, 0, sizeof(struct SubLevelTable) * subTableCount);

    uint32_t expoBoundary[subTableCount];
    uint8_t lastExpo = 0xff;
    uint8_t lastIndex = 0;
    for(int i=0; i<count; i++){
        uint8_t expo = MLCT_GetExpoIndex(precisionTable[i]);
        if(expo != lastExpo){
            expoBoundary[lastIndex] = i;
            lastExpo = expo;
            lastIndex++;
        }
    }

    for(int i=topTable->topIndex-topTable->baseIndex; i>0; i--){
        struct SubLevelTable* processingSubTable = &topTable->subTables[i];
        if(i == topTable->topIndex - topTable->baseIndex){
            processingSubTable->topIndex = MLCT_GetMantiIndex(topTable->topBoundary, bits);
        }else{
            uint32_t maxIndex = 0;
            for(int j=0; j<bits; j++){
                maxIndex += 1 << j;
            }
            processingSubTable->topIndex = maxIndex;
        }
        if(i == 1){
            processingSubTable->baseIndex = MLCT_GetMantiIndex(topTable->bottomBoundary, bits)+1;
        }else{
            processingSubTable->baseIndex = 0;
        }

        int subTableLength = processingSubTable->topIndex - processingSubTable->baseIndex + 1;
        processingSubTable->table = (uint32_t*)malloc(sizeof(uint32_t) * subTableLength);
        memset(processingSubTable->table, 0, sizeof(uint32_t) * subTableLength);
    }

    uint32_t lastIndexInExpoRange = count-1;
    bool trigger = false;
    float preRange = 0.0;
    uint32_t preIndex = 0;
    for(int i=topTable->topIndex-topTable->baseIndex; i>0; i--){
        struct SubLevelTable* processingSubTable = &topTable->subTables[i];
        if(trigger){
            uint32_t bound = MLCT_GetMantiIndex(preRange, bits);
            for(int j = processingSubTable->topIndex; j>=processingSubTable->baseIndex; j--){
                if(j >= bound){
                    processingSubTable->table[j-processingSubTable->baseIndex] = preIndex;
                }else{
                    break;
                }
            }
            trigger = false;
        }
        uint32_t firstIndexInExpoRange = expoBoundary[i];
        uint8_t expoInRange = MLCT_GetExpoIndex(precisionTable[firstIndexInExpoRange]);
        for(int j=lastIndexInExpoRange; j>=firstIndexInExpoRange; j--){
            uint32_t rangeTop = MLCT_GetMantiIndex(precisionTable[j]*(1+precision), bits);
            uint32_t rangeBottom;
            if(j == firstIndexInExpoRange){
                preRange = precisionTable[j]/(1+precision);
                if(expoInRange != MLCT_GetExpoIndex(preRange)){
                    trigger = true;
                    preIndex = firstIndexInExpoRange;
                    rangeBottom = 0;
                }else{
                    rangeBottom= MLCT_GetMantiIndex(precisionTable[j]/(1+precision), bits);
                }
            }else{
                rangeBottom= MLCT_GetMantiIndex(precisionTable[j]/(1+precision), bits);
            }
            for(int k = rangeBottom; k<=rangeTop; k++){
                if( k <= processingSubTable->topIndex && k >= processingSubTable->baseIndex)
                    processingSubTable->table[k - processingSubTable->baseIndex] = j;
            }
        }
        lastIndexInExpoRange = firstIndexInExpoRange-1;
    }
}

uint32_t MultiLevelCacheTableGetIndex(float value, struct TopLevelTable* topLevelTable){
    if(value <= topLevelTable->topBoundary && value >= topLevelTable->bottomBoundary){
        uint8_t expoIndex = MLCT_GetExpoIndex(value);
        struct SubLevelTable* subLevelTable = &topLevelTable->subTables[expoIndex-topLevelTable->baseIndex];
        uint32_t mantiIndex = MLCT_GetMantiIndex(value, topLevelTable->bits);
        return subLevelTable->table[mantiIndex - subLevelTable->baseIndex];
    }else{
        return 0;
    }

}

#endif //SZ_MULTILEVELCACHETABLE_H
