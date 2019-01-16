//
// Created by borelset on 2019/1/16.
//

#ifndef SZ_LINECACHE_H
#define SZ_LINECACHE_H

struct LineCache{
    int index;
};

struct LCSourcePool{
    struct List idle;
    int counter = 0;
};

void LCSourcePoolDeinit(struct LCSourcePool* lcSourcePool){
    struct LineCache* lineCache;
    for(int i=0; i<lcSourcePool->idle.count; i++){
        lineCache = (struct LineCache*)ListGet(&lcSourcePool->idle);
        free(lineCache);
    }
    ListDestroy(&lcSourcePool->idle);
}

struct LineCache* LCSourcePoolGet(struct LCSourcePool* lcSourcePool){
    struct LineCache* ptr;
    if(lcSourcePool->idle.count){
        ptr = (struct LineCache*)ListGet(&lcSourcePool->idle);
    }else{
        ptr = new struct LineCache();
        ptr->index = lcSourcePool->counter++;
    }
    return ptr;
}

void LCSourcePoolAdd(struct LCSourcePool* lcSourcePool, struct LineCache* lineCache){
    ListPut(&lcSourcePool->idle, lineCache);
}

#endif //SZ_LINECACHE_H
