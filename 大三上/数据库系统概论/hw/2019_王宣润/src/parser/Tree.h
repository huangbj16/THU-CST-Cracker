#ifndef DATABASE_TREE_H
#define DATABASE_TREE_H

#include <string>
#include <vector>
#include "../global.h"

class Tree;
class SelectTree;
class InsertTree;
class UpdateTree;
class DeleteTree;
class CreateDatabaseTree;
class CreateTableTree;
class DropDatabaseTree;
class DropTableTree;
class UseDatabaseTree;
class ColumnsTree;
class ColumnTree;
class AttributesTree;
class AttributeTree;
class RelationsTree;
class WhereClauseTree;
class ConditionsTree;
class ComparisonTree;
class ConstValuesTree;
class ConstValueTree;
class InsertValueTree;
class CreateIndexTree;
class DropIndexTree;
class TypeTree;
class SetClauseTree;
class OperatorTree;
class PrimarySetTree;
class AddPrimaryTree;
class ForeignSetTree;
class RenameTree;
class AddAttrTree;
class DropAttrTree;
class AlterAttrTree;
class ShowDatabaseTree;

class Tree {
public:
    virtual void visit() {
        assert(false);
    }
    virtual ~Tree() {}
    static void setInstance(Tree *t) {
        if (tree != nullptr) {
            delete tree;
        }
        tree = t;
    }
    static void run() {
        if (tree != nullptr)
            tree->visit();
    }
    Tree(){ error = false; }
    static Tree *tree;
    bool error; // 如果error为真，表示该句存在问题，不应执行
};

class TypeTree : public Tree {
public:
    TypeTree(AttrType t);
    TypeTree(AttrType t, int l);
    AttrType getType();
    int getLen();
private:
    AttrType type;
    int len;
};

class SelectTree : public Tree {
public:
    SelectTree(AttributesTree *attributes,
               RelationsTree *relations,
               WhereClauseTree *whereClause);
    SelectTree(RelationsTree *relations, WhereClauseTree *whereClause);
    virtual ~SelectTree();
    void visit();
private:
    AttributesTree *attributes;
    RelationsTree *relations;
    WhereClauseTree *whereClause;
    std::string groupAttrName;
};


class InsertTree : public Tree {
public:
    InsertTree(const char *relationName, InsertValueTree* insertValueTree, bool force = false);
    InsertTree(const char *relationName, AttributesTree *attrs, InsertValueTree* insertValueTree, bool force = false);
    virtual ~InsertTree();
    void visit();
private:
    std::string relationName;
    InsertValueTree* insertValueTree;
    AttributesTree* attrs;
    bool force;
};


class UpdateTree : public Tree {
public:
    UpdateTree(std::string relationName,
               SetClauseTree *setClause,
               WhereClauseTree *whereClause);
    virtual ~UpdateTree();
    void visit();
private:
    std::string relationName;
    SetClauseTree *setClause;
    WhereClauseTree *whereClause;
};


class DeleteTree : public Tree {
public:
    DeleteTree(const char *relationName, WhereClauseTree *whereClause);
    virtual ~DeleteTree();
    void visit();
private:
    std::string relationName;
    WhereClauseTree *whereClause;
};


class UseDatabaseTree : public Tree {
public:
    UseDatabaseTree(const char *dbName);
    void visit();
private:
    std::string dbName;
};


class CreateDatabaseTree : public Tree {
public:
    CreateDatabaseTree(const char *dbName);
    virtual ~CreateDatabaseTree();
    void visit();
private:
    std::string dbName;
};

class DescTableTree : public Tree {
public:
    DescTableTree(const char* relName);
    virtual ~DescTableTree();
    void visit();
private:
    std::string relName;
};


class CreateTableTree : public Tree {
public:
    CreateTableTree(const char *tableName, ColumnsTree *columns);
    virtual ~CreateTableTree();
    void visit();
private:
    std::string tableName;
    ColumnsTree *columns;
};


class DropDatabaseTree : public Tree {
public:
    DropDatabaseTree(const char *dbName);
    virtual ~DropDatabaseTree();
    void visit();
private:
    std::string dbName;
};


class DropTableTree : public Tree {
public:
    DropTableTree(const char *tableName);
    virtual ~DropTableTree();
    void visit();
private:
    std::string tableName;
};


class CreateIndexTree: public Tree {
public:
    CreateIndexTree(const char *idxName, const char *relName, AttributesTree *attr);
    virtual ~CreateIndexTree();
    void visit();

private:
    std::string idxName;
    std::string relName;
    AttributesTree *attribute;
};


/*
    索引也是有名字的，一个索引名对应了一组索引信息
    删除和加入的时候，都直接对索引名为xx的索引进行操作
*/
class DropIndexTree: public Tree {
public:
    // DropIndexTree(const char *relName, AttributeTree *attr);
    DropIndexTree(const char *idxName, const char *relName);
    virtual ~DropIndexTree();
    void visit();

private:
    std::string relName;
    std::string idxName;
    // AttributeTree *attribute;
};

class ColumnsTree : public Tree {
public:
    ColumnsTree();
    virtual ~ColumnsTree();
    void addColumn(ColumnTree *);
    bool setPrimaryKey(const char* attr);

    int getColumnCount();
    AttrInfo *getAttrInfos();
    void deleteAttrInfos();

    std::vector<ForeignSetTree *> getForeignKeys();
private:
    std::vector<ColumnTree *> columns;
    AttrInfo *attrInfos;
    int primaryCount;       // 主键有几个attr
    std::vector<ForeignSetTree *> fks;
};


class ColumnTree : public Tree {
public:
    ColumnTree(const char *columnName, AttrType type, int size = 4,
               int notNull = 0, ConstValueTree *defaultValue = NULL);
    ColumnTree();
    virtual ~ColumnTree();

    AttrInfo getAttrInfo();
    void setNotNull(int notNull);
    friend class ColumnsTree;
    bool isPrimarySetTree;
    bool isForeignSetTree;
private:
    std::string columnName;
    AttrType type;
    int size;
    int isPrimaryKey;
    int notNull;
    int isDefault;
    AttrValue constDescriptor;
};

class PrimarySetTree : public ColumnTree {
public:
    PrimarySetTree(AttributesTree *attrs);
    virtual ~PrimarySetTree();
    AttributesTree *attrs;
};

class ForeignSetTree : public ColumnTree {
public:
    ForeignSetTree(const char *fkName, const char *masRelName, AttributesTree *serAttrs, AttributesTree *masAttrs);
    virtual ~ForeignSetTree();
    std::string fkName;
    std::string masRelName;
    AttributesTree *serAttrs;
    AttributesTree *masAttrs;
private:
};

class AttributeTree : public Tree {
public:
    AttributeTree(const char *relationName, const char *attributeName);
    AttributeTree(const char *attributeName);
    virtual ~AttributeTree();

    bool operator ==(const AttributeTree &attribute) const;

    struct AttributeDescriptor {
        std::string relName;
        std::string attrName;
        AttributeDescriptor(std::string relName = "",
                            std::string attrName = "") :
                relName(relName), attrName(attrName) {}
    };

    AttributeDescriptor getDescriptor() const;
private:
    std::string relation;
    std::string attribute;
};


class AttributesTree : public Tree {
public:
    AttributesTree();
    virtual ~AttributesTree();
    void addAttribute(AttributeTree *attribute);
    std::vector<AttributeTree::AttributeDescriptor> getDescriptors() const;
private:
    std::vector<AttributeTree *> attributes;
};


class ConstValueTree : public Tree {
public:
    ConstValueTree(int i);
    ConstValueTree(float f);
    ConstValueTree(const char *s, bool isDate = false);
    virtual ~ConstValueTree();

    AttrValue getDescriptor();
    void setNull() {isNull = true;}

    typedef AttrValue ConstValueDescriptor;

    friend class ComparisonTree;
private:
    int i;
    float f;
    std::string s;
    AttrType type;
    bool isNull;
};


class ComparisonTree : public Tree {
public:
    ComparisonTree(AttributeTree *attribute, CmpOP op);   // is (not) null
    ComparisonTree(AttributeTree *attribute, CmpOP op, ConstValueTree *constValue);
    ComparisonTree(AttributeTree *attribute, CmpOP op, AttributeTree *attribute2);
    virtual ~ComparisonTree();

    struct ComparisonDescriptor {
        AttributeTree::AttributeDescriptor attr;
        CmpOP op;
        AttrValue val;
        AttributeTree::AttributeDescriptor attr2;
        bool isAttrCmp;
    };

    ComparisonDescriptor getDescriptor();

private:
    ConstValueTree *constValue;
    AttributeTree *attribute;
    AttributeTree *attribute2;
    CmpOP op;
    bool isAttrCmp;
};


class RelationsTree : public Tree {
public:
    RelationsTree();
    virtual ~RelationsTree();
    void addRelation(const char *relation);
    std::vector<std::string> getRelations();
private:
    std::vector<std::string> relations;
};


class WhereClauseTree : public Tree {
public:
    WhereClauseTree(ConditionsTree *conditions);
    WhereClauseTree();
    virtual ~WhereClauseTree();

    std::vector<ComparisonTree::ComparisonDescriptor> getComparision();

private:
    ConditionsTree *conditions;
};


class ConditionsTree : public Tree {
public:
    ConditionsTree();
    virtual ~ConditionsTree();
    void addComparison(ComparisonTree *comparison);
    std::vector<ComparisonTree::ComparisonDescriptor> getComparisions();
private:
    std::vector<ComparisonTree *> comparisons;
};


class ConstValuesTree : public Tree {
public:
    ConstValuesTree();
    virtual ~ConstValuesTree();
    void addConstValue(ConstValueTree *constValue);

    std::vector<AttrValue> getConstValues();
private:
    std::vector<ConstValueTree *> constValues;
};

class InsertValueTree : public Tree {
public:
    InsertValueTree(ConstValuesTree *constValues);
    virtual ~InsertValueTree();
    void addConstValues(ConstValuesTree* constValuesTree);
    std::vector<ConstValuesTree*> values;
};

class CheckTree : public Tree {
public:
    CheckTree();
    virtual ~CheckTree();
};

class ChecksTree : public Tree {
public:
    ChecksTree();
    virtual ~ChecksTree();
    void addCheckTree(CheckTree* tree);
    std::vector<CheckTree*> checks;
};

class OperatorTree : public Tree {
public:
    OperatorTree(CmpOP op);
    virtual ~OperatorTree();
    CmpOP op;
};

class SetClauseTree : public Tree {
public:
    SetClauseTree();
    virtual ~SetClauseTree();
    void addClause(const char *colName, ConstValueTree *constValue);
    std::vector<std::pair<std::string, ConstValueTree*> > clauses;
};

class AddPrimaryTree : public Tree {
public:
    AddPrimaryTree(const char *relName, AttributesTree* attrs);
    virtual ~AddPrimaryTree();
    void visit();
private:
    std::string relName;
    std::vector<std::string> attrs;
};

class AddForeignTree : public Tree {
public:
    AddForeignTree(const char *fkName, const char *serRelName, const char *masRelName, AttributesTree* serAttrs, AttributesTree* masAttrs);
    virtual ~AddForeignTree();
    void visit();
private:
    std::string fkName, serRelName, masRelName;
    std::vector<AttributeTree::AttributeDescriptor> serAttrs, masAttrs;
};

class DropPrimaryTree : public Tree {
public:
    DropPrimaryTree(const char *relName);
    virtual ~DropPrimaryTree();
    void visit();
private:
    std::string relName;
};

class DropForeignTree : public Tree {
public:
    DropForeignTree(const char *relName, const char *fkName);
    virtual ~DropForeignTree();
    void visit();
private:
    std::string relName, fkName;
};

class RenameTree : public Tree {
public:
    RenameTree(const char *a, const char *b);
    virtual ~RenameTree();
    void visit();
private:
    std::string a, b;
};

class AddAttrTree : public Tree {
public:
    AddAttrTree(const char *relName, ColumnTree *attr);
    virtual ~AddAttrTree();
    void visit();
private:
    std::string relName;
    ColumnTree *attr;
};

class DropAttrTree : public Tree {
public:
    DropAttrTree(const char *relName, const char *attrName);
    virtual ~DropAttrTree();
    void visit();
private:
    std::string relName, attrName;
};

class AlterAttrTree : public Tree {
public:
    AlterAttrTree(const char *relName, const char *attrName, ColumnTree *attr);
    virtual ~AlterAttrTree();
    void visit();
private:
    std::string relName, attrName;
    ColumnTree *attr;
};

class ShowDatabaseTree : public Tree {
public:
    ShowDatabaseTree();
    virtual ~ShowDatabaseTree();
    void visit();
};

#endif //DATABASE_TREE_H
