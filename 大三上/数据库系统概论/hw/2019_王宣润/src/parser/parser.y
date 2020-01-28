%{
#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#include <cstdio>
#define __STRICT_ANSI__
#else
#include <cstdio>
#endif

#include <stdlib.h>
#include <string>
#include "../global.h"
#include "tree.h"

int yylex();
void yyerror(const char *);

class Tree;
%}

%union {
    int ivalue;
    float fvalue;
    char *string;
    Tree *tree;
    SelectTree *selectTree;
    DeleteTree *deleteTree;
    AttributesTree *attributesTree;
    AttributeTree *attributeTree;
    RelationsTree *relationsTree;
    WhereClauseTree *whereClauseTree;
    ConditionsTree *conditionsTree;
    ComparisonTree *comparisonTree;
    ConstValuesTree *constValuesTree;
    ConstValueTree *constValueTree;
    ColumnsTree *columnsTree;
    ColumnTree *columnTree;
    InsertValueTree *insertValueTree;
    TypeTree *typeTree;
    SetClauseTree *setClauseTree;
    OperatorTree *operatorTree;
    PrimarySetTree *primarySetTree;
    AddPrimaryTree *addPrimaryTree;
    DropPrimaryTree *dropPrimaryTree;
    ForeignSetTree *foreignSetTree;
    AddForeignTree *addForeignTree;
    RenameTree *renameTree;
}


/* keyword */
%token SELECT DELETE UPDATE INSERT
%token CREATE DROP USE SHOW TABLES
%token DATABASE TABLE
%token STAR FROM WHERE OPERATOR VALUES SET INTO
%token DATABASES DEFAULT CONSTRAINT CHANGE ALTER 
%token ADD RENAME REFERENCES FOREIGN ON TO
%token FORCE


/* COLUMN DESCPRITION */
%token KINT KFLOAT KVARCHAR KDATE 


/* number */
%token <ivalue> INTEGER
%token <fvalue> FLOAT
%token <string> STRING IDENTIFIER DATE


/* operator */
%token EQ GT LT GE LE NE
%token NOTNULL PRIMARY DESC INDEX T_NULL IS AND

%type <string>              dbName tbName fkName pkName colName idxName
%type <tree>                program stmt sysStmt dbStmt tbStmt idxStmt alterStmt
%type <attributesTree>      colList columnList
%type <attributeTree>       col
%type <relationsTree>       tableList
%type <whereClauseTree>     whereClause
%type <conditionsTree>      conditions
%type <comparisonTree>      comparisonClause
%type <constValuesTree>     valueList
%type <constValueTree>      value
%type <columnsTree>         fieldList
%type <columnTree>          field
%type <typeTree>            type
%type <insertValueTree>     valueLists
%type <operatorTree>        op
%type <setClauseTree>       setClause

%%

program:
    stmt
        { }
    | program stmt
        { }
    ;

stmt:
    sysStmt ';'
        { }
    | dbStmt ';'
        { }
    | tbStmt ';'
        { }
    | idxStmt ';'
        { }
    | alterStmt ';'
        { }
    | ';'
        { }
    ;
    
sysStmt:
    SHOW DATABASES
        {
            $$ = new ShowDatabaseTree();
            Tree::setInstance($$);
            Tree::run();
        }
    ;

dbStmt:
    CREATE DATABASE dbName
        {
            $$ = new CreateDatabaseTree($3);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | DROP DATABASE dbName
        {
            $$ = new DropDatabaseTree($3);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | USE dbName
        {
            $$ = new UseDatabaseTree($2);
            Tree::setInstance($$);
            delete $2;
            Tree::run();
        }
    | SHOW TABLES
        {
            $$ = new DescTableTree("");
            Tree::setInstance($$);
            Tree::run();
        }
    ;

tbStmt:
    CREATE TABLE tbName '(' fieldList ')'
        {
            $$ = new CreateTableTree($3, $5);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | DROP TABLE tbName
        {
            $$ = new DropTableTree($3);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | DESC tbName
        {
            $$ = new DescTableTree($2);
            Tree::setInstance($$);
            delete $2;
            Tree::run();
        }
    | INSERT INTO tbName VALUES valueLists
        {
            $$ = new InsertTree($3, $5);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | INSERT INTO tbName '(' columnList ')' VALUES valueLists
        {
            $$ = new InsertTree($3, $5, $8);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | INSERT INTO tbName VALUES valueLists FORCE
        {
            $$ = new InsertTree($3, $5, true);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | INSERT INTO tbName '(' columnList ')' VALUES valueLists FORCE
        {
            $$ = new InsertTree($3, $5, $8, true);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | DELETE FROM tbName whereClause
        {
            $$ = new DeleteTree($3, $4);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | UPDATE tbName SET setClause whereClause
        {
            // 这里不考虑WhereClause，因为update加上whereclause后就不能逐列修改了
            $$ = new UpdateTree(std::string($2), $4, $5); 
            Tree::setInstance($$);
            delete $2;
            Tree::run();
        }
    | SELECT STAR FROM tableList whereClause
        {
            $$ = new SelectTree($4, $5);
            Tree::setInstance($$);
            Tree::run();
        }
    | SELECT colList FROM tableList whereClause
        {
            $$ = new SelectTree($2, $4, $5);
            Tree::setInstance($$);
            Tree::run();
        }
    ;

idxStmt:
    CREATE INDEX idxName ON tbName '(' columnList ')'
        {
            $$ = new CreateIndexTree($3, $5, $7);
            Tree::setInstance($$);
            delete $3;
            delete $5;
            Tree::run();
        }
    | DROP INDEX idxName ON tbName
        {
            $$ = new DropIndexTree($3, $5);
            Tree::setInstance($$);
            delete $3;
            delete $5;
            Tree::run();
        }
    | ALTER TABLE tbName ADD INDEX idxName '(' columnList ')'
        {
            $$ = new CreateIndexTree($6, $3, $8);
            Tree::setInstance($$);
            delete $3;
            delete $6;
            Tree::run();
        }
    | ALTER TABLE tbName DROP INDEX idxName
        {
            $$ = new DropIndexTree($6, $3);
            Tree::setInstance($$);
            delete $3;
            delete $6;
            Tree::run();
        }
    ;
    
alterStmt:
    ALTER TABLE tbName ADD field
        {
            $$ = new AddAttrTree($3, $5);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | ALTER TABLE tbName DROP colName
        {
            $$ = new DropAttrTree($3, $5);
            Tree::setInstance($$);
            delete $3;
            delete $5;
            Tree::run();
        }
    | ALTER TABLE tbName CHANGE colName field
        {
            $$ = new AlterAttrTree($3, $5, $6);
            Tree::setInstance($$);
            delete $3;
            delete $5;
            Tree::run();
        }
    | ALTER TABLE tbName RENAME TO tbName
        {
            $$ = new RenameTree($3, $6);
            Tree::setInstance($$);
            delete $3;
            delete $6;
            Tree::run();
        }
    | ALTER TABLE tbName ADD PRIMARY '(' columnList ')'
        {
            $$ = new AddPrimaryTree($3, $7);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | ALTER TABLE tbName DROP PRIMARY
        {
            $$ = new DropPrimaryTree($3);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | ALTER TABLE tbName ADD CONSTRAINT pkName PRIMARY '(' columnList ')'
        {
            $$ = new AddPrimaryTree($3, $9);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | ALTER TABLE tbName DROP PRIMARY pkName
        {
            $$ = new DropPrimaryTree($3);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | ALTER TABLE tbName ADD CONSTRAINT fkName FOREIGN '(' columnList ')' REFERENCES tbName '(' columnList ')'
        {
            $$ = new AddForeignTree($6, $3, $12, $9, $14);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    | ALTER TABLE tbName DROP FOREIGN fkName
        {
            $$ = new DropForeignTree($3, $6);
            Tree::setInstance($$);
            delete $3;
            Tree::run();
        }
    ;

fieldList:
    field
        {
            $$ = new ColumnsTree();
            $$->addColumn($1);
        }
    | fieldList ',' field
        {
            $$->addColumn($3);
        }
    ;

field:
    colName type
        {
            $$ = new ColumnTree($1, $2->getType(), $2->getLen());
            delete $1;
            delete $2;
        }
    | colName type NOTNULL
        {
            $$ = new ColumnTree($1, $2->getType(), $2->getLen(), 1);
            delete $1;
            delete $2;
        }
    | colName type DEFAULT value
        {
            $$ = new ColumnTree($1, $2->getType(), $2->getLen(), 0, $4);
            delete $1;
            delete $2;
        }
    | colName type NOTNULL DEFAULT value
        {
            $$ = new ColumnTree($1, $2->getType(), $2->getLen(), 1, $5);
            delete $1;
            delete $2;
        }
    | PRIMARY '(' columnList ')'
        {
            // 这里检查columnList里出现的所有名称是否在fieldList里出现过
            $$ = new PrimarySetTree($3);
        }
    | FOREIGN fkName '(' columnList ')' REFERENCES tbName '(' columnList ')'
        {
            $$ = new ForeignSetTree($2, $7, $4, $9);
        }
    ;

type:
    KINT
        {
            $$ = new TypeTree(AttrType::T_INT);
        }
    | KINT '(' INTEGER ')'
        {
            $$ = new TypeTree(AttrType::T_INT, $3);
        }
    | KVARCHAR '(' INTEGER ')'
        {
            $$ = new TypeTree(AttrType::T_STRING, $3);
        }
    | KDATE
        {
            $$ = new TypeTree(AttrType::T_DATE, 15);
        }
    | KFLOAT
        {
            $$ = new TypeTree(AttrType::T_FLOAT);
        }
    ;

valueLists:
    '(' valueList ')'
        {
            $$ = new InsertValueTree($2);
        }
    | valueLists ',' '(' valueList ')'
        {
            $$->addConstValues($4);
        }
    ;

valueList:
    value
        {
            $$ = new ConstValuesTree();
            $$->addConstValue($1);
        }
    | valueList ',' value
        {
            $$->addConstValue($3);
        }
    ;

value:
    /*
        如果是一个合法日期，则输入的时候会识别为DATE
        日期以||括起来，字符串以单引号括起来
    */
    INTEGER
        {
            $$ = new ConstValueTree($1);
        }
    | STRING
        {
            $$ = new ConstValueTree($1);
            delete $1;
        }
    | FLOAT
        {
            $$ = new ConstValueTree($1);
        }
    | DATE
        {
            $$ = new ConstValueTree($1, true);
            delete $1;
        }
    | T_NULL
        {
            $$ = new ConstValueTree(0);
            $$->setNull();
        }
    ;

comparisonClause:
    col op col
        {
            $$ = new ComparisonTree($1, $2->op, $3);
        }
    | col op value
        {
            $$ = new ComparisonTree($1, $2->op, $3);
        }
    | col IS T_NULL
        {
            $$ = new ComparisonTree($1, CmpOP::T_ISNULL);
        }
    | col IS NOTNULL
        {
            $$ = new ComparisonTree($1, CmpOP::T_ISNOTNULL);
        }
    ;


whereClause:
    WHERE conditions
        {
            $$ = new WhereClauseTree($2);
        }
    |
        {
            $$ = nullptr;
        }
    ;

conditions:
    comparisonClause
        {
            $$ = new ConditionsTree();
            $$->addComparison($1);
        }
    | conditions AND comparisonClause
        {
            $$->addComparison($3);
        }
    ;

col:
    tbName '.' colName
        {
            $$ = new AttributeTree($1, $3);
            delete $1;
            delete $3;
        }
    | colName
        {
            $$ = new AttributeTree($1);
            delete $1;
        }
    ;

op:
    EQ
        {
            $$ = new OperatorTree(CmpOP::T_EQ);
        }
    | NE
        {
            $$ = new OperatorTree(CmpOP::T_NE);
        }
    | GT
        {
            $$ = new OperatorTree(CmpOP::T_GT);
        }
    | LT
        {
            $$ = new OperatorTree(CmpOP::T_LT);
        }
    | GE
        {
            $$ = new OperatorTree(CmpOP::T_GE);
        }
    | LE
        {
            $$ = new OperatorTree(CmpOP::T_LE);
        }
    ;

setClause:
    colName EQ value
        {
            $$ = new SetClauseTree();
            $$->addClause($1, $3);
        }
    | setClause ',' colName EQ value
        {
            $$->addClause($3, $5);
        }
    ;

colList:
    col
        {
            $$ = new AttributesTree();
            $$->addAttribute($1);
        }
    | colList ',' col
        {
            $$->addAttribute($3);
        }
    ;

tableList:
    tbName
        {
            $$ = new RelationsTree();
            $$->addRelation($1);
            delete $1;
        }
    | tableList ',' tbName
        {
            $$->addRelation($3);
            delete $3;
        }
    ;

/*
    columnList 中的column项不能带表名
    colList 中的col项可以带表名
*/
columnList:
    colName
        {
            $$ = new AttributesTree();
            $$->addAttribute(new AttributeTree($1));
        }
    | columnList ',' colName
        {
            $$->addAttribute(new AttributeTree($3));
        }
    ;

dbName:
    IDENTIFIER
        { }
    ;

tbName:
    IDENTIFIER
        { }
    ;

fkName:
    IDENTIFIER
        { }
    ;

pkName:
    IDENTIFIER
        { }
    ;

idxName:
    IDENTIFIER
        { }
    ;

colName:
    IDENTIFIER
        { }
    ;

%%

void yyerror(const char *msg) {
    printf("YACC error: %s\n", msg);
}
