/**
 *  @file DynamicIntArray.c
 *  @author Sheng Di
 *  @date May, 2016
 *  @brief Dynamic Int Array
 *  (C) 2016 by Mathematics and Computer Science (MCS), Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include "DynamicIntArray.h"

void new_DIA(DynamicIntArray **dia, size_t cap) {
		*dia = (DynamicIntArray *)malloc(sizeof(DynamicIntArray));
        (*dia)->size = 0;
        (*dia)->capacity = cap;
        (*dia)->array = (unsigned char*)malloc(sizeof(unsigned char)*cap);
    }

void convertDIAtoInts(DynamicIntArray *dia, unsigned char **data)
{
	size_t size = dia->size;
	if(size>0)
		*data = (unsigned char*)malloc(size * sizeof(char));
	else
		*data = NULL;
	memcpy(*data, dia->array, size*sizeof(unsigned char));	
}

void free_DIA(DynamicIntArray *dia)
{
	free(dia->array);
	free(dia);
}

int getDIA_Data(DynamicIntArray *dia, size_t pos)
{
	if(pos>=dia->size)
	{
		printf("Error: wrong position of DIA.\n");
		exit(0);
	}
	return dia->array[pos];
}

void addDIA_Data(DynamicIntArray *dia, int value)
{
	if(dia->size==dia->capacity)
	{
		dia->capacity = dia->capacity << 1;
		dia->array = (unsigned char *)realloc(dia->array, dia->capacity*sizeof(unsigned char));
	}
	dia->array[dia->size] = (unsigned char)value;
	dia->size ++;
}

void new_DI32A(DynamicInt32Array **dia, size_t cap) {
	*dia = (DynamicIntArray *)malloc(sizeof(DynamicIntArray));
	(*dia)->size = 0;
	(*dia)->capacity = cap;
	(*dia)->array = (int*)malloc(sizeof(int)*cap);
}

void convertDI32AtoInts(DynamicInt32Array *dia, int **data)
{
	size_t size = dia->size;
	if(size>0)
		*data = (int*)malloc(size * sizeof(int));
	else
		*data = NULL;
	memcpy(*data, dia->array, size*sizeof(int));
}

void free_DI32A(DynamicInt32Array *dia)
{
	free(dia->array);
	free(dia);
}

int getDI32A_Data(DynamicInt32Array *dia, size_t pos)
{
	if(pos>=dia->size)
	{
		printf("Error: wrong position of DIA.\n");
		exit(0);
	}
	return dia->array[pos];
}

void addDI32A_Data(DynamicInt32Array *dia, int value)
{
	if(dia->size==dia->capacity)
	{
		dia->capacity = dia->capacity << 1;
		dia->array = (int*)realloc(dia->array, dia->capacity*sizeof(int));
	}
	dia->array[dia->size] = value;
	dia->size ++;
}