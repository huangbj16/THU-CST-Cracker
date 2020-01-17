#ifndef DATABASE_SYSTEMMANAGER_H
#define DATABASE_SYSTEMMANAGER_H


#include <map>
#include "../global.h"
#include "../record/FileHandler.h"
#include "../parser/Tree.h"
#include "RecordDescriptor.h"
#include "DBHandle.h"


class SystemManager {
public:

    static SystemManager* instance();
    static void resetInstance();

    // DB Operations
    RETVAL createDB(const char* dbName);
    RETVAL dropDB(const char* dbName);
    RETVAL openDB(const char *dbName);
    RETVAL closeDB();

    // Relation Operations
    RETVAL createTable(const char *relName, int attrCount, AttrInfo *attributes);
    // 如果check为false，则不检查数据完整性
    RETVAL dropTable(const char *relName, bool check = true);
    RETVAL createIndex(std::string relName, AttributeTree::AttributeDescriptor attr, std::string idxName);
    RETVAL dropIndex(std::string relName, std::string idxName);

    RETVAL addPrimaryKey(const char *relName, std::vector<std::string> attrs);
    RETVAL dropPrimaryKey(const char *relName);
    RETVAL getPrimaryKeyList(const char *relName, std::vector<std::string> &plist);

    RETVAL addForeignKey(const char *fkName, const char *serRelName, const char *masRelName, 
            std::vector<AttributeTree::AttributeDescriptor> &serAttrs, std::vector<AttributeTree::AttributeDescriptor> &masAttrs, bool checked = false);
    RETVAL getForeignKeyList(std::vector<DataFkInfo> &flist);
    RETVAL dropForeignKey(const char *relName, const char *fkName);

    // 给定表名、列名，获得列详细信息
    RETVAL getAttributeInfo(const char *relName, const char *attrName, DataAttrInfo &info);

    // 指定的表里是否有这样的记录：它在attrs[i]的值为vals[i]
    bool existsRecord(const char *relName, std::vector<AttributeTree::AttributeDescriptor> attrs, std::vector<AttrValue> vals);

    RETVAL load(const char *relName, const char *fileName);
    RETVAL help();
    std::vector<std::vector<string> > qHelp();
    RETVAL help(const char *relName);
    std::vector<std::vector<std::string>> qHelp(const char* relName);
    RETVAL print(const char *relName) {return dbHandle.print(relName);}
    std::map<RecordID, RecordDescriptor> qPrint(const char* relName) {return dbHandle.qPrint(relName);}
    RETVAL qUpdate(const std::string& relName, const RecordID& recordID,
            int attrNo, const AttrValue& newVal);
    RETVAL set(const char *paramName, const char *value);

    // SQL Instruction API
    RETVAL Select(std::vector<AttributeTree::AttributeDescriptor> attrs,
            std::vector<std::string> rels,
            std::vector<ComparisonTree::ComparisonDescriptor> coms);

    // 如果check为false，则不检查数据完整性
    RETVAL Insert(std::string relName, std::vector<std::string>* attrs,
            std::vector<AttrValue> vals, bool check = true);

    // 使得表relName中满足coms约束条件的attr列变为val
    RETVAL Update(std::string relName,
            AttributeTree::AttributeDescriptor attr,
            AttrValue val,
            std::vector<ComparisonTree::ComparisonDescriptor> coms);


    RETVAL Delete(std::string relName,
            std::vector<ComparisonTree::ComparisonDescriptor> coms);

    // Public Utility Functions
    // Given a Relation name in this DB, Fill the AttrInfo of this relation
    RETVAL fillAttributesFromTable(const char* relName, int& attrCount, DataAttrInfo*& dataAttrInfo)
    {RETURNIF(dbHandle.fillAttributesFromTable(relName, attrCount, dataAttrInfo)); return RETVAL_OK;}

    std::vector<RecordDescriptor> select(std::vector<AttributeTree::AttributeDescriptor> attrs,
                                        std::vector<std::string> rels,
                                        std::vector<ComparisonTree::ComparisonDescriptor> coms,
                                        RETVAL& rc);

    RETVAL AddAttr(std::string relName, AttrInfo attr);
    RETVAL DelAttr(std::string relName, std::string attrName);
    RETVAL ModifyAttr(std::string relName, std::string attrName, AttrInfo attr);
    RETVAL RenameTable(std::string a, std::string b);

    RETVAL ShowDatabase();

    ~SystemManager();
private:
    SystemManager();

    static SystemManager* systemManager;
    FileHandler* recordManager;
    DBHandle dbHandle;
    bool hasOpenDB;
    char currentDBName[MAX_NAME];

    // Return true if there is a relation named relName
    bool hasRelation(const char *relName) {return dbHandle.hasRelation(relName);}

    // Return true if there is a attribute name attrName in the relation
    // Assuming that we've got a relation named relName
    bool hasAttribute(const char *relName, const char *attrName) {return dbHandle.hasAttribute(relName, attrName);}
    bool hasIndex(const char *relName, const char *idxName) {return dbHandle.hasIndex(relName, idxName);}

    typedef ComparisonTree::ComparisonDescriptor Comparison;
    typedef AttributeTree::AttributeDescriptor Attribute;

    std::map<RecordID, RecordDescriptor> retrieveRecords(std::string relName, RETVAL& rc)
    { return dbHandle.retrieveRecords(relName, rc);}

    bool checkRelations(const std::vector<std::string> &relations);
    bool checkAttributes(std::vector<Attribute> &attributes, std::vector<std::string> &relations);
    bool checkComparison(std::vector<Comparison> &coms, std::string &relation);
    bool checkComparisons(std::vector<Comparison> &coms, std::vector<std::string> &relations);

    bool isValid(std::vector<Comparison> &coms, RecordDescriptor &record, const std::string &relation);

    void iterateCrossProduct(std::vector<std::vector<RecordDescriptor>> &records,
                            std::vector<AttributeTree::AttributeDescriptor> attrs,
                            std::vector<std::string> &rels,
                            std::vector<Comparison> &coms,
                            int depth,
                            std::vector<RecordDescriptor>& output);

    std::vector<RecordDescriptor> retrieveRecordsByIndex(std::string relName, const std::vector<Comparison>& coms, RETVAL& rc);

    static vector<string> &split(const string &s, char delim, vector<string> &elems) {
        stringstream ss(s);
        string item;
        while (getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }


    static vector<std::string> split(const string &s, char delim) {
        vector<string> elems;
        split(s, delim, elems);
        return elems;
    }
};


bool operator < (const vector<AttrValue> &a, const vector<AttrValue> &b);
bool operator == (const vector<AttrValue> &a, const vector<AttrValue> &b);

#endif //DATABASE_SYSTEMMANAGER_H
