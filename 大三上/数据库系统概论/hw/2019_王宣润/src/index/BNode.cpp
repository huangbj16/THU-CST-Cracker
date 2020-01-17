#include <cstring>
#include "BNode.h"
#include <algorithm>
#include <string.h>

const RecordID BNode::kInfoRid = RecordID(1, 0);
const RecordID BNode::kRootRid = RecordID(1, 1);
char BNode::buffer[MAX_RECORD_SIZE];

using std::string;
using std::find;

BNode::BNode() {
    isLeaf = false;
    parent = RecordID();
}

BNode::BNode(char *pData, AttrType attrType, int attrLength) {
    int m = ((int *) (pData + kOffsetM))[0];
    isLeaf = ((int *) (pData + kOffsetIsLeaf))[0];
    parent = ((RecordID *) (pData + kOffsetParent))[0];

    RecordID *rbuf = (RecordID *) (pData + kOffsetKids);
    for (int i = 0; i < m; ++i)
        kids.push_back(rbuf[i]);
    switch (attrType) {
        case T_INT: {
            int *ibuf = (int *) (pData + kOffsetKeys());
            for (int i = 0; i < m; ++i)
                keys.push_back(AttrValue{T_INT, ibuf[i], 0, ""});
            break;
        }
        case T_FLOAT: {
            float *fbuf = (float *) (pData + kOffsetKeys());
            for (int i = 0; i < m; ++i)
                keys.push_back(AttrValue{T_FLOAT, 0, fbuf[i], ""});
            break;
        }
        case T_STRING: {
            char *cbuf = pData + kOffsetKeys();
            for (int i = 0; i < m; ++i) {
                keys.push_back(AttrValue{T_STRING, 0, 0, string(cbuf, attrLength)});
                cbuf += attrLength;
            }
        }
    }
}

std::string BNode::toString(AttrType attrType, int attrLength) const {
    ((int *) (buffer + kOffsetM))[0] = size();
    ((int *) (buffer + kOffsetIsLeaf))[0] = isLeaf;
    ((RecordID *) (buffer + kOffsetParent))[0] = parent;

    RecordID *rbuf = (RecordID *)(buffer + kOffsetKids);
    for (int i = 0; i < kids.size(); ++i)
        rbuf[i] = kids[i];

    switch (attrType) {
        case T_INT: {
            int *ibuf = (int *) (buffer + kOffsetKeys());
            for (int i = 0; i < keys.size(); ++i)
                ibuf[i] = keys[i].i;
            break;
        }
        case T_FLOAT: {
            float *fbuf = (float *) (buffer + kOffsetKeys());
            for (int i = 0; i < keys.size(); ++i)
                fbuf[i] = keys[i].f;
            break;
        }
        case T_STRING: {
            char *cbuf = (char *) (buffer + kOffsetKeys());
            for (int i = 0; i < keys.size(); ++i) {
                memcpy(cbuf, keys[i].s.c_str(), (size_t) attrLength);
                cbuf += attrLength;
            }
        }
    }
    return string(buffer, (unsigned long) NodeSize(attrLength));
}

int BNode::NodeSize(int attrLength) {
    return int(attrLength * kM + sizeof(RecordID) * (kM + 1) + sizeof(int) * 2);
}

int BNode::kOffsetKeys() const {
    return (int) (kOffsetKids + kids.size() * sizeof(RecordID));
}

int BNode::size() const {
    assert(keys.size() == kids.size());
    return (int) keys.size();
}

int BNode::rank(const RecordID &rid) const {
    return (int) (find(kids.begin(), kids.end(), rid) - kids.begin());
}
