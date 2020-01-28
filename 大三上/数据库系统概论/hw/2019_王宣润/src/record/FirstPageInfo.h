#ifndef DATABASE_FIRSTPAGEHANDLE_H
#define DATABASE_FIRSTPAGEHANDLE_H


#include "PageInfo.h"

class FirstPageInfo : PageInfo {
public:
    FirstPageInfo(int fileID, BufPageManager *bufPageManager);

    // Initialize Page[0] when creating a new file
    void init(int recordSize);

    // The following methods changed the meta-data in Page[0]
    int getRecordSize() const;

    int getSlotNumber() const;

    int getPageNumber() const;

    void increaseRecordNumber();

    void decreaseRecordNumber();

    // Return true if Page[pageID] has no free slots
    bool isFull(int pageID);

    void setFull(int pageID, bool full);

    // Extend One Page in file
    RETVAL extendPage(int &pageID);


private:
    static const int kFileIDOffset = 0;
    static const int kRecordSizeOffset = kFileIDOffset + 4;
    static const int kSlotNumberOffset = kRecordSizeOffset + 4;
    static const int kPageNumberOffset = kSlotNumberOffset + 4;
    static const int kRecordNumberOffset = kPageNumberOffset + 4;
    static const int kFreePageFlagsOffset = kRecordNumberOffset + 4;
};


#endif //DATABASE_FIRSTPAGEHANDLE_H
