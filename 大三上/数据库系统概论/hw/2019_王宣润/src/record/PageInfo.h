#ifndef DATABASE_PAGEHANDLE_H
#define DATABASE_PAGEHANDLE_H


#include "RecordID.h"
#include "Record.h"
#include "../filesystem/bufmanager/BufPageManager.h"

class PageInfo {
public:
    PageInfo(int fileID, int pageID, BufPageManager *bufPageManager);

    RETVAL init(int recordSize, int slotNumber);

    RETVAL getRecord(int slotID, Record &record);

    RETVAL insertRecord(const char *pData, RecordID &rid);

    RETVAL deleteRecord(int slotID);

    RETVAL updateRecord(int slotID, const Record &record);

    // Return
    bool isFull();

protected:
    int index;
    int fileID;
    int pageID;
    BufPageManager *bufPageManager;
    char *pHeader;
    char *pSlot;
    char *pData;

private:
    static const int kPageIDOffset = 0;
    static const int kRecordSizeOffset = kPageIDOffset + 4;
    static const int kSlotNumberOffset = kRecordSizeOffset + 4;
    static const int kSlotBitMapOffset = PAGE_HEADER_SIZE;
    int slotNumber;
    int recordSize;
};


#endif //DATABASE_PAGEHANDLE_H
