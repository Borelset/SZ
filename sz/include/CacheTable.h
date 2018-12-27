//
// Created by borelset on 2018/12/18.
//

#ifndef SZ_MASTER_CACHETABLE_H
#define SZ_MASTER_CACHETABLE_H

#include "stdio.h"
#include "stdint.h"
#include <math.h>

double* g_CacheTable;
uint32_t * g_InverseTable;
uint32_t baseIndex;
uint32_t topIndex;
int bits;

int doubleGetExpo(double d);

int CacheTableGetRequiredBits(double precision, int quantization_intervals);

uint32_t CacheTableGetIndex(float value, int bits);

uint64_t CacheTableGetIndexDouble(double value, int bits);

int CacheTableIsInBoundary(uint32_t index);

void CacheTableBuild(double * table, int count, double smallest, double largest, double precision, int quantization_intervals);

uint32_t CacheTableFind(uint32_t index);

void CacheTableFree();

#endif //SZ_MASTER_CACHETABLE_H
