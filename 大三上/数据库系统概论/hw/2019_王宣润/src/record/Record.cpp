#include "Record.h"

#include <cstring>

Record::Record() {
    this->pData = nullptr;
    this->recordSize = -1;
}

Record::Record(const RecordID &rid, const char *pData, int recordSize) {
    this->rid = rid;
    this->pData = new char[recordSize];
    this->recordSize = recordSize;
    memcpy(this->pData, pData, recordSize);
}

Record &Record::operator=(Record &&record) {
    this->rid = record.rid;
    delete this->pData;
    this->pData = record.pData;
    this->recordSize = record.recordSize;
    record.pData = nullptr;
    return *this;
}

Record::~Record() {
    delete pData;
}

char * Record::getData() const {
    return this->pData;
}

RecordID Record::getRid() const {
    return this->rid;
}
