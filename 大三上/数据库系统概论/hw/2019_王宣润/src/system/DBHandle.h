#ifndef DATABASE_DBHANDLE_H
#define DATABASE_DBHANDLE_H

#include <vector>
#include "../global.h"
#include "../record/RecordID.h"
#include "RecordDescriptor.h"

class SystemManager;

class DBHandle {
public:
    RETVAL refreshHandle();
    RETVAL createTable(const char* relName, int attrCount, AttrInfo* attributes);
    RETVAL dropTable(const char *relName);
    RETVAL createIndex(const char *relName, const char *attrName);
    RETVAL dropIndex(const char *relName, const char *attrName);

    RETVAL help();
    std::vector<std::vector<std::string>> qHelp();
    RETVAL help(const char* relName);
    std::vector<std::vector<std::string>> qHelp(const char* relName);
    RETVAL print(const char *relName);
    std::map<RecordID, RecordDescriptor> qPrint(const char* relName);

    // Given a Relation name in this DB, Fill the AttrInfo of this relation
    RETVAL fillAttributesFromTable(const char* relName, int& attrCount, DataAttrInfo*& dataAttrInfo);

    // Return true if there is a relation named relName
    bool hasRelation(const char *relName);
    // Return true if there is a attribute name attrName in the relation
    // Assuming that we've got a relation named relName
    bool hasAttribute(const char *relName, const char *attrName);
    bool hasIndex(const char *relName, const char *idxName);

    typedef ComparisonTree::ComparisonDescriptor Comparison;
    typedef AttributeTree::AttributeDescriptor Attribute;
    std::map<RecordID, RecordDescriptor> retrieveRecords(std::string relName, RETVAL& rc);
    RecordDescriptor retrieveOneRecord(std::string relName, const RecordID& recordID, RETVAL& rc);

    DBHandle();
    ~DBHandle();
    friend class SystemManager;
private:

    std::vector<DataAttrInfo> attributes;
    std::vector<DataRelInfo> relations;
    std::vector<DataIdxInfo> indexes;
    std::vector<DataFkInfo> foreignKeys;
    std::vector<RecordID> relationRecordIDs;
    std::vector<RecordID> attributeRecordIDs;
    std::vector<RecordID> indexRecordIDs;
    std::vector<RecordID> foreignKeyRecordIDs;
};


#endif //DATABASE_DBHANDLER_H
