#ifndef SQLIKE_COMPARISON_H
#define SQLIKE_COMPARISON_H

#include <string>

#include "Const.h"
#include "ParseTree.h"
#include "Record.h"
#include "Schema.h"

using namespace std;

class Comparison {
public:
    int whichAttr1, whichAttr2;
    string whichAttrVal1, whichAttrVal2;
    Target operand1, operand2;

    Type attrType;

    CompOperator compOp;

    Comparison() = default;

    Comparison(const Comparison &copyMe);

    void Print();

    void PrintStr();
};

// This structure encapsulates a sort order for records
class OrderMaker {
public:
    int numberOfAttrs;

    int whichAttrs[MAX_ANDS];
    Type whichTypes[MAX_ANDS];

    OrderMaker();

    // Creates an OrderMaker that can be used to sort the records based upon all of their attributes.
    explicit OrderMaker(Schema &schema);

    void Print();

    // Converts this order-maker instance to string.
    string ToString();

    // Creates order-maker instance from string.
    void FromString(string src);
};

class CNF {
public:
    Comparison orList[MAX_ANDS][MAX_ORS];

    int orLens[MAX_ANDS];

    int numberOfAnds;

    /*
     * Returns an instance of the OrderMaker class that allows the CNF to be implemented using a
     * sort-based algorithm such as sort-merge-join.
     * Returns 0 if and only if it is impossible to determine an acceptable ordering for the
     * given comparison.
     */
    int GetSortOrder(OrderMaker &left, OrderMaker &right);

    /*
     * Creates a 2-D matrix storing the CNF expression, from a given parse tree
     * for a CNF. Useful for single relation expression.
     */
    void GrowFromParseTree(AndList *parseTree, Schema &schema, Record &literal);

    /*
     * Creates a 2-D matrix storing the CNF expression, from a given parse tree
     * for a CNF. Applicable when two relations are involved in a CNF.
     */
    void GrowFromParseTree(AndList *parseTree, Schema &left, Schema &right, Record &literal);

    void Print();
};

#endif //SQLIKE_COMPARISON_H
