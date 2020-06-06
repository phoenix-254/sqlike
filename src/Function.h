#ifndef SQLIKE_FUNCTION_H
#define SQLIKE_FUNCTION_H

#include "ParseTree.h"
#include "Record.h"

#define MAX_DEPTH 100

enum ArithmeticOp {
    PushInt, PushDouble, ToDouble, ToDouble2Down,
    IntUnaryMinus, IntMinus, IntPlus, IntDivide, IntMultiply,
    DblUnaryMinus, DblMinus, DblPlus, DblDivide, DblMultiply
};

struct Arithmetic {
    ArithmeticOp op;
    int recordInput;
    void *literalInput;
};

class Function {
private:
    int opListSize;
    Arithmetic *opList;

    bool doesReturnInt;

    // Helper function for GrowFromParseTree
    Type RecursivelyBuild(struct FuncOperator *parseTree, Schema &schema);

public:
    Function();

    ~Function() = default;

    /*
     * Grows the specified function from a parse tree and converts it into an accumulator-based
     * computation over the attributes in a record with the given schema; the record "literal" is
     * produced by this method.
     */
    void GrowFromParseTree(struct FuncOperator *parseTree, Schema &schema);

    // Applies the function to the given record and returns the result
    Type Apply (Record &toMe, int &intResult, double &doubleResult);

    void Print();
};

#endif //SQLIKE_FUNCTION_H
