#include <cstring>

#include "ComparisonEngine.h"

int ComparisonEngine::Run(Record *left, Record *literal, Comparison *comparison) {
    char *val1, *val2;
    char *leftBits = left->GetBits(), *literalBits = literal->GetBits();

    // First value to compare
    val1 = (comparison->operand1 == Left) ?
            (leftBits + ((int *) leftBits)[comparison->whichAttr1 + 1]) :
            (literalBits + ((int *) literalBits)[comparison->whichAttr1 + 1]);

    // Second value to compare
    val2 = (comparison->operand2 == Left) ?
            (leftBits + ((int *) leftBits)[comparison->whichAttr2 + 1]) :
            (literalBits + ((int *) literalBits)[comparison->whichAttr2 + 1]);

    switch (comparison->attrType) {
        case Int:{
            int int1 = *((int *) val1), int2 = *((int *) val2);
            if (comparison->compOp == LessThan) return int1 < int2;
            else if (comparison->compOp == GreaterThan) return int1 > int2;
            else return int1 == int2;
        }
        case Double:{
            double dbl1 = *((double *) val1), dbl2 = *((double *) val2);
            if (comparison->compOp == LessThan) return dbl1 < dbl2;
            else if (comparison->compOp == GreaterThan) return dbl1 > dbl2;
            else return dbl1 == dbl2;
        }
        case String:{
            int res = strcmp(val1, val2);
            if (comparison->compOp == LessThan) return res < 0;
            else if (comparison->compOp == GreaterThan) return res > 0;
            else return res == 0;
        }
    }
}

int ComparisonEngine::Run(Record *left, Record *right, Record *literal, Comparison *comparison) {
    char *val1, *val2;
    char *leftBits = left->GetBits(), *rightBits = right->GetBits(), *literalBits = literal->GetBits();

    // First value to compare
    val1 = (comparison->operand1 == Left) ?
           (leftBits + ((int *) leftBits)[comparison->whichAttr1 + 1]) :
           (comparison->operand1 == Right) ?
           (rightBits + ((int *) rightBits)[comparison->whichAttr1 + 1]) :
           (literalBits + ((int *) literalBits)[comparison->whichAttr1 + 1]);

    // Second value to compare
    val2 = (comparison->operand2 == Left) ?
           (leftBits + ((int *) leftBits)[comparison->whichAttr2 + 1]) :
           (comparison->operand2 == Right) ?
           (rightBits + ((int *) rightBits)[comparison->whichAttr2 + 1]) :
           (literalBits + ((int *) literalBits)[comparison->whichAttr2 + 1]);

    switch (comparison->attrType) {
        case Int:{
            int int1 = *((int *) val1), int2 = *((int *) val2);
            if (comparison->compOp == LessThan) return int1 < int2;
            else if (comparison->compOp == GreaterThan) return int1 > int2;
            else return int1 == int2;
        }
        case Double:{
            double dbl1 = *((double *) val1), dbl2 = *((double *) val2);
            if (comparison->compOp == LessThan) return dbl1 < dbl2;
            else if (comparison->compOp == GreaterThan) return dbl1 > dbl2;
            else return dbl1 == dbl2;
        }
        case String:{
            int res = strcmp(val1, val2);
            if (comparison->compOp == LessThan) return res < 0;
            else if (comparison->compOp == GreaterThan) return res > 0;
            else return res == 0;
        }
    }
}

int ComparisonEngine::Compare(Record *left, Record *right, OrderMaker *orderMaker) {
    char *val1, *val2;
    char *leftBits = left->GetBits(), *rightBits = right->GetBits();

    for (int index = 0; index < orderMaker->numberOfAttrs; index++) {
        val1 = leftBits + ((int *) leftBits)[orderMaker->whichAttrs[index] + 1];
        val2 = rightBits + ((int *) rightBits)[orderMaker->whichAttrs[index] + 1];

        switch (orderMaker->whichTypes[index]) {
            case Int: {
                    int int1 = *((int *) val1), int2 = *((int *) val2);
                    if (int1 < int2) return -1;
                    else if (int1 > int2) return 1;
                }
                break;
            case Double: {
                    double dbl1 = *((double *) val1), dbl2 = *((double *) val2);
                    if (dbl1 < dbl2) return -1;
                    else if (dbl1 > dbl2) return 1;
                }
                break;
            case String: {
                    int res = strcmp(val1, val2);
                    if (res != 0) return res;
                }
                break;
        }
    }

    return 0;
}

int ComparisonEngine::Compare(Record *left, OrderMaker *orderLeft, Record *right, OrderMaker *orderRight) {
    char *val1, *val2;
    char *leftBits = left->GetBits(), *rightBits = right->GetBits();

    for (int index = 0; index < orderLeft->numberOfAttrs; index++) {
        val1 = leftBits + ((int *) leftBits)[orderLeft->whichAttrs[index] + 1];
        val2 = rightBits + ((int *) rightBits)[orderRight->whichAttrs[index] + 1];

        switch (orderLeft->whichTypes[index]) {
            case Int: {
                int int1 = *((int *) val1), int2 = *((int *) val2);
                if (int1 < int2) return -1;
                else if (int1 > int2) return 1;
            }
                break;
            case Double: {
                double dbl1 = *((double *) val1), dbl2 = *((double *) val2);
                if (dbl1 < dbl2) return -1;
                else if (dbl1 > dbl2) return 1;
            }
                break;
            case String: {
                int res = strcmp(val1, val2);
                if (res != 0) return res;
            }
                break;
        }
    }

    return 0;
}

int ComparisonEngine::Compare(Record *left, Record *literal, CNF *cnf) {
    for (int i = 0; i < cnf->numberOfAnds; i++) {
        for (int j = 0; j < cnf->orLens[i]; j++) {
            // this returns a 0 if the comparison did not eval to true
            int result = Run(left, literal, &cnf->orList[i][j]);
            if (result != 0) break;

            // if we made it through all of the comparisons without a hit, return a 0
            if (j == cnf->orLens[i] - 1) return 0;
        }

        // this ends the for-loop for the OR-list... if we made it this far, then we'll go on to the
        // next disjunction in the overall CNF expression
    }

    // this ends the for-loop for the AND-list... if we made it this far, then we know that we
    // have satisfied the overall CNF expression
    return 1;
}

int ComparisonEngine::Compare(Record *left, Record *right, Record *literal, CNF *cnf) {
    for (int i = 0; i < cnf->numberOfAnds; i++) {
        for (int j = 0; j < cnf->orLens[i]; j++) {
            // this returns a 0 if the comparison did not eval to true
            int result = Run (left, right, literal, &cnf->orList[i][j]);
            if (result != 0) break;

            // if we made it through all of the comparisons without a hit, return a 0
            if (j == cnf->orLens[i] - 1) return 0;
        }

        // this ends the for-loop for the OR-list... if we made it this far, then we'll go on to the
        // next disjunction in the overall CNF expression
    }

    // this ends the for-loop for the AND-list... if we made it this far, then we know that we
    // have satisfied the overall CNF expression
    return 1;
}