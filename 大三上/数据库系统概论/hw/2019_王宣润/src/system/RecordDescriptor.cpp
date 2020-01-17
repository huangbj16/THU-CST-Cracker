#include <string.h>
#include "RecordDescriptor.h"
#include "SystemManager.h"
#include "../record/FileScan.h"

using namespace std;

RecordDescriptor::RecordDescriptor() {

}


AttrValue &RecordDescriptor::operator[](const string &attrName) {
    int i = getIndex(attrName);
    return attrVals[i];
}

void RecordDescriptor::assign(const string &attrName, AttrValue &val) {
    int i = getIndex(attrName);
    attrVals[i] = val;
}

bool RecordDescriptor::checkAssignType(const string &attrName, AttrValue &val) {
    const AttrValue& constValue = attrVals[getIndex(attrName)];
    if(constValue.type == val.type)
        return true;
    if(constValue.type == T_FLOAT && val.type == T_INT)
        return true;
    return false;
}

bool RecordDescriptor::checkCompareType(const string &attrName, AttrValue &val) {
    const AttrValue& constValue = attrVals[getIndex(attrName)];
    if(constValue.type == val.type)
        return true;
    if((constValue.type == T_INT && val.type == T_FLOAT) ||
            (constValue.type == T_FLOAT && val.type == T_INT))
        return true;
    return false;
}

RETVAL RecordDescriptor::toRecord(const RecordID rid, Record& record) {
    int recordSize = 0;
    int attrCount = 0;
    DataAttrInfo* dataAttrInfo = nullptr;
    SystemManager::instance()->fillAttributesFromTable(relName.c_str(), attrCount, dataAttrInfo);
    int nullVectorBase = dataAttrInfo[attrCount-1].offset + dataAttrInfo[attrCount-1].attrLength;
    recordSize = nullVectorBase + attrCount;
    if(attrNames.size() != attrCount) {
        cerr << "[ERROR] InValid RecordDescriptor!" << endl;
        return RETVAL_ERR;
    }
    char* buffer = new char[recordSize];
    for(int i = 0; i < attrCount; ++i) {
        int offset = dataAttrInfo[i].offset;
        if(attrVals[i].type == T_INT) {
            memcpy(buffer + offset, (void*)&(attrVals[i].i), sizeof(int));
        }
        else if(attrVals[i].type == T_FLOAT) {
            memcpy(buffer + offset, (void*)&(attrVals[i].f), 4);
        }
        else
        {
            if(attrVals[i].s.size() < dataAttrInfo[i].attrLength) {
                strncpy(buffer + offset, attrVals[i].s.c_str(), dataAttrInfo[i].attrLength);
            }
            else {
                memcpy(buffer + offset, attrVals[i].s.c_str(), dataAttrInfo[i].attrLength);
            }
        }
    }
    for(int i = 0; i < attrCount; ++i) {
        if(attrVals[i].isNull) {
            memset(buffer + nullVectorBase + i, 1, 1);
        }
        else {
            memset(buffer + nullVectorBase + i, 0, 1);
        }
    }
    record =  Record(rid, buffer, recordSize);
    delete [] dataAttrInfo;
    delete [] buffer;
    return RETVAL_OK ;
}


int RecordDescriptor::getIndex(string attrName) {
    int i = 0;
    for(; i < attrNames.size(); ++i) {
        if(attrNames[i] == attrName)
            break;
    }
    return i;
}

RecordDescriptor RecordDescriptor::createRecordDescriptor(const std::string &relName,
                                                          vector<AttrValue> vals, RETVAL &rc) {
    RecordDescriptor recordDescriptor;
    recordDescriptor.relName = relName;
    int attrCount = 0;
    DataAttrInfo* dataAttrInfo = nullptr;
    SystemManager::instance()->fillAttributesFromTable(relName.c_str(), attrCount, dataAttrInfo);
    if(attrCount != vals.size()) {
        cerr << "[ERROR] Relation <" << relName << "> has " << attrCount << " attributes, but you only provided " << vals.size() << "." << endl;
        rc = RETVAL_ERR;
        return recordDescriptor;
    }
    AttrValue primaryValue;
    DataAttrInfo primaryAttrInfo;
    bool hasPrimaryKey = false;
    for(int i = 0; i < attrCount; ++i) {
        if(vals[i].isNull) {
            if(dataAttrInfo[i].notNull) {
                // Check NULL
                cerr << "[ERROR] Insert NULL into NOT NULL attribute." << endl;
                rc = RETVAL_ERR;
                delete[] dataAttrInfo;
                return recordDescriptor;
            }
            else {
                recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
                vals[i].type = dataAttrInfo[i].attrType;
                recordDescriptor.attrVals.push_back(vals[i]);
            }
        }
        else if((dataAttrInfo[i].attrType == vals[i].type)
           || (dataAttrInfo[i].attrType == T_FLOAT && vals[i].type == T_INT)) {
            if(dataAttrInfo[i].isPrimaryKey) {
                // Check Primary Key
                hasPrimaryKey = true;
                primaryValue = vals[i];
                primaryAttrInfo = dataAttrInfo[i];
            }

            recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
            recordDescriptor.attrVals.push_back(vals[i]);
        }
        else {
            cerr << "[ERROR] Input type is invalid." << endl;
            rc = RETVAL_ERR;
            delete[] dataAttrInfo;
            return recordDescriptor;
        }
    }

    // Check Primary Key
    // if(hasPrimaryKey) {
    //     void* data;
    //     if(primaryValue.type == T_INT)
    //         data = (void*)&(primaryValue.i);
    //     else if(primaryValue.type == T_FLOAT)
    //         data = (void*)&(primaryValue.f);
    //     else
    //         data = (void*)(primaryValue.s.c_str());
    //     SingleFileHandler *fileHandle = FileHandler::instance()->openFile(relName.c_str());
    //     FileScan fileScan;
    //     fileScan.openScan(*fileHandle,
    //                       primaryAttrInfo.attrType,
    //                       primaryAttrInfo.attrLength,
    //                       primaryAttrInfo.offset,
    //                       CmpOP::T_EQ, data);
    //     Record record;
    //     rc = fileScan.getNextRec(record);
    //     if(rc != RETVAL_EOF) {
    //         cerr << "[ERROR] Primary Key Duplicate!" << endl;
    //         rc = RETVAL_ERR;
    //     }
    //     else
    //         rc = RETVAL_OK;
    // }
    // else
    //     rc = RETVAL_OK;
    rc = RETVAL_OK;
    delete[] dataAttrInfo;
    return recordDescriptor;
}

RecordDescriptor RecordDescriptor::createRecordDescriptor(const std::string &relName,
                                                        vector<std::string> &attrs,
                                                        vector<AttrValue> vals, RETVAL &rc) {
    RecordDescriptor recordDescriptor;
    if (attrs.size() != vals.size()){
        cerr << "[ERROR] The length of insert attrs should equal to the length of insert vals." << endl;
        rc = RETVAL_ERR;
        return recordDescriptor;
    }
    recordDescriptor.relName = relName;
    int attrCount = 0;
    DataAttrInfo* dataAttrInfo = nullptr;
    SystemManager::instance()->fillAttributesFromTable(relName.c_str(), attrCount, dataAttrInfo);

    int givenAttrCount = attrs.size();

    // 先确保attrs里的列都在原表中出现过
    for (int j = 0; j < givenAttrCount; ++j){
        int idx = -1;
        for (int i = 0; i < attrCount; ++i)
            if (strcmp(dataAttrInfo[i].attrName, attrs[j].c_str()) == 0){
                idx = j;
                break;
            }
        if (idx == -1){
            rc = RETVAL_ERR;
            cerr << "[ERROR] Attribute <" << attrs[j] << "> not found in relation." << endl;
            delete[] dataAttrInfo;
            return recordDescriptor;
        }
    }

    for(int i = 0; i < attrCount; ++i) {    // i是原表的列下标
        int idx = -1;
        for (int j = 0; j < givenAttrCount; ++j)    // j是给定attr的下标
            if (strcmp(dataAttrInfo[i].attrName, attrs[j].c_str()) == 0){
                idx = j;
                break;
            }
        if (idx == -1){ // 这说明原表的列在attrs里没找到
            if (!dataAttrInfo[i].isDefault){    // 首先查看default
                if (dataAttrInfo[i].notNull){   // 限制不能为空
                    rc = RETVAL_ERR;
                    cerr << "[ERROR] Attribute <" << dataAttrInfo[i].attrName << "> has no default value and cannot be NULL, but you leave it blank." << endl;
                    delete[] dataAttrInfo;
                    return recordDescriptor;
                }else{  // 填入NULL
                    AttrValue val;
                    val.isNull = true;
                    val.type = dataAttrInfo[i].attrType;
                    recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
                    recordDescriptor.attrVals.push_back(val);
                }
            }else{  // 从default中提取
                AttrValue val;
                val.type = dataAttrInfo[i].attrType;
                unsigned int magic = *((unsigned int*)dataAttrInfo[i].defaultVal);
                if (magic == NULL_MAGIC_NUMBER)
                    val.isNull = true;
                else{
                    switch (val.type) {
                        case T_INT:
                            val.i = *((int*)dataAttrInfo[i].defaultVal);
                            break;
                        case T_FLOAT:
                            val.f = *((float*)dataAttrInfo[i].defaultVal);
                            break;
                        case T_STRING:
                            val.s = std::string(dataAttrInfo[i].defaultVal);
                            break;
                        case T_DATE:
                            val.s = std::string(dataAttrInfo[i].defaultVal);
                            break;
                    }
                    val.isNull = false;
                }

                recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
                recordDescriptor.attrVals.push_back(val);
            }
        }else{  // 找到了，采用对应列的值
            if(vals[idx].isNull) {
                if(dataAttrInfo[i].notNull) {
                    // Check NULL
                    cerr << "[ERROR] Insert NULL into NOT NULL attribute!" << endl;
                    rc = RETVAL_ERR;
                    delete[] dataAttrInfo;
                    return recordDescriptor;
                }
                else {
                    recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
                    vals[idx].type = dataAttrInfo[i].attrType;
                    recordDescriptor.attrVals.push_back(vals[idx]);
                }
            }
            else if((dataAttrInfo[i].attrType == vals[idx].type)
            || (dataAttrInfo[i].attrType == T_FLOAT && vals[idx].type == T_INT)) {
                recordDescriptor.attrNames.push_back(dataAttrInfo[i].attrName);
                recordDescriptor.attrVals.push_back(vals[idx]);
            }
            else {
                cerr << "[ERROR] Input type is invalid." << endl;
                rc = RETVAL_ERR;
                delete[] dataAttrInfo;
                return recordDescriptor;
            }
        }
    }
    rc = RETVAL_OK;
    delete[] dataAttrInfo;
    return recordDescriptor;
}

// Filter Attributes to create a new RecordDescriptor
RecordDescriptor RecordDescriptor::filteredByAttributeName(std::vector<AttributeTree::AttributeDescriptor> attrs, bool checkRel) const{
    RecordDescriptor recordDescriptor;
    recordDescriptor.relName = relName;
    // TODO: Check this Seqence
    for (const auto &attr : attrs) {
        for (int i = 0; i < attrNames.size(); ++i) {
            if (checkRel)
                if (attr.relName != relName)
                    continue;
            if(attrNames[i] == attr.attrName) {
                recordDescriptor.attrNames.push_back(attr.attrName);
                recordDescriptor.attrVals.push_back(attrVals[i]);
            }
        }
    }
    return recordDescriptor;
}

// Use this function ONLY to Print something. AttrNames and Relname are not filled by this function
RecordDescriptor RecordDescriptor::concatenate(const RecordDescriptor &next) const{
    RecordDescriptor recordDescriptor;
    for(const auto& attr : attrVals)
        recordDescriptor.attrVals.push_back(attr);
    for(const auto& attr : next.attrVals)
        recordDescriptor.attrVals.push_back(attr);
    return recordDescriptor;
}
