#include "SingleFileHandler.h"
#include "RecordID.h"

SingleFileHandler::SingleFileHandler() {
    this->isOpened = false;
}

SingleFileHandler::SingleFileHandler(int fileID, BufPageManager *bufPageManager) {
    this->firstPageHandle = new FirstPageInfo(fileID, bufPageManager);
    this->isOpened = true;
    this->fileID = fileID;
    this->bufPageManager = bufPageManager;

    this->recordSize = firstPageHandle->getRecordSize();
    this->slotNumber = firstPageHandle->getSlotNumber();

    int pageNumber = firstPageHandle->getPageNumber();
    for (int i = 1; i < pageNumber; ++i)
        if (!firstPageHandle->isFull(i)) {
            freePageStack.push(i);
            freePageFlags.push_back(true);
        } else {
            freePageFlags.push_back(false);
        }
}

SingleFileHandler &SingleFileHandler::operator=(SingleFileHandler &&fileHandle) {
    this->firstPageHandle = fileHandle.firstPageHandle;
    this->isOpened = fileHandle.isOpened;
    this->fileID = fileHandle.fileID;
    this->recordSize = fileHandle.recordSize;
    this->slotNumber = fileHandle.slotNumber;
    this->freePageFlags = std::move(fileHandle.freePageFlags);
    this->freePageStack = std::move(fileHandle.freePageStack);
    this->bufPageManager = fileHandle.bufPageManager;
    fileHandle.firstPageHandle = nullptr;
    return *this;
}

SingleFileHandler::~SingleFileHandler() {
    delete firstPageHandle;
}

RETVAL SingleFileHandler::getRecord(const RecordID &rid, Record &record) const {
    if (!isOpened)
        return RETVAL_ERR;

    int pageID, slotID;
    RETURNIF(rid.getPageID(pageID));
    RETURNIF(rid.getSlotID(slotID));

    PageInfo pageHandle = PageInfo(fileID, pageID, bufPageManager);
    return pageHandle.getRecord(slotID, record);
}

RETVAL SingleFileHandler::insertRecord(const char *pData, RecordID &rid) {
    if (!isOpened)
        return RETVAL_ERR;

    if (freePageStack.empty()) {
        // Overflow
        int pageNumber = firstPageHandle->getPageNumber();
        if (pageNumber >= MAX_PAGE_NUMBER) {
            printf("No free pages to insert a record\n");
            return RETVAL_ERR;
        }

        // Append a new page
        int pageID;
        RETURNIF(firstPageHandle->extendPage(pageID));
        freePageStack.push(pageID);
        freePageFlags.push_back(true);
    }

    // Insert Record
    int pageID = freePageStack.top();
    PageInfo pageHandle = PageInfo(fileID, pageID, bufPageManager);
    pageHandle.insertRecord(pData, rid);
    firstPageHandle->increaseRecordNumber();

    if (pageHandle.isFull()) {
        freePageStack.pop();
        freePageFlags[pageID] = false;
        firstPageHandle->setFull(pageID, true);
    }

    return RETVAL_OK;
}

RETVAL SingleFileHandler::deleteRecord(const RecordID &rid) {
    if (!isOpened)
        return RETVAL_ERR;
    int pageID, slotID;
    RETURNIF(rid.getPageID(pageID));
    RETURNIF(rid.getSlotID(slotID));
    PageInfo pageHandle = PageInfo(fileID, pageID, bufPageManager);

    bool pageIsFull = pageHandle.isFull();
    pageHandle.deleteRecord(slotID);
    firstPageHandle->decreaseRecordNumber();

    // When a full page delete one record, put it into free page stack
    if (pageIsFull) {
        freePageStack.push(pageID);
        freePageFlags[pageID] = true;
        firstPageHandle->setFull(pageID, false);
    }

    return RETVAL_OK;
}

RETVAL SingleFileHandler::updateRecord(const Record &record) {
    if (!isOpened)
        return RETVAL_ERR;
    RecordID rid = record.getRid();

    int pageID, slotID;
    RETURNIF(rid.getPageID(pageID));
    RETURNIF(rid.getSlotID(slotID));
    PageInfo pageHandle = PageInfo(fileID, pageID, bufPageManager);
    pageHandle.updateRecord(slotID, record);
    return RETVAL_OK;
}

int SingleFileHandler::getFileID() const {
    return fileID;
}

int SingleFileHandler::getRecordSize() const {
    return recordSize;
}

int SingleFileHandler::getSlotNumber() const {
    return slotNumber;
}

int SingleFileHandler::getPageNumber() const {
    return firstPageHandle->getPageNumber();
}

void SingleFileHandler::release() {
    isOpened = false;
}
