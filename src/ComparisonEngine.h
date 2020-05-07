#ifndef SQLIKE_COMPARISONENGINE_H
#define SQLIKE_COMPARISONENGINE_H

#include "Comparison.h"
#include "Record.h"

class ComparisonEngine {
private:
    static int Run(Record *left, Record *literal, Comparison *comparison);
    static int Run(Record *left, Record *right, Record *literal, Comparison *comparison);

public:
    /*
     * Compare method used in sorting when both the records come from the SAME RELATION.
     * orderMaker - encapsulates the specification for a sort order.
     *
     * e.g - You are joining two tables on left.attr1 = right.attr2, and so you want to sort left using attr1,
     * so the orderMaker given here specifies this sort ordering on attr1.
     *
     * This methods returns a negative number, a zero, or a positive number indicating "left < right",
     * "left = right" and "left > right" respectively.
     */
    int Compare(Record *left, Record *right, OrderMaker *orderMaker);

    /*
     * Similar to the above version except that this methods is used in the case when two records
     * come from the different input relations, used to do sorts for a sort-merge join.
     */
    int Compare(Record *left, OrderMaker *orderLeft, Record *right, OrderMaker *orderRight);

    // like the last one, but for unary operations

    /*
     * Applied the given CNF to the records and either accepts the records or rejects them.
     * Used for Unary operations like SUM.
     * Returns 0 if the given CNF evaluates to false over the record.
     */
    int Compare(Record *left, Record *literal, CNF *cnf);

    // Similar to the above, but used for Binary operations like JOIN.
    int Compare(Record *left, Record *right, Record *literal, CNF *cnf);
};

#endif //SQLIKE_COMPARISONENGINE_H
