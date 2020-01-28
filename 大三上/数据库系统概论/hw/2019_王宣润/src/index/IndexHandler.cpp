#include "IndexHandler.h"
#include "../record/FileScan.h"
#include "BNode.h"

#include <string>

IndexHandler *IndexHandler::indexManager = nullptr;

IndexHandler::IndexHandler() {
}

IndexHandler::~IndexHandler() {
}

IndexHandler *IndexHandler::instance() {
    if (indexManager == nullptr) {
        indexManager = new IndexHandler();
        indexManager->recordManager = FileHandler::instance();
    }
    return indexManager;
}

RETVAL IndexHandler::CreateIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    string indexfile = string(fileName) + "_idx_" + to_string(indexNo);
    int indexSize = BNode::NodeSize(attrLength);
    recordManager->createFile(indexfile.c_str(), indexSize);

    // Insert Index Info
    SingleFileHandler *fileHandle = recordManager->openFile(indexfile.c_str());
    int indexInfo[2]{(int)attrType, attrLength};
    RecordID rid;
    fileHandle->insertRecord((char *) indexInfo, rid);                          // Ensure that rid = BNode::kInfoRid

    // Insert Root
    BNode root = BNode();
    root.isLeaf = true;
    fileHandle->insertRecord(root.toString(attrType, attrLength).c_str(), rid); // Ensure that
    return 0;
}

RETVAL IndexHandler::DestroyIndex(const char *fileName, int indexNo) {
    // Assume that SM already remove info about index in sysrel
    recordManager->destroyFile((string(fileName) + "_idx_" + to_string(indexNo)).c_str());
    return 0;
}

RETVAL IndexHandler::OpenIndex(const char *fileName, int indexNo, SingleIndexHandler &indexHandle) {
    string indexfile = string(fileName) + "_idx_" + to_string(indexNo);
    indexHandle.OpenIndex(indexfile.c_str());
    return 0;
}

RETVAL IndexHandler::CloseIndex(SingleIndexHandler &indexHandle) {
    indexHandle.ForcePages();
    return 0;
}
