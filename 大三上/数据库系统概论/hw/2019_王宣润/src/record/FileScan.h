#ifndef DATABASE_FILESCAN_H
#define DATABASE_FILESCAN_H


#include "SingleFileHandler.h"

/*
    在整张表中，扫描对应列(可以为多列的组合)的值为给定value(s)的行
    如果提供完全信息，则用于查找某条确定的记录
    如果不提供任何信息，则用于逐个提取出所有记录
*/
class FileScan {
public:


    FileScan();

    ~FileScan();

    // 开启一个新的扫描，记录其列类型、列长度、列偏移以及比较符
    RETVAL openScan(const SingleFileHandler &fileHandle,
                AttrType attrType,
                int attrLength,
                int attrOffset,
                CmpOP compOP,
                void *value);
    RETVAL openScan(const SingleFileHandler &fileHandle,
                std::vector<AttrType> attrTypes,
                std::vector<int> attrLengths,
                std::vector<int> attrOffsets,
                CmpOP compOP,
                std::vector<void*> values);
    RETVAL openScan(const SingleFileHandler &fileHandle);

    // 在表中遍历每一行，一旦满足条件则放入record
    RETVAL getNextRec(Record &record);
    RETVAL getNextRec(Record &record, RecordID &recordID);

    RETVAL closeScan();

private:
    bool isOpened;
    int attrLength;
    std::vector<int> attrLengths;
    int attrOffset;
    std::vector<int> attrOffsets;
    void *value;
    std::vector<void*> values;
    const SingleFileHandler *fileHandle;
    AttrType attrType;
    std::vector<AttrType> attrTypes;
    CmpOP op;

    int nextPageID;
    int nextSlotID;
    bool multiAttrScan;     // 是否是多列扫描

    bool isValid(Record &record);
};


#endif //DATABASE_FILESCAN_H
