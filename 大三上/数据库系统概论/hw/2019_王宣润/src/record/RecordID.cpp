#include "RecordID.h"

RecordID::RecordID() {
    this->pageID = -1;
    this->slotID = -1;
}

RecordID::~RecordID() {
}

RecordID::RecordID(int pageID, int slotID) {
    this->pageID = pageID;
    this->slotID = slotID;
}

RETVAL RecordID::getSlotID(int &slotID) const {
    if (this->pageID != -1 && this->slotID != -1) {
        slotID = this->slotID;
        return RETVAL_OK;
    } else {
        return RETVAL_ERR;
    }
}

RETVAL RecordID::getPageID(int &pageID) const {
    if (this->pageID != -1 && this->slotID != -1) {
        pageID = this->pageID;
        return RETVAL_OK;
    } else {
        return RETVAL_ERR;
    }
}

bool RecordID::operator <(const RecordID &recordID) const {
    return this->pageID < recordID.pageID || (this->pageID == recordID.pageID && this->slotID < recordID.slotID);
}

bool RecordID::isNull() const {
    return pageID < 0 || slotID < 0;
}

bool RecordID::operator==(const RecordID &recordID) const {
    return this->pageID == recordID.pageID && this->slotID == recordID.slotID;
}
