#ifndef DATABASE_BNODE_H
#define DATABASE_BNODE_H


#include <string>
#include <vector>
#include "../record/RecordID.h"

struct BNode {
public:

    std::vector<RecordID> kids;
    std::vector<AttrValue> keys;
    int isLeaf;
    RecordID parent;

    BNode();
    // 将节点从字符串中load出来
    BNode(char *pData, AttrType attrType, int AttrLength);
    // 将节点dump到字符串里，以便保存到record中
    std::string toString(AttrType attrType, int attrLength) const;
    int size() const;
    int rank(const RecordID &rid) const;

    const static int kM = 4;
    const static RecordID kInfoRid;
    const static RecordID kRootRid;
    static const int kOffsetM = 0;
    static const int kOffsetIsLeaf = sizeof(int) + kOffsetM;
    static const int kOffsetParent = sizeof(int) + kOffsetIsLeaf;
    static const int kOffsetKids = sizeof(RecordID) + kOffsetParent;
    int kOffsetKeys() const;
    static int NodeSize(int attrLength);

    static char buffer[MAX_RECORD_SIZE];
};


#endif //DATABASE_BNODE_H
