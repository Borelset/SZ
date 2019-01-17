//
// Created by borelset on 2019/1/16.
//

#ifndef SZ_LINECACHE_H
#define SZ_LINECACHE_H

struct LineCache{
    int index;
    float * readCache;
    int * typeCache;
    float * dataCache;
};

void LineCacheInit(struct LineCache* lineCache){
    lineCache->readCache = NULL;
    lineCache->typeCache = NULL;
    lineCache->dataCache = NULL;
}

struct LCSourcePool{
    struct List idle;
    int counter;
    int readCacheLength;
};

void LineCacheDestroy(struct LineCache* lineCache){
    if(lineCache->readCache){
        free(lineCache->readCache);
    }
}

void LCSourcePoolInit(struct LCSourcePool* lcSourcePool, int readCacheLength){
    lcSourcePool->readCacheLength = readCacheLength;
    ListInit(&lcSourcePool->idle);
    lcSourcePool->counter = 0;
}

void LCSourcePoolDestroy(struct LCSourcePool* lcSourcePool){
    struct LineCache* lineCache;
    for(int i=0; i<lcSourcePool->idle.count; i++){
        lineCache = (struct LineCache*)ListGet(&lcSourcePool->idle);
        LineCacheDestroy(lineCache);
        free(lineCache);
    }
    ListDestroy(&lcSourcePool->idle);
}

struct LineCache* LCSourcePoolGet(struct LCSourcePool* lcSourcePool){
    struct LineCache* ptr;
    if(lcSourcePool->idle.count){
        ptr = (struct LineCache*)ListGet(&lcSourcePool->idle);
    }else{
        ptr = (struct LineCache*)malloc(sizeof(struct LineCache));
        LineCacheInit(ptr);
        ptr->readCache = (float*)malloc(sizeof(float) * lcSourcePool->readCacheLength);
        ptr->index = lcSourcePool->counter++;
    }
    return ptr;
}

void LCSourcePoolAdd(struct LCSourcePool* lcSourcePool, struct LineCache* lineCache){
    ListPut(&lcSourcePool->idle, lineCache);
}

#endif //SZ_LINECACHE_H
