//
// Created by borelset on 2018/12/22.
//

#include <math.h>
#include "MultiLevelCacheTableWideInterval.h"
#include "Huffman.h"

#define C   10
#define R   10
#define ArrayLength C*R
#define StateNum 2048

int main(){

    HuffmanTree* huffmanTree1 = createHuffmanTree(StateNum);
    int array[ArrayLength];
    for(int i=0; i<R; i++){
        int number = rand()%StateNum;
        for(int j=0; j<C; j++){
            array[i*10+j] = number;
        }
    }
    for(int i=0; i<ArrayLength; i++){
        printf("%d ", array[i]);
    }
    printf("\n");
    unsigned char * buffer = (unsigned char*)malloc(sizeof(array));
    size_t hsize;
    int maxBits = encode_withTree_alter(huffmanTree1, array, ArrayLength, &buffer, &hsize);
    SZ_ReleaseHuffman(huffmanTree1);

    size_t tsize = sizeof(array)/ sizeof(int);
    int * array2 = (int*)malloc(sizeof(array));
    HuffmanTree* huffmanTree2 = createHuffmanTree(StateNum);
    decode_withTree_alter(huffmanTree2, buffer, tsize, array2, maxBits);
    SZ_ReleaseHuffman(huffmanTree2);
    for(int i=0; i<tsize; i++){
        printf("%d ", array2[i]);
    }

    /*
    FILE* file = fopen("testFile", "w");
    float one = 1.0;
    float min = 0.000001;
    float hundred = 1.0e20;
    for(int i=0; i<100; i++){
        fwrite(&one, sizeof(float), 1, file);
    }
    for(int i=0; i<100; i++){
        fwrite(&min, sizeof(float), 1, file);
    }
    for(int i=0; i<200; i++){
        fwrite(&hundred, sizeof(float), 1, file);
    }
     */

    /*
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
     */

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
