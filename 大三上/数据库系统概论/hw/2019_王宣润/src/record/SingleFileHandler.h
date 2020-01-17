#ifndef DATABASE_FILEHANDLER_H
#define DATABASE_FILEHANDLER_H


#include "../global.h"
#include "../filesystem/bufmanager/BufPageManager.h"

#include "RecordID.h"
#include "Record.h"
#include "FirstPageInfo.h"

#include <stack>
#include <vector>

class SingleFileHandler {
public:
    SingleFileHandler();

    SingleFileHandler(int fileID, BufPageManager *bufPageManager);
    SingleFileHandler &operator =(SingleFileHandler && fileHandle);
//    SingleFileHandler &operator =(const SingleFileHandler &fileHandle);

    ~SingleFileHandler();

    RETVAL getRecord(const RecordID &rid, Record &record) const;

    RETVAL insertRecord(const char *pData, RecordID &rid);

    RETVAL insertOrUpdate(const char *pData, const RecordID &rid);

    RETVAL deleteRecord(const RecordID &rid);

    RETVAL updateRecord(const Record &record);


    int getFileID() const;

    int getRecordSize() const;

    int getSlotNumber() const;

    int getPageNumber() const;

    void release();

    bool isOpen() {return isOpened;}

private:
    static const int kAllPages = -1;

    bool isOpened;
    int fileID;
    int recordSize;
    int slotNumber;

    BufPageManager *bufPageManager;
    FirstPageInfo* firstPageHandle;
    std::stack<int> freePageStack;
    std::vector<bool> freePageFlags;
};

#endif //DATABASE_FILEHANDLER_H
