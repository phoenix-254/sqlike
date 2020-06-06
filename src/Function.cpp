#include <iostream>

#include "Function.h"

using namespace std;

Type Function::RecursivelyBuild(struct FuncOperator *parseTree, Schema &schema) {
    // Case 1: unary operation
    if (parseTree->right == nullptr && parseTree->leftOperand == nullptr && parseTree->code == '-') {
        // Figure out the type of operation
        Type type = RecursivelyBuild(parseTree->leftOperator, schema);

        switch (type) {
            case Int: {
                opList[opListSize++].op = IntUnaryMinus;
                return Int;
            }
            case Double: {
                opList[opListSize++].op = DblUnaryMinus;
                return Double;
            }
            default: {
                cout << "Invalid type!" << endl;
                exit(1);
            }
        }
    }
    // Case 2: we have either a literal value or a variable value, so do a push
    else if (parseTree->leftOperator == nullptr && parseTree->right == nullptr) {
        // Sub case 1: the value if from the record that we are operating over, so we'll find it in the schema.
        if (parseTree->leftOperand->code == NAME) {
            int index = schema.FindIndex(parseTree->leftOperand->value);
            if (index == -1) {
                cout << "Error! Attribute " << parseTree->leftOperand->value
                     << " of Arithmetic expression is not present in schema." << endl;
                exit(1);
            }

            int type = schema.FindType(parseTree->leftOperand->value);
            switch (type) {
                case String: {
                    cout << "Error! Arithmetic operations over string is not allowed." << endl;
                    exit(1);
                }
                case Int: {
                    opList[opListSize].op = PushInt;
                    opList[opListSize].recordInput = index;
                    opList[opListSize++].literalInput = nullptr;
                    return Int;
                }
                case Double: {
                    opList[opListSize].op = PushDouble;
                    opList[opListSize].recordInput = index;
                    opList[opListSize++].literalInput = nullptr;
                    return Double;
                }
                default: {
                    cout << "Invalid type!" << endl;
                    exit(1);
                }
            }
        }
        // Sub case 2: we have literal int value.
        else if (parseTree->leftOperand->code == INT) {
            opList[opListSize].op = PushInt;
            opList[opListSize].recordInput = -1;
            opList[opListSize].literalInput = (void *) (new int);
            *((int *) opList[opListSize++].literalInput) = atoi(parseTree->leftOperand->value);
            return Int;
        }
        // Sub case 3: we have literal double value.
        else {
            opList[opListSize].op = PushDouble;
            opList[opListSize].recordInput = -1;
            opList[opListSize].literalInput = (void *) (new double);
            *((double *) opList[opListSize++].literalInput) = atof(parseTree->leftOperand->value);
            return Double;
        }
    }
    // Case 3: Arithmetic op
    else {
        // Recursively handle the left operator; this should give us the left side's value, sitting on
        // top of the stack.
        Type typeLeft = RecursivelyBuild(parseTree->leftOperator, schema);

        // do same for right operator.
        Type typeRight = RecursivelyBuild(parseTree->right, schema);

        /*
         * The two values on which operation is to be performed are sitting on top of the stack and are int.
         */
        if (typeLeft == Int && typeRight == Int) {
            if (parseTree->code == '+') {
                opList[opListSize++].op = IntPlus;
            } else if (parseTree->code == '-') {
                opList[opListSize++].op = IntMinus;
            } else if (parseTree->code == '*') {
                opList[opListSize++].op = IntMultiply;
            } else if (parseTree->code == '/') {
                opList[opListSize++].op = IntDivide;
            } else {
                cout << "Invalid operation!" << endl;
                exit(1);
            }
            return Int;
        }

        // If we reach here then at least one of the type is double, ans hence we need to cast int to double.
        if (typeLeft == Int) {
            opList[opListSize++].op = ToDouble2Down;
        }
        if (typeRight == Int) {
            opList[opListSize++].op = ToDouble;
        }

        /*
         * The two values on which operation is to be performed are sitting on top of the stack and are double.
         */
        if (parseTree->code == '+') {
            opList[opListSize++].op = DblPlus;
        } else if (parseTree->code == '-') {
            opList[opListSize++].op = DblMinus;
        } else if (parseTree->code == '*') {
            opList[opListSize++].op = DblMultiply;
        } else if (parseTree->code == '/') {
            opList[opListSize++].op = DblDivide;
        } else {
            cout << "Invalid operation!" << endl;
            exit(1);
        }
        return Double;
    }
}

Function::Function() {
    opListSize = 0;
    opList = new Arithmetic[MAX_DEPTH];
    doesReturnInt = false;
}

void Function::GrowFromParseTree(struct FuncOperator *parseTree, Schema &schema) {
    opListSize = 0;

    // Recursively build the list
    Type resType = RecursivelyBuild(parseTree, schema);

    doesReturnInt = resType == Int;
}

// Loop through all of the operations that are specified during the function creation and perform them.
Type Function::Apply(Record &toMe, int &intResult, double &doubleResult) {
    // Stack to hold intermediate results from the function.
    double stack[MAX_DEPTH];
    double *lastPos = stack - 1;

    char *bits = toMe.GetBits();

    for (int i = 0; i < opListSize; i++) {
        switch (opList[i].op) {
            case PushInt:
                lastPos++;
                // see if we need to get the int from the record
                if (opList[i].recordInput >= 0) {
                    int pointer = ((int *) toMe.GetBits())[opList[i].recordInput + 1];
                    *((int *) lastPos) = *((int *) &(bits[pointer]));
                } else { // or from the literal value
                    *((int *) lastPos) = *((int *) opList[i].literalInput);
                }
                break;

            case PushDouble:
                lastPos++;
                // see if we need to get the int from the record
                if (opList[i].recordInput >= 0) {
                    int pointer = ((int *) toMe.GetBits())[opList[i].recordInput + 1];
                    *((double *) lastPos) = *((double *) &(bits[pointer]));
                } else { // or from the literal value
                    *((double *) lastPos) = *((double *) opList[i].literalInput);
                }
                break;

            case ToDouble:
                *((double *) lastPos) = *((int *) lastPos);
                break;

            case ToDouble2Down:
                *((double *) (lastPos - 1)) = *((int *) (lastPos - 1));
                break;

            case IntUnaryMinus:
                *((int *) lastPos) = -(*((int *) lastPos));
                break;

            case DblUnaryMinus:
                *((double *) lastPos) = -(*((double *) lastPos));
                break;

            case IntMinus:
                *((int *) (lastPos - 1)) = *((int *) (lastPos - 1)) - *((int *) lastPos);
                lastPos--;
                break;

            case DblMinus:
                *((double *) (lastPos - 1)) = *((double *) (lastPos - 1)) - *((double *) lastPos);
                lastPos--;
                break;

            case IntPlus:
                *((int *) (lastPos - 1)) = *((int *) (lastPos - 1)) + *((int *) lastPos);
                lastPos--;
                break;

            case DblPlus:
                *((double *) (lastPos - 1)) = *((double *) (lastPos - 1)) + *((double *) lastPos);
                lastPos--;
                break;

            case IntDivide:
                *((int *) (lastPos - 1)) = *((int *) (lastPos - 1)) / *((int *) lastPos);
                lastPos--;
                break;

            case DblDivide:
                *((double *) (lastPos - 1)) = *((double *) (lastPos - 1)) / *((double *) lastPos);
                lastPos--;
                break;

            case IntMultiply:
                *((int *) (lastPos - 1)) = *((int *) (lastPos - 1)) * *((int *) lastPos);
                lastPos--;
                break;

            case DblMultiply:
                *((double *) (lastPos - 1)) = *((double *) (lastPos - 1)) * *((double *) lastPos);
                lastPos--;
                break;

            default:
                cerr << "Had a function operation I did not recognize!\n";
                exit (1);
        }
    }

    // now, we are just about done.  First we have a sanity check to make sure that exactly one value is on the stack!
    if (lastPos != stack) {
        cerr << "During function evaluation, we did not have exactly one value ";
        cerr << "left on the stack.  BAD!\n";
        exit (1);
    }

    // got here, so we are good to go; just return the final value
    if (doesReturnInt) {
        intResult = *((int *) lastPos);
        return Int;
    } else {
        doubleResult = *((double *) lastPos);
        return Double;
    }
}

void Function::Print() {

}