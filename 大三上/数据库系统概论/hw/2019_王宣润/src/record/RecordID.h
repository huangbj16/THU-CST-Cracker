#ifndef DATABASE_RECORDID_H
#define DATABASE_RECORDID_H

#include "../global.h"

class RecordID {
public:
    RecordID();

    ~RecordID();

    RecordID(int pageID, int slotID);

    RETVAL getPageID(int &pageID) const;

    RETVAL getSlotID(int &slotID) const;

    bool isNull() const;
    bool operator <(const RecordID &recordID) const;
    bool operator ==(const RecordID &recordID) const;
private:
    int pageID;
    int slotID;
};

#endif //DATABASE_RECORDID_H
