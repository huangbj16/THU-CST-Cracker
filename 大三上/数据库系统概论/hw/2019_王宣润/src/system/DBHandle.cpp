#include <string.h>
#include <set>
#include <map>
#include "DBHandle.h"
#include "../record/FileHandler.h"
#include "../record/FileScan.h"
#include "Printer.h"
#include "SystemManager.h"

using namespace std;

DBHandle::DBHandle() {

}


// 从系统表中重新获得信息
RETVAL DBHandle::refreshHandle() {
    relations.clear();
    attributes.clear();
    foreignKeys.clear();
    indexes.clear();
    relationRecordIDs.clear();
    attributeRecordIDs.clear();
    foreignKeyRecordIDs.clear();
    indexRecordIDs.clear();
    FileHandler *recordManager = FileHandler::instance();
    SingleFileHandler* fileHandle = recordManager->openFile(kDefaultRelCatName);
    FileScan fileScan;
    RETURNIF(fileScan.openScan(*fileHandle, AttrType::T_INT, sizeof(int), 0, CmpOP::T_NO, nullptr));
    Record record;
    RETVAL rc = RETVAL_OK;
    while(rc != RETVAL_EOF) {
        rc = fileScan.getNextRec(record);
        if(rc == RETVAL_ERR) {
            return rc;
        }
        if(rc == RETVAL_EOF) {
            break;
        }
        DataRelInfo* dataRelInfo = (DataRelInfo *) record.getData();
        relations.push_back(*dataRelInfo);
        relationRecordIDs.push_back(record.getRid());
    }
    RETURNIF(fileScan.closeScan());

    fileHandle = recordManager->openFile(kDefaultAttrCatName);
    RETURNIF(fileScan.openScan(*fileHandle, AttrType::T_INT, sizeof(int), 0, CmpOP::T_NO, nullptr));
    rc = RETVAL_OK;
    while(rc != RETVAL_EOF) {
        rc = fileScan.getNextRec(record);
        if(rc == RETVAL_ERR) {
            return rc;
        }
        if(rc == RETVAL_EOF) {
            break;
        }
        DataAttrInfo* dataAttrInfo = (DataAttrInfo*) record.getData();
        attributes.push_back(*dataAttrInfo);
        attributeRecordIDs.push_back(record.getRid());
    }
    RETURNIF(fileScan.closeScan());

    fileHandle = recordManager->openFile(kDefaultFkCatName);
    RETURNIF(fileScan.openScan(*fileHandle, AttrType::T_INT, sizeof(int), 0, CmpOP::T_NO, nullptr));
    rc = RETVAL_OK;
    while (rc != RETVAL_EOF) {
        rc = fileScan.getNextRec(record);
        if(rc == RETVAL_ERR) {
            return rc;
        }
        if(rc == RETVAL_EOF) {
            break;
        }
        DataFkInfo* dataFkInfo = (DataFkInfo*) record.getData();
        foreignKeys.push_back(*dataFkInfo);
        foreignKeyRecordIDs.push_back(record.getRid());
    }
    RETURNIF(fileScan.closeScan());

    fileHandle = recordManager->openFile(kDefaultIdxCatName);
    RETURNIF(fileScan.openScan(*fileHandle, AttrType::T_INT, sizeof(int), 0, CmpOP::T_NO, nullptr));
    rc = RETVAL_OK;
    while (rc != RETVAL_EOF) {
        rc = fileScan.getNextRec(record);
        if(rc == RETVAL_ERR) {
            return rc;
        }
        if(rc == RETVAL_EOF) {
            break;
        }
        DataIdxInfo* dataIdxInfo = (DataIdxInfo*) record.getData();
        indexes.push_back(*dataIdxInfo);
        indexRecordIDs.push_back(record.getRid());
    }
    RETURNIF(fileScan.closeScan());

    return RETVAL_OK;
}

/*
    创建一个数据表table，其参数如下：
    relName:    表名
    attrCount:  列数
    attributes: 列信息
    其中，列信息是AttrInfo类型
*/
RETVAL DBHandle::createTable(const char *relName, int attrCount, AttrInfo *attributes) {
    FileHandler* recordManager = FileHandler::instance();
    // First Updata System Table
    SingleFileHandler* fileHandle = recordManager->openFile(kDefaultAttrCatName);
    int record_size = 0;
    DataAttrInfo dataAttrInfo;
    ::set<string> attrs;
    RecordID recordID;
    /*
        这个循环用于把所有attr的信息转化为record格式，便于保存
    */
    for(int i = 0; i < attrCount; ++i) {
        const AttrInfo& attrInfo = attributes[i];
        memset(&dataAttrInfo, 0, sizeof(DataAttrInfo));
        dataAttrInfo.attrLength = attrInfo.attrLength;
        dataAttrInfo.attrType = attrInfo.attrType;
        strcpy(dataAttrInfo.attrName, attrInfo.attrName);
        dataAttrInfo.offset = record_size;
        dataAttrInfo.indexNo = 0;
        dataAttrInfo.isPrimaryKey = attrInfo.isPrimaryKey;
        dataAttrInfo.notNull = attrInfo.notNull;
        strcpy(dataAttrInfo.relName, relName);
        dataAttrInfo.isDefault = attrInfo.isDefault;
        if (attrInfo.isDefault){
            if (attrInfo.defaultVal.isNull)
                *((unsigned int*)dataAttrInfo.defaultVal) = NULL_MAGIC_NUMBER;  // 默认值为NULL，则用magicnumber来记载
            else
                switch (attrInfo.attrType){
                    case T_INT:
                        memcpy((void*)dataAttrInfo.defaultVal, (void*)&attrInfo.defaultVal.i, sizeof (int));
                        // printf("Default Value = %d\n", attrInfo.defaultVal.i);
                        break;
                    case T_FLOAT:
                        memcpy((void*)dataAttrInfo.defaultVal, (void*)&attrInfo.defaultVal.f, sizeof (float));
                        break;
                    case T_STRING:
                        strcpy(dataAttrInfo.defaultVal, attrInfo.defaultVal.s.c_str());
                        break;
                    case T_DATE:
                        strcpy(dataAttrInfo.defaultVal, attrInfo.defaultVal.s.c_str());
                        break;
                }
        }
        record_size += attrInfo.attrLength;

        if (attrs.find(string(attrInfo.attrName)) == attrs.end())
        {
            attrs.insert(string(attrInfo.attrName));
        }
        else
        {
            cerr << "[ERROR] Duplicate attribute name." << endl;
            return RETVAL_ERR;
        }
        RETURNIF(fileHandle->insertRecord((const char*)&dataAttrInfo, recordID));
    }

    // Create New File
    RETURNIF(recordManager->createFile(relName, record_size + attrCount));

    DataRelInfo dataRelInfo;
    memset(&dataRelInfo, 0, sizeof(DataRelInfo));
    // Update relcat
    // TODO: Checkout Duplicated RelName
    strcpy(dataRelInfo.relName, relName);
    dataRelInfo.attrCount = attrCount;
    dataRelInfo.recordSize = record_size;
    dataRelInfo.indexCount = 0;
    dataRelInfo.primaryCount = 0;
    for (int i = 0; i < attrCount; ++i)
        if (attributes[i].isPrimaryKey > 0)
            dataRelInfo.primaryCount++;
    fileHandle = recordManager->openFile(kDefaultRelCatName);
    RETURNIF(fileHandle->insertRecord((const char*)&dataRelInfo, recordID));

    refreshHandle();
    return RETVAL_OK;
}

RETVAL DBHandle::dropTable(const char *relName) {
    FileHandler* recordManager = FileHandler::instance();
    // Now Delete Info in Relcat
    RETVAL rc = RETVAL_OK;
    bool found = false;
    int i = 0;
    for(; i < relations.size(); ++i) {
        if(strcmp(relations[i].relName, relName) == 0) {
            found = true;
            break;
        }
    }
    if(!found)
    {
        cerr << "[ERROR] No such table." << endl;
        return RETVAL_ERR;
    }

    RecordID recordID = relationRecordIDs[i];
    SingleFileHandler* fileHandle = recordManager->openFile(kDefaultRelCatName);
    RETURNIF(fileHandle->deleteRecord(recordID));
    RETURNIF(recordManager->destroyFile(relName));

    // Now delete attr in attrCat
    fileHandle = recordManager->openFile(kDefaultAttrCatName);
    for(i = 0; i < attributes.size(); ++i) {
        if(strcmp(attributes[i].relName, relName) == 0) {
            RETURNIF(fileHandle->deleteRecord(attributeRecordIDs[i]));
        }
    }
    refreshHandle();
    return 0;
}

RETVAL DBHandle::createIndex(const char *relName, const char *attrName) {
    return 0;
}

RETVAL DBHandle::dropIndex(const char *relName, const char *attrName) {
    return 0;
}

RETVAL DBHandle::help() {
    RETVAL rc = RETVAL_OK;
    auto records = retrieveRecords(kDefaultRelCatName, rc);
    RETURNIF(rc);
    vector<RecordDescriptor> descriptors;
    for(auto it = records.begin(); it != records.end(); ++it)
        descriptors.push_back(it->second);
    Printer::printAll(descriptors);
    return 0;


//    DataAttrInfo* dataAttrInfo;
//    int attrCount;
//    RETURNIF(fillAttributesFromTable(kDefaultRelCatName, attrCount, dataAttrInfo));
//    DataAttrInfo dataAttrInfoArray[1];
//    for(int i = 0; i < attrCount; ++i)
//    {
//        if(strcmp(dataAttrInfo[i].attrName, "relName") == 0)
//            dataAttrInfoArray[0] = dataAttrInfo[i];
//    }
//
//    Printer printer(dataAttrInfoArray, 1);
//    printer.printHeader(cout);
//
//    // TODO: Change Below to Unified Interface
//    FileScan fileScan;
//    SingleFileHandler* fileHandle = FileHandler::instance()->openFile(kDefaultRelCatName);
//    RETURNIF(fileScan.openScan(*fileHandle, AttrType::T_INT, sizeof(int), 0, CmpOP::T_NO, nullptr));
//    RETVAL rc = RETVAL_OK;
//    Record record;
//    while(rc != RETVAL_EOF)
//    {
//        rc = fileScan.getNextRec(record);
//        if(rc != RETVAL_EOF)
//        {
//            char* data = record.getData();
//            printer.print(data, cout);
//        }
//    }
//    delete []dataAttrInfo;
    //    return RETVAL_OK;
}

vector<vector<string>> DBHandle::qHelp()
{
    RETVAL rc = RETVAL_OK;
    auto records = retrieveRecords(kDefaultRelCatName, rc);
    vector<vector<string>> ret;
    for(auto it = records.begin(); it != records.end(); ++it) {
        vector<string> tmp;
        tmp.push_back(it->second["relName"].s);
        tmp.push_back(to_string(it->second["recordSize"].i));
        tmp.push_back(to_string(it->second["attrCount"].i));
        tmp.push_back(to_string(it->second["indexCount"].i));
        ret.push_back(tmp);
    }
    return ret;
}

RETVAL DBHandle::help(const char *relName) {
    RETVAL rc = RETVAL_OK;
    auto records = retrieveRecords(kDefaultAttrCatName, rc);
    RETURNIF(rc);
    vector<RecordDescriptor> descriptors;
    for(auto it = records.begin(); it != records.end(); ++it) {
        if(it->second["relName"].s == string(relName))
            descriptors.push_back(it->second);
    }
    if(descriptors.empty()) {
        cerr << "[ERROR] No Such Relation: '" << relName << "'!" << endl;
        return 0;
    }
    Printer::printAll(descriptors);
    return 0;

//    DataAttrInfo* dataAttrInfo;
//    int attrCount;
//    int attrLength;
//    int offset;
//
//    RETURNIF(fillAttributesFromTable(kDefaultAttrCatName, attrCount, dataAttrInfo));
//
//    for(int i = 0; i < attrCount; ++i)
//    {
//        if(strcmp(dataAttrInfo[i].attrName, "relName") == 0)
//        {
//            attrLength = dataAttrInfo[i].attrLength;
//            offset = dataAttrInfo[i].offset;
//        }
//    }
//
//    Printer printer(dataAttrInfo, attrCount);
//    printer.printHeader(cout);
//
//    // TODO: Change Below to Unified Interface
//    FileScan fileScan;
//    SingleFileHandler* fileHandle = FileHandler::instance()->openFile(kDefaultAttrCatName);
//    RETURNIF(fileScan.openScan(*fileHandle, AttrType::T_STRING, attrLength, offset,
//                               CmpOP::T_EQ, (void*)relName));
//    RETVAL rc = RETVAL_OK;
//    Record record;
//    while(rc != RETVAL_EOF)
//    {
//        rc = fileScan.getNextRec(record);
//        if(rc != RETVAL_EOF)
//        {
//            char* data = record.getData();
//            printer.print(data, cout);
//        }
//    }
//
//    delete [] dataAttrInfo;
    //    return 0;
}

std::vector<std::vector<string> > DBHandle::qHelp(const char *relName)
{
    RETVAL rc = RETVAL_OK;
    auto records = retrieveRecords(kDefaultAttrCatName, rc);
    vector<vector<string>> ret;
    for(auto it = records.begin(); it != records.end(); ++it) {
        if(it->second["relName"].s == string(relName)) {
            vector<string> tmp;
            tmp.push_back(it->second["attrName"].s);
            tmp.push_back(to_string(it->second["attrType"].i));
            tmp.push_back(to_string(it->second["attrLength"].i));
            tmp.push_back(to_string(it->second["offset"].i));
            tmp.push_back(to_string(it->second["indexNo"].i));
            tmp.push_back(to_string(it->second["isPrimaryKey"].i));
            tmp.push_back(to_string(it->second["notNull"].i));
            ret.push_back(tmp);
        }
    }
    return ret;
}

RETVAL DBHandle::print(const char *relName) {
    RETVAL rc = RETVAL_OK;
    auto records = retrieveRecords(relName, rc);
    RETURNIF(rc);
    vector<RecordDescriptor> descriptors;
    for(auto it = records.begin(); it != records.end(); ++it)
        descriptors.push_back(it->second);
    Printer::printAll(descriptors);
    return 0;
}

map<RecordID, RecordDescriptor> DBHandle::qPrint(const char *relName)
{
    RETVAL rc = RETVAL_OK;
    return retrieveRecords(relName, rc);
}


DBHandle::~DBHandle() {

}

/**
 * Remember to delete the DataAttrInfo Array after calling this function
 * 在dataAttrInfo 中填入relName的表中的Attr项
 */
RETVAL DBHandle::fillAttributesFromTable(const char *relName, int &attrCount, DataAttrInfo *&dataAttrInfo) {
    int i = 0;
    for(; i < relations.size(); ++i) {
        if(strcmp(relations[i].relName, relName) == 0) {
            break;
        }
    }
    if(i == relations.size()) {
        cerr << "[ERROR] No Such Table" << endl;
        return RETVAL_ERR;
    }
    attrCount = relations[i].attrCount;
    dataAttrInfo = new DataAttrInfo[attrCount];
    // cerr << "Relations.size = " << relations.size() << endl;
    // cerr << "Attr Count = " << attrCount << endl;
    int j = 0;
    for(i = 0; i < attributes.size(); ++i) {
        // cerr << "Attr Name = " << attributes[i].attrName << endl;
        if(strcmp(attributes[i].relName, relName) == 0) {
            dataAttrInfo[j++] = attributes[i];
        }
    }
    assert(j == attrCount);
    return RETVAL_OK;
}

bool DBHandle::hasRelation(const char *relName) {
    for(int i = 0; i < relations.size(); ++i) {
        if(strcmp(relations[i].relName, relName) == 0) {
            return true;
        }
    }
    return false;
}

bool DBHandle::hasAttribute(const char *relName, const char *attrName) {
    for(int i = 0; i < attributes.size(); ++i) {
        if(strcmp(attributes[i].attrName, attrName) == 0 && strcmp(attributes[i].relName, relName) == 0) {
            return true;
        }
    }
    return false;
}

map<RecordID, RecordDescriptor> DBHandle::retrieveRecords(string relName, RETVAL &rc) {
    map<RecordID, RecordDescriptor> descriptors;

    // First Get this Relation's All Attribute Size and Offset
    DataAttrInfo* dataAttrInfo;
    int attrCount;
    rc = fillAttributesFromTable(relName.c_str(), attrCount, dataAttrInfo);
    if(rc != RETVAL_OK) {
        cerr << "[ERROR] Invalid relName" << endl;
        return descriptors;
    }

    // Next Get all the records
    SingleFileHandler* fileHandle = FileHandler::instance()->openFile(relName.c_str());
    FileScan fileScan;
    rc = fileScan.openScan(*fileHandle);
    if(rc != RETVAL_OK) {
        cerr << "error" << endl;
        return descriptors;
    }
    // Iterate throughout all the records
    Record record;
    RecordID recordID;
    const char *pData;
    while(rc == RETVAL_OK) {
        RecordDescriptor recordDescriptor;
        rc = fileScan.getNextRec(record);
        if (rc == RETVAL_EOF) {
            break;
        }
        recordID = record.getRid();
        pData = record.getData();
        // Iterate throughout all the attributes
        for(int i = 0; i < attrCount; ++i) {
            AttrValue descriptor;
            int offset = dataAttrInfo[i].offset;

            AttrType attrType = dataAttrInfo[i].attrType;
            if(attrType == T_INT) {
                int data = ((int*)(pData+offset))[0];
                descriptor.i = data;
            } else if(attrType == T_FLOAT) {
                float data = ((float*)(pData+offset))[0];
                descriptor.f = data;
            } else {
                int attrLength = dataAttrInfo[i].attrLength;
                char data[attrLength];
                memset(data, 0, attrLength);
                memcpy(data, pData + offset, attrLength);
                descriptor.s = string(data);
            }
            descriptor.type = attrType;
            recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
            recordDescriptor.attrVals.push_back(descriptor);
        }
        int nullVectorBase = dataAttrInfo[attrCount-1].offset + dataAttrInfo[attrCount-1].attrLength;
        for(int i = 0; i < attrCount; ++i) {
            char t;
            memcpy((void*)&t, pData + nullVectorBase + i, 1);
            if(t == 1 && relName != string(kDefaultAttrCatName) && relName != string(kDefaultRelCatName))
                recordDescriptor.attrVals[i].isNull = true;
            else
                recordDescriptor.attrVals[i].isNull = false;
        }

        recordDescriptor.relName = relName;
        descriptors[recordID] = recordDescriptor;
    }
    rc = RETVAL_OK;
    delete [] dataAttrInfo;
    return descriptors;
}

RecordDescriptor DBHandle::retrieveOneRecord(std::string relName, const RecordID& recordID, RETVAL &rc) {
    // First Get this Relation's All Attribute Size and Offset
    DataAttrInfo* dataAttrInfo;
    int attrCount;
    rc = fillAttributesFromTable(relName.c_str(), attrCount, dataAttrInfo);
    if(rc != RETVAL_OK) {
        cerr << "[ERROR] Invalid relName" << endl;
        return RecordDescriptor();
    }
    RecordDescriptor recordDescriptor;

    SingleFileHandler* fileHandle = FileHandler::instance()->openFile(relName.c_str());
    Record record;
    fileHandle->getRecord(recordID, record);
    const char* pData = record.getData();
    // Iterate throughout all the attributes
    for(int i = 0; i < attrCount; ++i) {
        AttrValue descriptor;
        int offset = dataAttrInfo[i].offset;

        AttrType attrType = dataAttrInfo[i].attrType;
        if(attrType == T_INT) {
            int data = ((int*)(pData+offset))[0];
            descriptor.i = data;
        } else if(attrType == T_FLOAT) {
            float data = ((float*)(pData+offset))[0];
            descriptor.f = data;
        } else {
            int attrLength = dataAttrInfo[i].attrLength;
            char data[attrLength];
            memset(data, 0, attrLength);
            memcpy(data, pData + offset, attrLength);
            descriptor.s = string(data);
        }
        descriptor.type = attrType;
        recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
        recordDescriptor.attrVals.push_back(descriptor);
    }
    int nullVectorBase = dataAttrInfo[attrCount-1].offset + dataAttrInfo[attrCount-1].attrLength;
    for(int i = 0; i < attrCount; ++i) {
        char t;
        memcpy((void*)&t, pData + nullVectorBase + i, 1);
        if(t == 1)
            recordDescriptor.attrVals[i].isNull = true;
        else
            recordDescriptor.attrVals[i].isNull = false;
    }
    recordDescriptor.relName = relName;
    return recordDescriptor;
}

bool DBHandle::hasIndex(const char *relName, const char *idxName){
    for (int i = 0, lim = indexes.size(); i < lim; ++i)
        if (strcmp(relName, indexes[i].relName) == 0 && strcmp(idxName, indexes[i].idxName) == 0)
            return true;
    return false;
}