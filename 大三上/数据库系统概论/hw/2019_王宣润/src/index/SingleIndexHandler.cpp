#include "SingleIndexHandler.h"
#include "../record/FileHandler.h"
#include "IndexHandler.h"
#include <algorithm>

using std::string;
using std::find;

void SingleIndexHandler::OpenIndex(const char *filename) {
    fileHandle = FileHandler::instance()->openFile(filename);
    Record record;
    // Retrieve indexInfo
    RETVAL rc = fileHandle->getRecord(BNode::kInfoRid, record);
    if (rc != RETVAL_OK)
        return;
    attrType = (AttrType) ((int *) record.getData())[0];
    attrLength = ((int *) record.getData())[1];
}

SingleIndexHandler::SingleIndexHandler() {
    fileHandle = nullptr;
}

SingleIndexHandler::~SingleIndexHandler() {
}

RETVAL SingleIndexHandler::InsertEntry(char *pData, const RecordID &rid) {
    AttrValue v = toAttrValue(pData);
    search(v, rid, BNode::kRootRid);

    int i;
    for (i = 0; i < hot.size(); ++i)
        if (compare(v, hot.keys[i], CmpOP::T_LE))
            break;

    hot.keys.insert(hot.keys.begin() + i, v);
    hot.kids.insert(hot.kids.begin() + i, rid);
    saveNode(hot, hid);
    updateTree(hid);
    solveOverflow(hot, hid);
    return RETVAL_OK;
}

RETVAL SingleIndexHandler::DeleteEntry(char *pData, const RecordID &rid) {
    AttrValue v = toAttrValue(pData);
    if (search(v, rid, BNode::kRootRid) == RETVAL_NOIDX)
        return RETVAL_ERR;

    for (int i = 0; i < hot.size(); ++i)
        if (compare(v, hot.keys[i], CmpOP::T_EQ) && hot.kids[i] == rid) {
            hot.keys.erase(hot.keys.begin() + i);
            hot.kids.erase(hot.kids.begin() + i);
            saveNode(hot, hid);
            updateTree(hid);
            solveUnderflow(hot, hid);
            break;
        }

    return RETVAL_OK;
}

RETVAL SingleIndexHandler::SearchEntry(char *pData, RecordID &rid) {
    AttrValue v = toAttrValue(pData);
    search(v, rid, BNode::kRootRid);
    for (int i = 0; i < hot.size(); ++i)
        if (compare(v, hot.keys[i], CmpOP::T_EQ)) {
            rid = hot.kids[i];
            return RETVAL_OK;
        }
    return RETVAL_ERR;
}

RETVAL SingleIndexHandler::search(const AttrValue &v, const RecordID &vid, const RecordID &nid) {
    BNode node = getNode(nid);
    // Search in the inner nodes
    if (!node.isLeaf) {
        for (int i = 0; i < node.size(); ++i) {
            if (compare(v, node.keys[i], CmpOP::T_LE)) {
                if (search(v, vid, node.kids[i]) == RETVAL_OK)
                    return RETVAL_OK;
                if (compare(v, node.keys[i], CmpOP::T_LT))
                    return RETVAL_NOIDX;
            }
        }
        search(v, vid, node.kids.back());
    } else {
        hot = node;
        hid = nid;
        for (int i = 0; i < node.size(); ++i)
            if (compare(v, node.keys[i], CmpOP::T_EQ) && vid == node.kids[i])
                return RETVAL_OK;
        return RETVAL_NOIDX;
    }
    return RETVAL_NOIDX;
}

RETVAL SingleIndexHandler::ForcePages() {
    return 0;
}

void SingleIndexHandler::solveOverflow(BNode node, RecordID nid) {
    while (node.size() >= BNode::kM) {
        if (node.size() > BNode::kM) {
            printf("AAA");
        }
        assert(node.size() == BNode::kM);
        // When node == root, move node to another place and generate a new root
        if (nid == BNode::kRootRid) {
            node.parent = BNode::kRootRid;
            nid = saveNode(node);
            updateKids(node, nid);
            BNode root = BNode();
            root.keys.push_back(node.keys.back());
            root.kids.push_back(nid);
            saveNode(root, BNode::kRootRid);
        }

        // s is the axis
        int s = BNode::kM / 2;

        // Split node, initialize u
        BNode u = BNode();
        for (int i = s; i < BNode::kM; i++) {
            u.kids.push_back(node.kids[i]);
            u.keys.push_back(node.keys[i]);
        }
        u.isLeaf = node.isLeaf;
        u.parent = node.parent;
        RecordID uid = saveNode(u);

        // Update node
        for (int i = s; i < BNode::kM; i++) {
            node.kids.pop_back();
            node.keys.pop_back();
        }

        // Change kids' parents
        if (!u.isLeaf)
            updateKids(u, uid);

        // Add one key & one kid into parent
        BNode p = getNode(node.parent);
        int r = p.rank(nid);
        assert(r < p.size());
        p.keys.insert(p.keys.begin() + r, node.keys.back());
        p.kids.insert(p.kids.begin() + r + 1, uid);
        saveNode(p, node.parent);
        saveNode(node, nid);
        nid = node.parent, node = p;
    }
}

void SingleIndexHandler::solveUnderflow(BNode node, RecordID nid) {
    while (node.size() < BNode::kM / 2) {
        if (nid == BNode::kRootRid) {
            if (node.size() == 1 && !node.isLeaf) {
                RecordID kid = node.kids[0];
                BNode knode = getNode(kid);
                knode.parent = node.parent;
                saveNode(knode, BNode::kRootRid);
                updateKids(knode, BNode::kRootRid);
                fileHandle->deleteRecord(kid);
            }
            break;
        }
        BNode p = getNode(node.parent);
        int r = p.rank(nid);
        assert(r < p.size());
        // Borrow one from the left brother
        if (r > 0) {
            BNode lb = getNode(p.kids[r - 1]);
            if (lb.size() > BNode::kM / 2) {
                // pass the last kid of lb onto node
                node.keys.insert(node.keys.begin(), lb.keys.back());
                node.kids.insert(node.kids.begin(), lb.kids.back());
                lb.keys.pop_back();
                lb.kids.pop_back();

                // update parent and kid
                p.keys[r - 1] = lb.keys.back();
                if (!node.isLeaf)
                    updateKid(node.kids[0], nid);

                saveNode(p, node.parent);
                saveNode(lb, p.kids[r - 1]);
                saveNode(node, nid);
                break;
            }
        }
        // Borrow one from the right brother
        if (r < p.kids.size() - 1) {
            BNode rb = getNode(p.kids[r + 1]);
            if (rb.size() > BNode::kM / 2) {
                node.keys.push_back(rb.keys.front());
                node.kids.push_back(rb.kids.front());
                rb.keys.erase(rb.keys.begin());
                rb.kids.erase(rb.kids.begin());

                p.keys[r] = node.keys.back();
                if (!node.isLeaf)
                    updateKid(node.kids.back(), nid);

                saveNode(p, node.parent);
                saveNode(rb, p.kids[r + 1]);
                saveNode(node, nid);
                break;
            }
        }

        if (r > 0) {        // Join with the left brother
            BNode lb = getNode(p.kids[r - 1]);
            node.keys.insert(node.keys.begin(), lb.keys.begin(), lb.keys.end());
            node.kids.insert(node.kids.begin(), lb.kids.begin(), lb.kids.end());
            updateKids(lb, nid);
            fileHandle->deleteRecord(p.kids[r - 1]);

            p.keys.erase(p.keys.begin() + r - 1);
            p.kids.erase(p.kids.begin() + r - 1);

            saveNode(node, nid);
            saveNode(p, node.parent);
            nid = node.parent, node = p;
        } else {            // Join with the right brother
            assert(p.size() > 1);
            BNode rb = getNode(p.kids[r + 1]);
            node.keys.insert(node.keys.end(), rb.keys.begin(), rb.keys.end());
            node.kids.insert(node.kids.end(), rb.kids.begin(), rb.kids.end());
            updateKids(rb, nid);
            fileHandle->deleteRecord(p.kids[r + 1]);

            p.keys.erase(p.keys.begin() + r);
            p.kids.erase(p.kids.begin() + r + 1);


            saveNode(node, nid);
            saveNode(p, node.parent);
            nid = node.parent, node = p;
        }
    }
}

AttrValue SingleIndexHandler::toAttrValue(char *pData) {
    AttrValue val;
    val.type = attrType;
    val.isNull = false;
    switch (attrType) {
        case T_INT: val.i = *((int *) pData); break;
        case T_FLOAT: val.f = *((float *) pData); break;
        case T_STRING: val.s = string(pData, (unsigned long) attrLength);
        case T_DATE: val.s = string(pData, (unsigned long) attrLength);
    }
    return val;
}

BNode SingleIndexHandler::getNode(const RecordID &rid) {
    Record record;
    fileHandle->getRecord(rid, record);
    return BNode(record.getData(), attrType, attrLength);
}

void SingleIndexHandler::saveNode(const BNode &node, const RecordID &rid) {
    Record record(rid, node.toString(attrType, attrLength).c_str(), node.NodeSize(attrLength));
    fileHandle->updateRecord(record);
}

RecordID SingleIndexHandler::saveNode(const BNode &node) {
    RecordID rid;
    fileHandle->insertRecord(node.toString(attrType, attrLength).c_str(), rid);
    return rid;
}

void SingleIndexHandler::updateKids(BNode &node, const RecordID &rid) {
    if (node.isLeaf)
        return;
    for (const auto &kid : node.kids) {
        assert(!kid.isNull());
        BNode c = getNode(kid);
        c.parent = rid;
        saveNode(c, kid);
    }
}

void SingleIndexHandler::updateKid(const RecordID &kid, const RecordID &rid) {
    BNode c = getNode(kid);
    c.parent = rid;
    saveNode(c, kid);
}

void SingleIndexHandler::updateTree(RecordID nid) {
    for (BNode node = getNode(nid); !(nid == BNode::kRootRid); ) {
        BNode p = getNode(node.parent);
        int r = p.rank(nid);
        assert(r < p.size());
        if (p.keys[r] != node.keys.back()) {
            p.keys[r] = node.keys.back();
            saveNode(p, node.parent);
            nid = node.parent;
            node = p;
        } else {
            break;
        }
    }
}

vector<RecordID> SingleIndexHandler::ScanIndex(char *pData, CmpOP op) {
    AttrValue v = toAttrValue(pData);
    vector<RecordID> rids;
    scan(v, op, BNode::kRootRid, rids);
    return rids;
}

RETVAL SingleIndexHandler::scan(const AttrValue &v, CmpOP op, const RecordID &nid, std::vector<RecordID> &rids) {
    BNode node = getNode(nid);
    assert(node.parent.isNull() || node.size() >= BNode::kM / 2);
    if (node.parent.isNull() && node.size() == 0)
        return RETVAL_OK;
    AttrValue rbound = node.keys.back();
    AttrValue lbound = AttrValue{attrType, -0x7FFFFFFF, float(-0x7FFFFFFF), "", false};
    if (!node.parent.isNull()) {
        BNode p = getNode(node.parent);
        int r = p.rank(nid);
        if (r > 0) {
            BNode lb = getNode(p.kids[r - 1]);
            lbound = lb.keys.back();
        }
    }
    switch (op) {
        case T_EQ:
            if (compare(lbound, v, T_GT) || compare(rbound, v, T_LT))
                return RETVAL_OK;
            break;
        case T_LT:
            if (compare(lbound, v, T_GE))
                return RETVAL_OK;
            break;
        case T_LE:
            if (compare(lbound, v, T_GT))
                return RETVAL_OK;
            break;
        case T_GT:
            if (compare(rbound, v, T_LE))
                return RETVAL_OK;
            break;
        case T_GE:
            if (compare(rbound, v, T_LT))
                return RETVAL_OK;
            break;
        default:
            return RETVAL_ERR;
    }
    if (!node.isLeaf) {
        for (int i = 0; i < node.size(); ++i)
            scan(v, op, node.kids[i], rids);
    } else {
        for (int i = 0; i < node.size(); ++i)
            if (compare(node.keys[i], v, op))
                rids.push_back(node.kids[i]);
    }
    return RETVAL_OK;
}

RETVAL SingleIndexHandler::CheckTree() {
    return check(BNode::kRootRid);
}

RETVAL SingleIndexHandler::check(const RecordID &nid) {
    BNode node = getNode(nid);
    // Check parent
    if (node.parent.isNull() == false) {
        BNode p = getNode(node.parent);
        int r = p.rank(nid);
        if (r == p.size()) {
            cerr << "NNN!!!\n";
        }
        assert(r < p.size());
        assert(p.keys[r] == node.keys.back());
    }
    if (!node.isLeaf) {
        for (int i = 0; i < node.size(); ++i)
            check(node.kids[i]);
    }
    return 0;
}
