//
// Created by borelset on 2018/12/22.
//

#include <math.h>
#include "MultiLevelCacheTableWideInterval.h"

int main(){
    TopLevelTableWideInterval topLevelTable;
    double precision = 1e-3;
    int quant_int = 32768;
    double* precisionTable = (double*)malloc(sizeof(double) * quant_int);
    for(int i=0; i<quant_int; i++){
        precisionTable[i] = pow((1+precision), (i-quant_int/2)*2);
    }
    double min = precisionTable[0];
    double max = precisionTable[quant_int-1];
    MultiLevelCacheTableWideIntervalBuild(&topLevelTable, precisionTable, quant_int, precision);

    for(uint16_t i=topLevelTable.baseIndex; i<=topLevelTable.topIndex; i++){
        SubLevelTableWideInterval* subLevelTable = &topLevelTable.subTables[i - topLevelTable.baseIndex];
        for(uint64_t j=subLevelTable->baseIndex; j<=subLevelTable->topIndex; j++){
            uint32_t index = subLevelTable->table[j - subLevelTable->baseIndex];
            double value = precisionTable[index];
            double rebuild = MLTCWI_RebuildDouble(i, j, topLevelTable.bits);
            double delta = abs(value - rebuild);
            if(delta / rebuild > precision){
                printf("expo:%d\tmanti:%d\tprecision:%f\trebuild:%f\n", i, j, value, rebuild);
            }
        }
    }

    /*
    TopLevelTable topLevelTable;
    float precision = 1e-3;
    int quant_int = 256;
    float* precisionTable = (float*)malloc(sizeof(float) * quant_int);
    for(int i=0; i<quant_int; i++){
        precisionTable[i] = pow((1+precision), i-quant_int/2);
    }
    float min = precisionTable[0];
    float max = precisionTable[quant_int-1];
    MultiLevelCacheTableBuild(&topLevelTable, precisionTable, quant_int, precision);

    for(int i=topLevelTable.baseIndex; i<=topLevelTable.topIndex; i++){
        SubLevelTable* subLevelTable = &topLevelTable.subTables[i - topLevelTable.baseIndex];
        for(int j=subLevelTable->baseIndex; j<=subLevelTable->topIndex; j++){
            uint32_t index = subLevelTable->table[j - subLevelTable->baseIndex];
            float value = precisionTable[index];
            float rebuild = MLTC_RebuildFloat(i, j, topLevelTable.bits);
            float delta = abs(value - rebuild);
            if(delta / rebuild > precision){
                printf("expo:%d\tmanti:%d\tprecision:%f\trebuild:%f\n", i, j, value, rebuild);
            }
        }
    }
     */
}