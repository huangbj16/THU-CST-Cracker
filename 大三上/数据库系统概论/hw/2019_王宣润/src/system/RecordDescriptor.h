#ifndef DATABASE_RECORDDESCRIPTOR_H
#define DATABASE_RECORDDESCRIPTOR_H

#include <string>
#include <map>
#include "../parser/Tree.h"
#include "../record/Record.h"

class Printer;

// RecordDescriptor is a concrete class for Record
// We can access Record's data by attrName
class RecordDescriptor {
public:
    friend class Printer;
    friend class DBHandle;

    RecordDescriptor();

    /*
        Used by Insert Command
        给定一个表名和一系列列名，返回一个Record的读取器
        在这里不对主键完整性进行检查，需要在Insert函数内手动检查
    */
    static RecordDescriptor createRecordDescriptor(const std::string &relName, std::vector<AttrValue> vals, RETVAL& rc);
    static RecordDescriptor createRecordDescriptor(const std::string &relName, std::vector<std::string> &attrs, std::vector<AttrValue> vals, RETVAL& rc);


    // Used by Select Command
    RecordDescriptor filteredByAttributeName(std::vector<AttributeTree::AttributeDescriptor> attrs, bool checkRel = true) const;
    RecordDescriptor concatenate(const RecordDescriptor& next) const;

    AttrValue & operator [](const std::string &attrName);

    // Use this function after check assign Type
    void assign(const std::string &attrName, AttrValue &val);

    // Check Whether ConstValue can Assign to this Record (Not Relations!)
    bool checkAssignType(const std::string &attrName, AttrValue &val);

    // Check Whether ConstValue can Compare with this Record (Not Relations!)
    bool checkCompareType(const std::string &attrName, AttrValue &val);

    // Convert this RecordDescriptor to a Record
    RETVAL toRecord(const RecordID rid, Record& record);

    std::vector<std::string> attrNames;
    std::vector<AttrValue> attrVals;
    std::string relName;
private:
    int getIndex(std::string attrName);
};


#endif //DATABASE_RECORDDESCRIPTOR_H
