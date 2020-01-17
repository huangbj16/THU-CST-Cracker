#ifndef DATABASE_PRINTER_H
#define DATABASE_PRINTER_H

#include <vector>
#include "../global.h"
#include "../parser/Tree.h"
#include "RecordDescriptor.h"


class Printer {
public:
    Printer() {}
    ~Printer(){}
    static void printHeader(std::vector<AttributeTree::AttributeDescriptor> attrs, bool limitWidth = false, std::ostream& c = std::cout);
    static void printAll(std::vector<RecordDescriptor> recordDescriptors, std::vector<AttributeTree::AttributeDescriptor> *attrs = NULL, std::ostream& c = std::cout);
    static void printFooter(std::ostream& c = std::cout);
    static std::vector<int> widths;

private:
    static void printBar(std::ostream &c = std::cout);
    static int attrLenSum;
};


#endif //DATABASE_PRINTER_H
