#ifndef DATABASE_RECORD_H
#define DATABASE_RECORD_H


#include "../global.h"
#include "RecordID.h"
#include "../filesystem/utils/pagedef.h"

class Record {
public:
    Record();

    Record(const RecordID &rid, const char *pData, int recordSize);

    Record & operator=(Record &&record);

    ~Record();

    char * getData() const;

    RecordID getRid() const;

private:
    RecordID rid;
    char *pData;
    int recordSize;


    Record &operator=(const Record &) { return *this; }
};


#endif //DATABASE_RECORD_H
