#include "FirstPageInfo.h"
#include "utils.h"

FirstPageInfo::FirstPageInfo(int fileID, BufPageManager *bufPageManager)
        : PageInfo(fileID, 0, bufPageManager) {
}

void FirstPageInfo::init(int recordSize) {
    int slotNumber = (PAGE_SIZE - PAGE_HEADER_SIZE) / (recordSize + 1);
    ((int *) (pHeader + kFileIDOffset))[0] = fileID;
    ((int *) (pHeader + kRecordSizeOffset))[0] = recordSize;
    ((int *) (pHeader + kSlotNumberOffset))[0] = slotNumber;
    ((int *) (pHeader + kPageNumberOffset))[0] = 1;
    ((int *) (pHeader + kRecordNumberOffset))[0] = 0;
    pData = pHeader + kFreePageFlagsOffset;
    bufPageManager->markDirty(index);
}

int FirstPageInfo::getRecordSize() const {
    bufPageManager->access(index);
    return ((int *) (pHeader + kRecordSizeOffset))[0];
}

int FirstPageInfo::getSlotNumber() const {
    bufPageManager->access(index);
    return ((int *) (pHeader + kSlotNumberOffset))[0];
}

int FirstPageInfo::getPageNumber() const {
    bufPageManager->access(index);
    return ((int *) (pHeader + kPageNumberOffset))[0];
}

void FirstPageInfo::increaseRecordNumber() {
    ((int *) (pHeader + kRecordNumberOffset))[0] += 1;
    bufPageManager->markDirty(index);
}

void FirstPageInfo::decreaseRecordNumber() {
    ((int *) (pHeader + kRecordNumberOffset))[0] -= 1;
    bufPageManager->markDirty(index);
}

bool FirstPageInfo::isFull(int pageID) {
    RMBitMap bitMap(pData, getPageNumber() + 1);
    bufPageManager->access(index);
    return bool(bitMap.get(pageID));
}

void FirstPageInfo::setFull(int pageID, bool full) {
    RMBitMap bitMap(pData, getPageNumber() + 1);
    bitMap.set(pageID, full);
    bufPageManager->markDirty(index);
}

RETVAL FirstPageInfo::extendPage(int &pageID) {
    // TODO: handle RETVAL
    pageID = getPageNumber();
    ((int *) (pHeader + kPageNumberOffset))[0] += 1;
    RMBitMap bitMap(pData, MAX_PAGE_NUMBER);
    bitMap.set(pageID, 0);
    bufPageManager->markDirty(index);

    PageInfo pageHandle(fileID, pageID, bufPageManager);
    pageHandle.init(getRecordSize(), getSlotNumber());

    return 0;
}

