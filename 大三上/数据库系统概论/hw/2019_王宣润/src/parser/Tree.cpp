#include "tree.h"
#include "../system/SystemManager.h"

#include <cstdlib>
#include <cstring>

using std::string;
using std::vector;

void Error(const char *msg) {
    printf("> Error: %s\n", msg);
    exit(0);
}

Tree* Tree::tree = nullptr;

/* TypeTree */
TypeTree::TypeTree(AttrType t){
    this->type = t;
    this->len = 4;
}

TypeTree::TypeTree(AttrType t, int l){
    this->type = t;
    this->len = l;
}

AttrType TypeTree::getType(){
    return this->type;
}

int TypeTree::getLen(){
    return this->len;
}


/* SelectTree */
SelectTree::SelectTree(RelationsTree *relations, WhereClauseTree *whereClause) {
    this->attributes = nullptr;
    this->relations = relations;
    this->whereClause = whereClause;
}

SelectTree::SelectTree(AttributesTree *attributes,
                       RelationsTree *relations,
                       WhereClauseTree *whereClause) {
    this->attributes = attributes;
    this->relations = relations;
    this->whereClause = whereClause;
}

SelectTree::~SelectTree() {
    delete attributes;
    delete relations;
    delete whereClause;
}

void SelectTree::visit() {
    vector<AttributeTree::AttributeDescriptor> attrs;
    if (attributes) {
        attrs = attributes->getDescriptors();
    } else {
        attrs = vector<AttributeTree::AttributeDescriptor>();
    }
    vector<string> rels = relations->getRelations();
    vector<ComparisonTree::ComparisonDescriptor> coms;
    if (whereClause != nullptr) {
        coms = whereClause->getComparision();
        SystemManager::instance()->Select(attrs, rels, coms);
    } else {
        SystemManager::instance()->Select(attrs, rels, coms);
    }
}


/* InsertTree */
InsertTree::InsertTree(const char *relationName, InsertValueTree* insertValueTree, bool force) {
    this->relationName = string(relationName);
    this->insertValueTree = insertValueTree;
    this->attrs = NULL;
    this->force = force;
}

InsertTree::InsertTree(const char *relationName, AttributesTree* attrs, InsertValueTree* insertValueTree, bool force) {
    this->relationName = string(relationName);
    this->insertValueTree = insertValueTree;
    this->attrs = attrs;
    this->force = force;
}

InsertTree::~InsertTree() {
    delete insertValueTree;
}

void InsertTree::visit() {
    std::vector<std::string> attrVector;
    if (attrs != NULL){
        auto des = attrs->getDescriptors();
        for (auto t: des)
            attrVector.push_back(t.attrName);
        for(const auto& constValues : insertValueTree->values)
            // force = true -> 不检查, 传入false
            SystemManager::instance()->Insert(relationName, &attrVector, constValues->getConstValues(), !force);
    }else{
        for(const auto& constValues : insertValueTree->values)
            SystemManager::instance()->Insert(relationName, NULL, constValues->getConstValues(), !force);
    }
}

/* UpdateTree */
UpdateTree::UpdateTree(string relationName,
                       SetClauseTree *setClause,
                       WhereClauseTree *whereClause) {
    this->relationName = relationName;
    this->setClause = setClause;
    this->whereClause = whereClause;
}

UpdateTree::~UpdateTree() {
    delete setClause;
    delete whereClause;
}

void UpdateTree::visit() {
    for (auto &t: setClause->clauses){
        AttributeTree::AttributeDescriptor attr = (AttributeTree::AttributeDescriptor){relationName, t.first};
        AttrValue val = t.second->getDescriptor();
        vector<ComparisonTree::ComparisonDescriptor> coms;
        if (whereClause != NULL){
            cerr << "[WARNING] Where-clause is not supported and will be ignored." << endl;
            // coms = whereClause->getComparision();
        }
        SystemManager::instance()->Update(relationName, attr, val, coms);
    }
}

/* DeleteTree */
DeleteTree::DeleteTree(const char *relationName, WhereClauseTree *whereClause) {
    this->relationName = string(relationName);
    this->whereClause = whereClause;
}

DeleteTree::~DeleteTree() {
    delete whereClause;
}

void DeleteTree::visit() {
    vector<ComparisonTree::ComparisonDescriptor> coms;
    if (whereClause != nullptr)
        coms = whereClause->getComparision();
    SystemManager::instance()->Delete(relationName, coms);
}

/* CreateDatabaseTree */
CreateDatabaseTree::CreateDatabaseTree(const char *dbName) {
    this->dbName = string(dbName);
}

CreateDatabaseTree::~CreateDatabaseTree() {
}

void CreateDatabaseTree::visit() {
    SystemManager::instance()->createDB(dbName.c_str());
}

/* CreateTableTree */
CreateTableTree::CreateTableTree(const char *tableName, ColumnsTree *columns) {
    this->tableName = string(tableName);
    this->columns = columns;
}

CreateTableTree::~CreateTableTree() {
    delete columns;
}

void CreateTableTree::visit() {
    int attrCount = columns->getColumnCount();
    if (columns->error);
    else{
        AttrInfo *attrInfos = columns->getAttrInfos();
        vector<ForeignSetTree*> fks = columns->getForeignKeys();
        // 确认外键合法
        vector<DataFkInfo> flist;
        SystemManager::instance()->getForeignKeyList(flist);
        for (const auto &foreignSetTree: fks){
            auto masAttrs = foreignSetTree->masAttrs->getDescriptors(); // vector
            auto serAttrs = foreignSetTree->serAttrs->getDescriptors();
            auto masRelName = string(foreignSetTree->masRelName);
            auto serRelName = this->tableName;
            if (masAttrs.size() != serAttrs.size()){
                cerr << "[ERROR] The lengths of column lists from servant table and master table should be equal." << endl;
                return;
            }
            // 检查是否有重复的键名
            for (int i = 0, limi = serAttrs.size(); i < limi; ++i)
                for (int j = i + 1; j < limi; ++j)
                    if (serAttrs[i].attrName == serAttrs[j].attrName ||
                        masAttrs[i].attrName == masAttrs[j].attrName){
                            cerr << "[ERROR] Duplicated attributes in foreign key setting clause." << endl;
                            return;
                        }
            // 检查外键列表是否在从表中出现
            for (const auto &fkattr: serAttrs){
                bool flag = false;
                for (int i = 0; i < attrCount; ++i)
                    if (fkattr.attrName == string(attrInfos[i].attrName)){
                        flag = true;
                        break;
                    }
                if (!flag){
                    cerr << "[ERROR] <" << fkattr.attrName << "> is not an attribute in this table." << endl;
                    return;
                }
            }
            // 检查外键列表是否是主表的主键
            vector<string> masPrimaryKeyList;
            int rc = SystemManager::instance()->getPrimaryKeyList(masRelName.c_str(), masPrimaryKeyList);
            if (rc != RETVAL_OK) return;
            for (const auto &fkattr: masAttrs){
                bool found = false;
                for (const auto &attr: masPrimaryKeyList)
                    if (attr == fkattr.attrName){
                        found = true;
                        break;
                    }
                if (!found){
                    cerr << "[ERROR] <" << fkattr.attrName << "> is not one of the primary key of the master table." << endl;
                    return;
                }
            }
            if (masAttrs.size() != masPrimaryKeyList.size()){
                cerr << "[ERROR] The attributes provided is not a primary key of the master table." << endl;
                return;
            }
            // 检查是否已有这个名字的外键
            for (const auto &fkInfo: flist)
                if (strcmp(fkInfo.fkName, foreignSetTree->fkName.c_str()) == 0 && strcmp(fkInfo.serRelName, serRelName.c_str()) == 0) {
                    cerr << "[ERROR] Foreign key named <" << fkInfo.fkName << "> on table <" << serRelName << "> already exists." << endl;
                    return;
                }
            // 检查复合外键长度
            if (serAttrs.size() > 3){
                cerr << "[ERROR] Foreign keys should have no more than 3 attributes." << endl;
                return;
            }
            // 检查类型匹配性
            for (int i = 0, limi = masAttrs.size(); i < limi; ++i){
                int rc;
                DataAttrInfo m;
                rc = SystemManager::instance()->getAttributeInfo(masRelName.c_str(), masAttrs[i].attrName.c_str(), m);
                if (rc != RETVAL_OK){
                    cerr << "[ERROR] Cannot find attribute <" << masAttrs[i].attrName << "> in master table." << endl;
                    return;
                }
                string serAttrName = serAttrs[i].attrName;
                AttrType stype = T_NONE;
                for (int j = 0; j < attrCount; ++j)
                    if (string(attrInfos[j].attrName) == serAttrName){
                        stype = attrInfos[j].attrType;
                        break;
                    }
                if (m.attrType != stype){
                    cerr << "[ERROR] Foreign key types mismatch." << endl;
                    return;
                }
            }
            
            // 合法，可以添加外键
        }

        SystemManager::instance()->createTable(tableName.c_str(), attrCount, attrInfos);
        for (const auto &foreignSetTree: fks){
            auto masAttrsDes = foreignSetTree->masAttrs->getDescriptors(); // vector
            auto serAttrsDes = foreignSetTree->serAttrs->getDescriptors();
            auto masRelName = string(foreignSetTree->masRelName);
            auto serRelName = this->tableName;
            SystemManager::instance()->addForeignKey(foreignSetTree->fkName.c_str(), 
                serRelName.c_str(), masRelName.c_str(), serAttrsDes, masAttrsDes, true);
        }
        // delete attrInfos;
        // columns->deleteAttrInfos();
    }
}


/* CreateIndexTree */
CreateIndexTree::CreateIndexTree(const char *idxName, const char *relName, AttributesTree *attr) {
    this->idxName = string(idxName);
    this->relName = string(relName);
    this->attribute = attr;
}

CreateIndexTree::~CreateIndexTree() {
    delete attribute;
}

void CreateIndexTree::visit() {
    // TODO MODIFY
    // 从单索引变成了多索引 需要修改        不改了
    if (attribute->getDescriptors().size() > 1)
        cerr << "[WARNING] This DB only support one-attribute indexing. This command is ignored." << endl;
    else{
        auto attr = attribute->getDescriptors()[0];
        SystemManager::instance()->createIndex(relName, attr, idxName);
    }
}


DropIndexTree::DropIndexTree(const char *idxName, const char *relName){
    this->relName = string(relName);
    this->idxName = string(idxName);
}

DropIndexTree::~DropIndexTree() {
}

void DropIndexTree::visit() {
    SystemManager::instance()->dropIndex(relName, idxName);
}


/* DropDatabaseTree */
DropDatabaseTree::DropDatabaseTree(const char *dbName) {
    this->dbName = string(dbName);
}

DropDatabaseTree::~DropDatabaseTree() {
}

void DropDatabaseTree::visit() {
    SystemManager::instance()->dropDB(dbName.c_str());
}

/* DropTableTree */
DropTableTree::DropTableTree(const char *tableName) {
    this->tableName = string(tableName);
}

DropTableTree::~DropTableTree() {
}

void DropTableTree::visit() {
    SystemManager::instance()->dropTable(tableName.c_str());
}

/* ColumnsTree */
ColumnsTree::ColumnsTree() {
    attrInfos = NULL;
    primaryCount = 0;
    fks.clear();
}

ColumnsTree::~ColumnsTree() {
    for (const auto &column : columns)
        delete column;
}

void ColumnsTree::addColumn(ColumnTree *column) {
    if (!column->isPrimarySetTree && !column->isForeignSetTree){  // 是添加一列
        for (const auto &col : columns)
            if (col->columnName == column->columnName){
                error = true;
                cerr << "[ERROR] Duplicated column name." << endl;
            }
        columns.push_back(column);
        if (column->error)
            error = true;
    }else if (column->isPrimarySetTree) {  // 是主键列表
        if (primaryCount > 0){
            error = true;
            cerr << "[ERROR] Duplicated primary key." << endl;
        }
        PrimarySetTree* pst = static_cast<PrimarySetTree*>(column);
        auto des = pst->attrs->getDescriptors();
        primaryCount = des.size();
        for (int i = 0; i < primaryCount; ++i){
            bool found = false;
            for (auto &col : columns)
                if (col->columnName == des[i].attrName){
                    col->isPrimaryKey = i + 1;      // 从1开始编号
                    col->notNull = 1;
                    found = true;
                    break;
                }
            if (!found){
                error = true;
                cerr << "[ERROR] Primary key <" << des[i].attrName << "> not found. You should declare the attribute name first." << endl;
            }
        }
    }else{  // 是外键列表，之后建表的时候再处理
        ForeignSetTree* fst = static_cast<ForeignSetTree*>(column);
        fks.push_back(fst);
    }
}

std::vector<ForeignSetTree *> ColumnsTree::getForeignKeys(){
    std::vector<ForeignSetTree *> tmp;
    for (const auto &t: fks)
        tmp.push_back(t);
    return tmp;
}

bool ColumnsTree::setPrimaryKey(const char *attr) {
    cerr << "[WARNING] setPrimaryKey() is deprecated." << endl;
    bool found = false;
    for(auto& tree : columns) {
        if(tree->columnName == string(attr)) {
            tree->isPrimaryKey = 1;
            tree->notNull = 1;
            found = true;
            break;
        }
    }
    return found;
}


int ColumnsTree::getColumnCount() {
    return (int) columns.size();
}

AttrInfo *ColumnsTree::getAttrInfos() {
    attrInfos = new AttrInfo[columns.size()];
    for (int i = 0; i < columns.size(); ++i)
        attrInfos[i] = columns[i]->getAttrInfo();
    return attrInfos;
}

void ColumnsTree::deleteAttrInfos() {
    delete attrInfos;
    attrInfos = nullptr;
}

/* ColumnTree */
ColumnTree::ColumnTree(const char *columnName, AttrType type, int size,
                       int notNull, ConstValueTree *defaultValue) {
    this->columnName = string(columnName);
    this->type = type;
    this->size = size;
    this->isPrimaryKey = 0;
    this->notNull = notNull;
    if (defaultValue != NULL){
        this->isDefault = true;
        this->constDescriptor = defaultValue->getDescriptor();
        if (this->constDescriptor.type == T_STRING && this->constDescriptor.s.length() > MAX_DEF){
            error = true;
            cerr << "[ERROR] Length of default string value should not exceed " << MAX_DEF << "." << endl;
        }
        if (this->constDescriptor.type != this->type){
            error = true;
            cerr << "[ERROR] Default value is not of the same type." << endl;
        }
    }else
        this->isDefault = false;
    
    if(type == T_STRING || type == T_DATE)
        this->size++;
    isPrimarySetTree = false;
    isForeignSetTree = false;
}
ColumnTree::ColumnTree(){
    isPrimarySetTree = false;
    isForeignSetTree = false;
}

void ColumnTree::setNotNull(int notNull) {
    this->notNull = notNull;
}

ColumnTree::~ColumnTree() {
}

AttrInfo ColumnTree::getAttrInfo() {
    AttrInfo attrInfo;
    strcpy(attrInfo.attrName, columnName.c_str());
    attrInfo.isDefault = isDefault;
    attrInfo.defaultVal = constDescriptor;
    attrInfo.attrType = (AttrType) type;
    attrInfo.attrLength = size;
    attrInfo.isPrimaryKey = isPrimaryKey;
    attrInfo.notNull = notNull;
    return attrInfo;
}


/* AttributesTree */
AttributesTree::AttributesTree() {
}

AttributesTree::~AttributesTree() {
    for (const auto &attribute : attributes)
        delete attribute;
}

void AttributesTree::addAttribute(AttributeTree *attribute) {
    attributes.push_back(attribute);
}

vector<AttributeTree::AttributeDescriptor> AttributesTree::getDescriptors() const {
    vector<AttributeTree::AttributeDescriptor> attrs;
    for (int i = 0; i < attributes.size(); ++i)
        attrs.push_back(attributes[i]->getDescriptor());
    return attrs;
}

/* AttributeTree */
AttributeTree::AttributeTree(const char *relationName, const char *attributeName) {
    this->relation = string(relationName);
    this->attribute = string(attributeName);
}

AttributeTree::AttributeTree(const char *attributeName) {
    this->attribute = string(attributeName);
}

AttributeTree::AttributeDescriptor AttributeTree::getDescriptor() const {
    return AttributeDescriptor(relation, attribute);
}

bool AttributeTree::operator==(const AttributeTree &attribute) const {
    return this->relation == attribute.relation &&
           this->attribute == attribute.attribute;
}

AttributeTree::~AttributeTree() {
}


/* RelationsTree */
RelationsTree::RelationsTree() {
}

RelationsTree::~RelationsTree() {
}

void RelationsTree::addRelation(const char *relation) {
    auto r = string(relation);
    for (const auto &rel : relations)
        if (rel == r)
            return;
    relations.push_back(r);
}

vector<string> RelationsTree::getRelations() {
    return relations;
}


/* WhereClauseTree */
WhereClauseTree::WhereClauseTree(ConditionsTree *conditions) {
    this->conditions = conditions;
}

WhereClauseTree::WhereClauseTree() {
    conditions = nullptr;
}

WhereClauseTree::~WhereClauseTree() {
    delete conditions;
}

vector<ComparisonTree::ComparisonDescriptor> WhereClauseTree::getComparision() {
    return conditions->getComparisions();
}

/* ConditionsTree */
ConditionsTree::ConditionsTree() {
}

ConditionsTree::~ConditionsTree() {
    for (const auto &comparision : comparisons)
        delete comparision;
}

void ConditionsTree::addComparison(ComparisonTree *comparison) {
    comparisons.push_back(comparison);
}

vector<ComparisonTree::ComparisonDescriptor> ConditionsTree::getComparisions() {
    vector<ComparisonTree::ComparisonDescriptor> coms;
    for (int i = 0; i < comparisons.size(); ++i)
        coms.push_back(comparisons[i]->getDescriptor());
    return coms;
}

/* ComparisonTree */
ComparisonTree::ComparisonTree(AttributeTree *attribute, CmpOP op) {
    this->op = op;
    this->attribute = attribute;
    this->constValue = nullptr;
    this->isAttrCmp = false;
}

ComparisonTree::ComparisonTree(AttributeTree *attribute, CmpOP op, ConstValueTree *constValue) {
    this->op = op;
    this->attribute = attribute;
    this->constValue = constValue;
    this->isAttrCmp = false;
}

ComparisonTree::ComparisonTree(AttributeTree *attribute, CmpOP op, AttributeTree *attribute2) {
    this->op = op;
    this->attribute = attribute;
    this->attribute2 = attribute2;
    this->isAttrCmp = true;
}

ComparisonTree::~ComparisonTree() {
    delete attribute;
    if(isAttrCmp)
        delete attribute2;
    else
        delete constValue;
}

ComparisonTree::ComparisonDescriptor ComparisonTree::getDescriptor() {
    ComparisonDescriptor com;
    com.attr = this->attribute->getDescriptor();
    if(!isAttrCmp && this->constValue != nullptr)
        com.val = this->constValue->getDescriptor();
    else if(isAttrCmp)
        com.attr2 = this->attribute2->getDescriptor();
    com.isAttrCmp = isAttrCmp;
    com.op = this->op;
    return com;
}


/* ConstValuesTree */
ConstValuesTree::ConstValuesTree() {
}

ConstValuesTree::~ConstValuesTree() {
    for (const auto &constValue: constValues)
        delete constValue;
}

void ConstValuesTree::addConstValue(ConstValueTree *constValue) {
    constValues.push_back(constValue);
}

vector<AttrValue> ConstValuesTree::getConstValues() {
    vector<AttrValue> vals;
    for (int i = 0; i < constValues.size(); ++i)
        vals.push_back(constValues[i]->getDescriptor());
    return vals;
}


/* ConstValueTree */
ConstValueTree::ConstValueTree(int i) {
    this->i = i;
    this->type = AttrType::T_INT;
    this->isNull = false;
}

ConstValueTree::ConstValueTree(float f) {
    this->f = f;
    this->type = AttrType::T_FLOAT;
    this->isNull = false;
}

ConstValueTree::ConstValueTree(const char *s, bool isDate) {
    this->s = string(s, 1, strlen(s) - 2);
    if (isDate)
        this->type = AttrType::T_DATE;
    else
        this->type = AttrType::T_STRING;
    this->isNull = false;
}

ConstValueTree::~ConstValueTree() {
}

AttrValue ConstValueTree::getDescriptor() {
    return AttrValue{type, i, f, s, isNull};
}

bool AttrValue::operator == (const AttrValue &val) const {
    if(this->isNull || val.isNull)
        return false;
    if ((type == AttrType::T_INT || type == AttrType::T_FLOAT) &&
        (val.type == AttrType::T_INT || val.type == AttrType::T_FLOAT)) {
        float f1 = this->type == AttrType::T_INT ? this->i : this->f;
        float f2 = val.type == AttrType::T_INT ? val.i : val.f;
        return f1 == f2;
    } else if (type == AttrType::T_STRING && val.type == AttrType::T_STRING) {
        return this->s == val.s;
    } else if (type == AttrType::T_DATE && val.type == AttrType::T_DATE) { 
        return this->s == val.s;
    } else {
        // TODO cannot compare
    }
    return false;
}

bool AttrValue::operator != (const AttrValue &val) const {
    if(this->isNull || val.isNull)
        return false;
    if ((type == AttrType::T_INT || type == AttrType::T_FLOAT) &&
        (val.type == AttrType::T_INT || val.type == AttrType::T_FLOAT)) {
        float f1 = this->type == AttrType::T_INT ? this->i : this->f;
        float f2 = val.type == AttrType::T_INT ? val.i : val.f;
        return f1 != f2;
    } else if (type == AttrType::T_STRING && val.type == AttrType::T_STRING) {
        return this->s != val.s;
    } else if (type == AttrType::T_DATE && val.type == AttrType::T_DATE) { 
        return this->s != val.s;
    } else {
        // TODO cannot compare
    }
    return false;
}

bool AttrValue::operator >= (const AttrValue &val) const {
    if(this->isNull || val.isNull)
        return false;
    if ((type == AttrType::T_INT || type == AttrType::T_FLOAT) &&
        (val.type == AttrType::T_INT || val.type == AttrType::T_FLOAT)) {
        float f1 = this->type == AttrType::T_INT ? this->i : this->f;
        float f2 = val.type == AttrType::T_INT ? val.i : val.f;
        return f1 >= f2;
    } else if (type == AttrType::T_STRING && val.type == AttrType::T_STRING) {
        return this->s >= val.s;
    } else if (type == AttrType::T_DATE && val.type == AttrType::T_DATE) { 
        return this->s >= val.s;
    } else {
        // TODO cannot compare
    }
    return false;
}

bool AttrValue::operator <= (const AttrValue &val) const {
    if(this->isNull || val.isNull)
        return false;
    if ((type == AttrType::T_INT || type == AttrType::T_FLOAT) &&
        (val.type == AttrType::T_INT || val.type == AttrType::T_FLOAT)) {
        float f1 = this->type == AttrType::T_INT ? this->i : this->f;
        float f2 = val.type == AttrType::T_INT ? val.i : val.f;
        return f1 <= f2;
    } else if (type == AttrType::T_STRING && val.type == AttrType::T_STRING) {
        return this->s <= val.s;
    } else if (type == AttrType::T_DATE && val.type == AttrType::T_DATE) { 
        return this->s <= val.s;
    } else {
        // TODO cannot compare
    }
    return false;
}

bool AttrValue::operator > (const AttrValue &val) const {
    if(this->isNull || val.isNull)
        return false;
    if ((type == AttrType::T_INT || type == AttrType::T_FLOAT) &&
        (val.type == AttrType::T_INT || val.type == AttrType::T_FLOAT)) {
        float f1 = this->type == AttrType::T_INT ? this->i : this->f;
        float f2 = val.type == AttrType::T_INT ? val.i : val.f;
        return f1 > f2;
    } else if (type == AttrType::T_STRING && val.type == AttrType::T_STRING) {
        return this->s > val.s;
    } else if (type == AttrType::T_DATE && val.type == AttrType::T_DATE) { 
        return this->s > val.s;
    } else {
        // TODO cannot compare
    }
    return false;
}

bool AttrValue::operator < (const AttrValue &val) const {
    if(this->isNull || val.isNull)
        return false;
    if ((type == AttrType::T_INT || type == AttrType::T_FLOAT) &&
        (val.type == AttrType::T_INT || val.type == AttrType::T_FLOAT)) {
        float f1 = this->type == AttrType::T_INT ? this->i : this->f;
        float f2 = val.type == AttrType::T_INT ? val.i : val.f;
        return f1 < f2;
    } else if (type == AttrType::T_STRING && val.type == AttrType::T_STRING) {
        return this->s < val.s;
    } else if (type == AttrType::T_DATE && val.type == AttrType::T_DATE) { 
        return this->s < val.s;
    } else {
        // TODO cannot compare
    }
    return false;
}

UseDatabaseTree::UseDatabaseTree(const char *dbName) {
    this->dbName = dbName;
}

void UseDatabaseTree::visit() {
    SystemManager::instance()->openDB(dbName.c_str());
}

DescTableTree::DescTableTree(const char *relName) {
    this->relName = string(relName);
}

DescTableTree::~DescTableTree() {

}

void DescTableTree::visit() {
    if(relName.empty())
        SystemManager::instance()->help();
    else
        SystemManager::instance()->help(relName.c_str());
}

InsertValueTree::InsertValueTree(ConstValuesTree *constValues) {
    values.push_back(constValues);
}

InsertValueTree::~InsertValueTree() {
    for(auto v : values) {
        delete v;
    }
}

void InsertValueTree::addConstValues(ConstValuesTree *constValuesTree) {
    values.push_back(constValuesTree);
}

OperatorTree::OperatorTree(CmpOP op):op(op){ }
OperatorTree::~OperatorTree(){}

SetClauseTree::SetClauseTree(){}
void SetClauseTree::addClause(const char *colName, ConstValueTree *constValue){
    clauses.push_back(make_pair(std::string(colName), constValue));
}
SetClauseTree::~SetClauseTree(){}

PrimarySetTree::PrimarySetTree(AttributesTree *attrs):attrs(attrs) {
    isPrimarySetTree = true;
}
PrimarySetTree::~PrimarySetTree(){}

AddPrimaryTree::AddPrimaryTree(const char *relName, AttributesTree* attrs){
    this->attrs.clear();
    for (const auto &s: attrs->getDescriptors())
        this->attrs.push_back(s.attrName);
    this->relName = string(relName);
}
AddPrimaryTree::~AddPrimaryTree(){}
void AddPrimaryTree::visit(){
    SystemManager::instance()->addPrimaryKey(relName.c_str(), attrs);
}

DropPrimaryTree::DropPrimaryTree(const char *relName){
    this->relName = string(relName);
}
DropPrimaryTree::~DropPrimaryTree(){}
void DropPrimaryTree::visit(){
    SystemManager::instance()->dropPrimaryKey(relName.c_str());
}

ForeignSetTree::ForeignSetTree(const char *fkName, const char *masRelName, AttributesTree *serAttrs, AttributesTree *masAttrs){
    isForeignSetTree = true;
    this->fkName = string(fkName);
    this->masRelName = string(masRelName);
    this->serAttrs = serAttrs;
    this->masAttrs = masAttrs;
}
ForeignSetTree::~ForeignSetTree(){}

DropForeignTree::DropForeignTree(const char *relName, const char *fkName){
    this->relName = string(relName);
    this->fkName = string(fkName);
}
DropForeignTree::~DropForeignTree(){}
void DropForeignTree::visit(){
    SystemManager::instance()->dropForeignKey(relName.c_str(), fkName.c_str());
}


AddForeignTree::AddForeignTree(const char *fkName, const char *serRelName, const char *masRelName, AttributesTree* serAttrs, AttributesTree* masAttrs){
    this->fkName = string(fkName);
    this->serRelName = string(serRelName);
    this->masRelName = string(masRelName);
    this->serAttrs = serAttrs->getDescriptors();
    this->masAttrs = masAttrs->getDescriptors();
}
AddForeignTree::~AddForeignTree(){}
void AddForeignTree::visit(){
    SystemManager::instance()->addForeignKey(fkName.c_str(), serRelName.c_str(), masRelName.c_str(), serAttrs, masAttrs);
}

RenameTree::RenameTree(const char *a, const char *b){
    this->a = string(a);
    this->b = string(b);
}
RenameTree::~RenameTree(){}
void RenameTree::visit(){
    SystemManager::instance()->RenameTable(a, b);
}

AddAttrTree::AddAttrTree(const char *relName, ColumnTree *attr){
    this->relName = string(relName);
    this->attr = attr;
}
AddAttrTree::~AddAttrTree(){}
void AddAttrTree::visit(){
    if (attr->isForeignSetTree || attr->isPrimarySetTree){
        cerr << "[ERROR] Invalid input." << endl;
        return;
    }
    SystemManager::instance()->AddAttr(relName, attr->getAttrInfo());
}


DropAttrTree::DropAttrTree(const char *relName, const char *attrName){
    this->relName = string(relName);
    this->attrName = string(attrName);
}
DropAttrTree::~DropAttrTree(){}
void DropAttrTree::visit(){
    SystemManager::instance()->DelAttr(relName, attrName);
}

AlterAttrTree::AlterAttrTree(const char *relName, const char *attrName, ColumnTree *attr){
    this->relName = string(relName);
    this->attrName = string(attrName);
    this->attr = attr;
}
AlterAttrTree::~AlterAttrTree(){}
void AlterAttrTree::visit(){
    if (attr->isForeignSetTree || attr->isPrimarySetTree){
        cerr << "[ERROR] Invalid input." << endl;
        return;
    }
    SystemManager::instance()->ModifyAttr(relName, attrName, attr->getAttrInfo());
}

ShowDatabaseTree::ShowDatabaseTree(){}
ShowDatabaseTree::~ShowDatabaseTree(){}
void ShowDatabaseTree::visit(){
    SystemManager::instance()->ShowDatabase();
}