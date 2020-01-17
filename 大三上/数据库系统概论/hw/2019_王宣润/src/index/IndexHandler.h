#ifndef DATABASE_INDEXMANAGER_H
#define DATABASE_INDEXMANAGER_H


#include "../global.h"
#include "SingleIndexHandler.h"
#include "../record/FileHandler.h"

class IndexHandler {
public:
    static IndexHandler *instance();
    ~IndexHandler();

    RETVAL CreateIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength);
    RETVAL DestroyIndex(const char *fileName, int indexNo);
    RETVAL OpenIndex(const char *fileName, int indexNo, SingleIndexHandler &indexHandle);
    RETVAL CloseIndex(SingleIndexHandler &indexHandle);

private:
    static IndexHandler *indexManager;
    FileHandler *recordManager;
    IndexHandler();
};


#endif //DATABASE_INDEXMANAGER_H
