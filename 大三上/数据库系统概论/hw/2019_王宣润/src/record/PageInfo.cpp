#include <cstring>
#include "PageInfo.h"
#include "utils.h"

PageInfo::PageInfo(int fileID, int pageID, BufPageManager *bufPageManager) {
    this->fileID = fileID;
    this->pageID = pageID;
    this->bufPageManager = bufPageManager;
    char *buf = bufPageManager->getPage(fileID, pageID, this->index);
    pHeader = buf;
    this->recordSize = ((int *) (pHeader + kRecordSizeOffset))[0];
    this->slotNumber = ((int *) (pHeader + kSlotNumberOffset))[0];
    pSlot = pHeader + PAGE_HEADER_SIZE;
    pData = pSlot + (slotNumber + 3) / 4;
}

RETVAL PageInfo::init(int recordSize, int slotNumber) {
    ((int *) (pHeader + kPageIDOffset))[0] = pageID;
    ((int *) (pHeader + kRecordSizeOffset))[0] = recordSize;
    ((int *) (pHeader + kSlotNumberOffset))[0] = slotNumber;
    this->recordSize = recordSize;
    this->slotNumber = slotNumber;
    pData = pSlot + (slotNumber + 3) / 4;
    RMBitMap bitMap(pSlot, slotNumber);
    for (int i = 0; i < slotNumber; ++i)
        bitMap.set(i, 0);
    bufPageManager->markDirty(index);
    return RETVAL_OK;
}

RETVAL PageInfo::getRecord(int slotID, Record &record) {
    RMBitMap bitMap(pSlot, slotNumber);
    if (bitMap.get(slotID) == 0)
        return RETVAL_NOREC;
    RecordID rid(pageID, slotID);
    record = Record(rid, pData + slotID * recordSize, recordSize);
    bufPageManager->access(index);
    return RETVAL_OK;
}

RETVAL PageInfo::insertRecord(const char *pData, RecordID &rid) {
    RMBitMap bitMap(pSlot, slotNumber);
    for (int i = 0; i < slotNumber; ++i)
        if (bitMap.get(i) == 0) {
            memcpy(this->pData + i * recordSize, pData, recordSize);
            rid = RecordID(pageID, i);
            bitMap.set(i, 1);
            bufPageManager->markDirty(index);
            return RETVAL_OK;
        }
    return RETVAL_ERR;
}

RETVAL PageInfo::deleteRecord(int slotID) {
    RMBitMap bitMap(pSlot, slotNumber);
    if (bitMap.get(slotID) == 0)
        return RETVAL_ERR;
    bitMap.set(slotID, 0);
    bufPageManager->markDirty(index);
    return RETVAL_OK;
}


RETVAL PageInfo::updateRecord(int slotID, const Record &record) {
    RMBitMap bitMap(pSlot, slotNumber);
    if (bitMap.get(slotID) == 0)
        return RETVAL_ERR;
    char *pData = record.getData();
    memcpy(this->pData + slotID * recordSize, pData, recordSize);
    bufPageManager->markDirty(index);
    return RETVAL_OK;
}

bool PageInfo::isFull() {
    RMBitMap bitMap(pSlot, slotNumber);
    for (int i = 0; i < slotNumber; ++i)
        if (bitMap.get(i) == 0)
            return false;
    bufPageManager->access(index);
    return true;
}
