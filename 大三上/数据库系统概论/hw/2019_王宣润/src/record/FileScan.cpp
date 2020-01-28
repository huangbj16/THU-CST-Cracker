#include <string.h>
#include "FileScan.h"

using std::string;

FileScan::FileScan() {
    isOpened = false;
    fileHandle = nullptr;
    multiAttrScan = false;
}

FileScan::~FileScan() {

}

RETVAL FileScan::openScan(const SingleFileHandler &fileHandle,
                      AttrType attrType,
                      int attrLength,
                      int attrOffset,
                      CmpOP compOP,
                      void *value) {
    if (isOpened)
        return RETVAL_ERR;
    this->isOpened = true;
    this->fileHandle = &fileHandle;
    this->attrType = attrType;
    this->attrLength = attrLength;
    this->attrOffset = attrOffset;
    this->op = compOP;
    this->value = value;
    nextPageID = 1;
    nextSlotID = 0;
    multiAttrScan = false;
    return RETVAL_OK;
}

RETVAL FileScan::openScan(const SingleFileHandler &fileHandle,
            std::vector<AttrType> attrTypes,
            std::vector<int> attrLengths,
            std::vector<int> attrOffsets,
            CmpOP compOP,
            std::vector<void*> values) {
    if (isOpened)
        return RETVAL_ERR;
    int len1 = attrTypes.size();
    int len2 = attrLengths.size();
    int len3 = attrOffsets.size();
    int len4 = values.size();
    if (len1 != len2 || len1 != len3 || len1 != len4)
        return RETVAL_ERR;
    this->isOpened = true;
    this->fileHandle = &fileHandle;
    this->attrTypes = attrTypes;
    this->attrLengths = attrLengths;
    this->attrOffsets = attrOffsets;
    this->op = compOP;
    this->values = values;
    nextPageID = 1;
    nextSlotID = 0;
    multiAttrScan = true;
    return RETVAL_OK;
}

RETVAL FileScan::openScan(const SingleFileHandler &fileHandle) {
    return openScan(fileHandle, AttrType::T_INT , 0, 0, CmpOP::T_NO , nullptr);
}

RETVAL FileScan::getNextRec(Record &record) {
    if (!isOpened)
        return RETVAL_ERR;
    while (nextPageID < fileHandle->getPageNumber()) {
        RecordID rid = RecordID(nextPageID, nextSlotID);
        RETVAL rc = fileHandle->getRecord(rid, record);
        nextSlotID += 1;
        if (nextSlotID == fileHandle->getSlotNumber()) {
            nextPageID += 1;
            nextSlotID = 0;
        }
        if (rc == RETVAL_OK) {
            if (op == CmpOP::T_NO || isValid(record)) {
                return RETVAL_OK;
            } else {
                continue;
            }
        } else if (rc == RETVAL_NOREC) {
            continue;
        } else {
            return RETVAL_ERR;
        }
    };
    isOpened = false;
    return RETVAL_EOF;
}

RETVAL FileScan::closeScan() {
    isOpened = false;
    return RETVAL_OK;
}

bool FileScan::isValid(Record &record) {
    if (op == CmpOP::T_NO)
        return true;
    char *pData = record.getData();
    if (multiAttrScan == false){
        pData += attrOffset;
        if (attrType == AttrType::T_INT) {
            return compare<int>(((int *) pData)[0], ((int *) value)[0], op);
        } else if (attrType == AttrType::T_FLOAT) {
            return compare<float>(((float *) pData)[0], ((float *) value)[0], op);
        } else if (attrType == AttrType::T_STRING) {
            string a = string(pData);
            string b = string((char *) value);
            return compare<string>(a, b, op);
        } else if (attrType == AttrType::T_DATE) {
            string a = string(pData);
            string b = string((char *) value);
            return compare<string>(a, b, op);
        }
        return false;
    }else{
        int len = attrTypes.size();
        bool valid = true;
        for (int i = 0; i < len; ++i){
            char *ppData = pData + attrOffsets[i];
            if (attrTypes[i] == AttrType::T_INT)
                valid = valid && compare<int>(((int*)ppData)[0], ((int*)values[i])[0], op);
            else if (attrTypes[i] == AttrType::T_FLOAT)
                valid = valid && compare<float>(((float*)ppData)[0], ((float*)values[i])[0], op);
            else if (attrTypes[i] == AttrType::T_STRING){
                string a = string(ppData);
                string b = string((char*) values[i]);
                valid = valid && compare<string>(a, b, op);
            } else if (attrTypes[i] == AttrType::T_DATE){
                string a = string(ppData);
                string b = string((char*) values[i]);
                valid = valid && compare<string>(a, b, op);
            }
            if (valid == false)
                return false;
        }
        return valid;
    }
}

