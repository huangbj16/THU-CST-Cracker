#ifndef DATABASE_INDEXHANDLE_H
#define DATABASE_INDEXHANDLE_H


#include "../record/RecordID.h"
#include "../record/SingleFileHandler.h"
#include "BNode.h"

class SingleIndexHandler {
public:
    SingleIndexHandler();
    ~SingleIndexHandler();
    // 插入一条record，键值为pData，RId为rid
    RETVAL InsertEntry(char *pData, const RecordID &rid);
    // 删除一条record，键值为pData，RId为rid
    RETVAL DeleteEntry(char *pData, const RecordID &rid);
    // 寻找一条record，键值为pData 结果存入rid
    RETVAL SearchEntry(char *pData, RecordID &rid);
    // 无功能
    RETVAL ForcePages();
    // 检查整棵树是否合法
    RETVAL CheckTree();
    // 给定一条record，找出满足 X op record 条件的所有index
    std::vector<RecordID> ScanIndex(char *pData, CmpOP op);

private:
    friend class IndexHandler;

    void OpenIndex(const char *filename);

    RETVAL search(const AttrValue &v, const RecordID &vid, const RecordID &nid);
    RETVAL scan(const AttrValue &v, CmpOP op, const RecordID &nid, std::vector<RecordID> &rids);
    RETVAL check(const RecordID &nid);
    void solveOverflow(BNode node, RecordID rid);
    void solveUnderflow(BNode node, RecordID rid);
    void updateKids(BNode &node, const RecordID &nid);
    void updateKid(const RecordID &kid, const RecordID &nid);
    void updateTree(RecordID nid);

    BNode getNode(const RecordID &rid);
    void saveNode(const BNode &node, const RecordID &rid);
    RecordID saveNode(const BNode &node);

    AttrValue toAttrValue(char *pData);

    SingleFileHandler *fileHandle;
    AttrType attrType;
    int attrLength;
    BNode hot;
    RecordID hid;
};


#endif //DATABASE_INDEXHANDLE_H
