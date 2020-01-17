#ifndef DATABASE_GLOBAL_H
#define DATABASE_GLOBAL_H
#include <iostream>
#include <cstdio>
#include <cassert>
#include <sstream>
#include <vector>
#include <regex>

#define MAX_PAGE_NUMBER 4096
#define PAGE_HEADER_SIZE 48
#define MAX_NAME 42
#define MAX_DEF  20
#define MAX_RECORD_SIZE 4096
#define MAX_CHECK_SIZE 10
#define NULL_MAGIC_NUMBER 0x19980112u

enum CmpOP {
    T_EQ, T_LT, T_GT, T_LE, T_GE, T_NE, T_NO, T_ISNOTNULL, T_ISNULL, T_IN
};

enum AttrType {
    T_INT, T_FLOAT, T_STRING, T_DATE, T_NONE
};


template<typename T>
bool compare(const T &a, const T &b, CmpOP op) {
    switch (op) {
        case CmpOP::T_EQ:
            return a == b;
        case CmpOP::T_LT:
            return a < b;
        case CmpOP::T_GT:
            return a > b;
        case CmpOP::T_LE:
            return a <= b;
        case CmpOP::T_GE:
            return a >= b;
        case CmpOP::T_NE:
            return a != b;
        default:
            return false;
    }
}

template<typename T>
static bool compareIn(const T &a, const std::vector<T>& b) {
    for(const auto v : b) {
        if(a == v)
            return true;
    }
    return false;
}

struct AttrValue {
    AttrType type;
    int i;
    float f;
    std::string s;
    bool isNull;

    // Definitions are in Tree.cpp
    bool operator ==(const AttrValue &val) const;
    bool operator !=(const AttrValue &val) const;
    bool operator >=(const AttrValue &val) const;
    bool operator <=(const AttrValue &val) const;
    bool operator > (const AttrValue &val) const;
    bool operator < (const AttrValue &val) const;

    void* getElementPointer() {
        switch (type){
            case T_INT:
                return (void*)(&i);
            case T_FLOAT:
                return (void*)(&f);
            case T_STRING:
                return (void*)(s.c_str());
            case T_DATE:
                return (void*)(s.c_str());
        }
        return NULL;
    }
};

struct AttrInfo {
    char attrName[MAX_NAME + 1];   
    int attrLength;
    AttrType attrType;
    int isPrimaryKey;
    int notNull;
    int isDefault;
    AttrValue defaultVal;
};

struct DataAttrInfo {
    char relName[MAX_NAME + 1];     // 这个Attr在哪个Relation里
    char attrName[MAX_NAME + 1];    // 这个Attr的名称
    int offset;                     // 这个Attr在Record中的偏移量
    AttrType attrType;              // Attr的类型
    int attrLength;                 // Attr占用字节长度
    int indexNo;                    // Attr在Relation中的编号
    int isPrimaryKey;               // 是否为主键，如果不为0表示在主键中的编号
    int notNull;                    // 是否为非空
    int isDefault;                  // 是否有默认值
    char defaultVal[MAX_DEF + 1];   // 如果有默认值，则默认值为defaultVal字节转化成对应类型之后的值
};

struct DataRelInfo {
    char relName[MAX_NAME + 1];     // 表名
    int recordSize;                 // 一条Record的长度
    int attrCount;                  // 有几列
    int indexCount;                 // 有几行
    int primaryCount;               // 主键含有几个attr
};

struct DataFkInfo {
    char fkName[MAX_NAME + 1];      // 外键名
    char serRelName[MAX_NAME + 1];  // 从表名
    char masRelName[MAX_NAME + 1];  // 主表名
    int attrCount;                  // 外键由几个attr组成
    char serAttr1Name[MAX_NAME + 1];// 从表attr1
    char serAttr2Name[MAX_NAME + 1];// 从表attr2
    char serAttr3Name[MAX_NAME + 1];// 从表attr3
    char masAttr1Name[MAX_NAME + 1];// 主表attr1
    char masAttr2Name[MAX_NAME + 1];// 主表attr2
    char masAttr3Name[MAX_NAME + 1];// 主表attr3
};

struct DataIdxInfo {
    char idxName[MAX_NAME + 1];     // 索引名
    char relName[MAX_NAME + 1];     // 表名
    char attrName[MAX_NAME + 1];    // 列名
};

typedef int RETVAL;

#define RETVAL_OK               0
#define RETVAL_ERR              1
#define RETVAL_EOF              2
#define RETVAL_NOREC            3
#define RETVAL_NOIDX            4

// Exit immediately if error occurs
#define RETURNIF(rc) do { RETVAL _x_ = (rc); if ((_x_) != RETVAL_OK) { PrintError(__LINE__, __FILE__, __func__); return _x_; }} while(0)

#define MSGIF(rc, msg) do { RETVAL _x_ = (rc); if ((_x_) != RETVAL_OK) { printf("%s\n", msg); return _x_; }} while(0)


inline void PrintError(int line, const char* file, const char* func) {
    printf("Error occured, in line %d of file \"%s\", in function<%s>\n",
          line,
          file,
          func);
}

#define kDefaultDBPosition "./"
#define kDefaultRelCatName "relcat"
#define kDefaultAttrCatName "attrcat"
#define kDefaultFkCatName "fkcat"
#define kDefaultIdxCatName "idxcat"
#define kDefaultCheckCatName "checkcat"

template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}


#endif //DATABASE_GLOBAL_H
