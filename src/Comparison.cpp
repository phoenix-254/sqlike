#include <cstring>
#include <iostream>

#include "Comparison.h"

using namespace std;

Comparison::Comparison(const Comparison &copyMe) {
    whichAttr1 = copyMe.whichAttr1;
    whichAttr2 = copyMe.whichAttr2;

    whichAttrVal1 = copyMe.whichAttrVal1;
    whichAttrVal2 = copyMe.whichAttrVal2;

    operand1 = copyMe.operand1;
    operand2 = copyMe.operand2;

    attrType = copyMe.attrType;

    compOp = copyMe.compOp;
}

void Comparison::Print() {
    cout << "Att " << whichAttr1 << " from ";

    if (operand1 == Left) cout << "left record";
    else if (operand1 == Right) cout << "right record";
    else cout << "literal record ";

    if (compOp == LessThan) cout << " < ";
    else if (compOp == GreaterThan) cout << " > ";
    else cout << " = ";

    cout << "Att " << whichAttr2 << " from ";

    if (operand2 == Left) cout << "left record";
    else if (operand2 == Right) cout << "right record";
    else cout << "literal record";

    if (attrType == Int) cout << " (Int)";
    else if (attrType == Double) cout << " (Double)";
    else cout << " (String)";
}

void Comparison::PrintStr() {
    cout << whichAttrVal1;
    if (compOp == LessThan) cout << " < ";
    else if (compOp == GreaterThan) cout << " > ";
    else cout << " = ";
    cout << whichAttrVal2;
}

OrderMaker::OrderMaker() : numberOfAttrs(0) {}

OrderMaker::OrderMaker(Schema &schema) {
    numberOfAttrs = 0;

    vector<Attribute> attrs = schema.GetAttributes();

    for (int index = 0; index < attrs.size(); index++) {
        if (attrs[index].type == Int) {
            whichAttrs[index] = index;
            whichTypes[index] = Int;
            numberOfAttrs++;
        }
    }

    for (int index = 0; index < attrs.size(); index++) {
        if (attrs[index].type == Double) {
            whichAttrs[index] = index;
            whichTypes[index] = Double;
            numberOfAttrs++;
        }
    }

    for (int index = 0; index < attrs.size(); index++) {
        if (attrs[index].type == String) {
            whichAttrs[index] = index;
            whichTypes[index] = String;
            numberOfAttrs++;
        }
    }
}

void OrderMaker::Print() {
    cout << "Number of Attributes = " << numberOfAttrs << endl;
    for (int index = 0; index < numberOfAttrs; index++) {
        cout << whichAttrs[index];
        if (whichTypes[index] == Int) cout << "\t" << "[Int]" << endl;
        else if (whichTypes[index] == Double) cout << "\t" << "[Double]" << endl;
        else if (whichTypes[index] == String) cout << "\t" << "[String]" << endl;
    }
}

string OrderMaker::ToString() {
    string res;
    res += to_string(numberOfAttrs);
    for (int i = 0; i < numberOfAttrs; i++) {
        res += "," + to_string(whichAttrs[i]) + ":" + to_string(whichTypes[i]);
    }
    return res;
}

void OrderMaker::FromString(string src) {
    string data;

    int index = 0;
    for (; index < src.size(); index++) {
        if (src[index] == ',') {
            index++;
            break;
        } else {
            data += src[index];
        }
    }

    this->numberOfAttrs = stoi(data);

    data = "";
    int attrIndex = 0;
    for (; index < src.size(); index++) {
        if (src[index] == ':') {
            whichAttrs[attrIndex] = stoi(data);
            data = "";
        } else if (src[index] == ',') {
            whichTypes[attrIndex++] = Type(stoi(data));
            data = "";
        } else {
            data += src[index];
        }
    }
    whichTypes[attrIndex] = Type(stoi(data));
}

int CNF::GetSortOrder(OrderMaker &left, OrderMaker &right) {
    // Initialize the size of the OrderMakers
    left.numberOfAttrs = right.numberOfAttrs = 0;

    // loop through all of the disjunctions in the CNF and find those
    // that are acceptable for use in a sort ordering
    for (int i = 0; i < numberOfAnds; i++) {
        // if we don't have a disjunction of length one, then it
        // can't be acceptable for use with a sort ordering
        if (orLens[i] != 1) continue;

        // made it this far, so first verify that it is an equality check
        if (orList[i][0].compOp != Equals) continue;

        // now verify that it operates over attributes from both tables
        if (!((orList[i][0].operand1 == Left && orList[i][0].operand2 == Right) ||
              (orList[i][0].operand2 == Left && orList[i][0].operand1 == Right))) {
            //continue;
        }

        // since we are here, we have found a join attribute!!!
        // so all we need to do is add the new comparison info into the
        // relevant structures
        if (orList[i][0].operand1 == Left) {
            left.whichAttrs[left.numberOfAttrs] = orList[i][0].whichAttr1;
            left.whichTypes[left.numberOfAttrs] = orList[i][0].attrType;
        }

        if (orList[i][0].operand1 == Right) {
            right.whichAttrs[right.numberOfAttrs] = orList[i][0].whichAttr1;
            right.whichTypes[right.numberOfAttrs] = orList[i][0].attrType;
        }

        if (orList[i][0].operand2 == Left) {
            left.whichAttrs[left.numberOfAttrs] = orList[i][0].whichAttr2;
            left.whichTypes[left.numberOfAttrs] = orList[i][0].attrType;
        }

        if (orList[i][0].operand2 == Right) {
            right.whichAttrs[right.numberOfAttrs] = orList[i][0].whichAttr2;
            right.whichTypes[right.numberOfAttrs] = orList[i][0].attrType;
        }

        // note that we have found two new attributes
        left.numberOfAttrs++;
        right.numberOfAttrs++;
    }

    return left.numberOfAttrs;
}

// Helper function that writes out another field for the literal record and its schema
void AddLiteralToFile(int &numberOfFieldsInRec, FILE *outputRecFile,
                     FILE *outputSchemaFile, char *value, Type type) {
    // First, write out the new record field.
    fprintf(outputRecFile, "%s|", value);

    // Now, write out the new schema field.
    if (type == Int) fprintf(outputSchemaFile, "att%d Int\n", numberOfFieldsInRec);
    else if (type == Double) fprintf(outputSchemaFile, "att%d Double\n", numberOfFieldsInRec);
    else if (type == String) fprintf(outputSchemaFile, "att%d String\n", numberOfFieldsInRec);
    else {
        cerr << "Invalid type!" << endl;
        exit(1);
    }

    numberOfFieldsInRec++;
}

void CNF::GrowFromParseTree(AndList *parseTree, Schema &schema, Record &literal) {
    CNF &cnf = *this;

    // Hack: literal record as well as schema for it is built up inside a temporary text file
    FILE *outputRecFile = fopen("outputRecFile", "w");

    FILE *outputSchemaFile = fopen ("outputSchemaFile", "w");
    fprintf(outputSchemaFile, "BEGIN\ntempSchema\nwherever\n");

    // Indicates the size of the literal record.
    int numberOfFieldsInLiteral = 0;

    int whichAnd = 0;
    while (parseTree) {
        OrList *tempOr = parseTree->left;

        int whichOr = 0;
        while (tempOr) {
            Type typeLeft, typeRight;

            // Left Operand
            if (tempOr->left->left->code == NAME) {
                int index = schema.FindIndex(tempOr->left->left->value);
                if (index != -1) {
                    cnf.orList[whichAnd][whichOr].operand1 = Left;
                    cnf.orList[whichAnd][whichOr].whichAttr1 = index;
                    cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                    typeLeft = schema.FindType(tempOr->left->left->value);
                }
                else {
                    cout << "ERROR: Could not find attribute " << tempOr->left->left->value << endl;
                    exit (1);
                }
            }
            else if (tempOr->left->left->code == STRING) {
                cnf.orList[whichAnd][whichOr].operand1 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr1 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                        tempOr->left->left->value, String);
                typeLeft = String;
            }
            else if (tempOr->left->left->code == DOUBLE) {
                cnf.orList[whichAnd][whichOr].operand1 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr1 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->left->value, Double);
                typeLeft = Double;
            }
            else if (tempOr->left->left->code == INT) {
                cnf.orList[whichAnd][whichOr].operand1 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr1 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->left->value, Int);
                typeLeft = Int;
            }
            else {
                cout << "Invalid type" << endl;
                exit(1);
            }

            // Right Operand
            if (tempOr->left->right->code == NAME) {
                int index = schema.FindIndex(tempOr->left->right->value);
                if (index != -1) {
                    cnf.orList[whichAnd][whichOr].operand2 = Left;
                    cnf.orList[whichAnd][whichOr].whichAttr2 = index;
                    cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                    typeRight = schema.FindType(tempOr->left->right->value);
                }
                else {
                    cout << "ERROR: Could not find attribute " << tempOr->left->right->value << endl;
                    exit (1);
                }
            }
            else if (tempOr->left->right->code == STRING) {
                cnf.orList[whichAnd][whichOr].operand2 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr2 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->right->value, String);
                typeRight = String;
            }
            else if (tempOr->left->right->code == DOUBLE) {
                cnf.orList[whichAnd][whichOr].operand2 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr2 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->right->value, Double);
                typeRight = Double;
            }
            else if (tempOr->left->right->code == INT) {
                cnf.orList[whichAnd][whichOr].operand2 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr2 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->right->value, Int);
                typeRight = Int;
            }
            else {
                cout << "Invalid type" << endl;
                exit(1);
            }

            if (typeLeft != typeRight) {
                cerr << "ERROR! Type mismatch in CNF. " << tempOr->left->left->value << " and "
                     << tempOr->left->right->value << " were found to not match.\n";
                exit(1);
            }

            // Save type info, and then finally set up the comparison operator
            cnf.orList[whichAnd][whichOr].attrType = typeLeft;
            if (tempOr->left->code == LESS_THAN) {
                cnf.orList[whichAnd][whichOr].compOp = LessThan;
            } else if (tempOr->left->code == GREATER_THAN) {
                cnf.orList[whichAnd][whichOr].compOp = GreaterThan;
            } else if (tempOr->left->code == EQUALS) {
                cnf.orList[whichAnd][whichOr].compOp = Equals;
            } else {
                cerr << "BAD: found a comparison op I don't recognize.\n";
                exit(1);
            }

            whichOr++;
            tempOr = tempOr->rightOr;
        }

        cnf.orLens[whichAnd] = whichOr;

        whichAnd++;
        parseTree = parseTree->rightAnd;
    }

    cnf.numberOfAnds = whichAnd;

    // The very last thing to do is to setup the literal record;
    // First, close the file where it's info is stored.
    fclose(outputRecFile);

    fprintf(outputSchemaFile, "END\n");
    fclose(outputSchemaFile);

    // Open the record file in read mode
    outputRecFile = fopen("outputRecFile", "r");

    // Read in the record's schema
    Schema outSchema("outputSchemaFile", "tempSchema");

    // Get the record
    literal.SuckNextRecord(outSchema, outputRecFile);

    fclose(outputRecFile);

    remove("outputRecFile");
    remove("outputSchemaFile");
}

void CNF::GrowFromParseTree(AndList *parseTree, Schema &left, Schema &right, Record &literal) {
    CNF &cnf = *this;

    // Hack: literal record as well as schema for it is built up inside a temporary text file
    FILE *outputRecFile = fopen("outputRecFile", "w");

    FILE *outputSchemaFile = fopen ("outputSchemaFile", "w");
    fprintf(outputSchemaFile, "BEGIN\ntempSchema\nwherever\n");

    // Indicates the size of the literal record.
    int numberOfFieldsInLiteral = 0;

    int whichAnd = 0;
    while (parseTree) {
        OrList *tempOr = parseTree->left;

        int whichOr = 0;
        while (tempOr) {
            Type typeLeft, typeRight;

            // Left Operand
            if (tempOr->left->left->code == NAME) {
                if (left.FindIndex(tempOr->left->left->value) != -1) {
                    cnf.orList[whichAnd][whichOr].operand1 = Left;
                    cnf.orList[whichAnd][whichOr].whichAttr1 =
                            left.FindIndex(tempOr->left->left->value);
                    cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                    typeLeft = left.FindType(tempOr->left->left->value);
                }
                else if (right.FindIndex(tempOr->left->left->value) != -1) {
                    cnf.orList[whichAnd][whichOr].operand1 = Right;
                    cnf.orList[whichAnd][whichOr].whichAttr1 =
                            right.FindIndex(tempOr->left->left->value);
                    cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                    typeLeft = right.FindType(tempOr->left->left->value);
                }
                else {
                    cout << "ERROR: Could not find attribute " << tempOr->left->left->value << endl;
                    exit (1);
                }
            }
            else if (tempOr->left->left->code == STRING) {
                cnf.orList[whichAnd][whichOr].operand1 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr1 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->left->value, String);
                typeLeft = String;
            }
            else if (tempOr->left->left->code == DOUBLE) {
                cnf.orList[whichAnd][whichOr].operand1 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr1 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->left->value, Double);
                typeLeft = Double;
            }
            else if (tempOr->left->left->code == INT) {
                cnf.orList[whichAnd][whichOr].operand1 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr1 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal1 = tempOr->left->left->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->left->value, Int);
                typeLeft = Int;
            }
            else {
                cout << "Invalid type" << endl;
                exit(1);
            }

            // Right Operand
            if (tempOr->left->right->code == NAME) {
                if (left.FindIndex(tempOr->left->right->value) != -1) {
                    cnf.orList[whichAnd][whichOr].operand2 = Left;
                    cnf.orList[whichAnd][whichOr].whichAttr2 =
                            left.FindIndex(tempOr->left->right->value);
                    cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                    typeRight = left.FindType(tempOr->left->right->value);
                }
                else if (right.FindIndex(tempOr->left->right->value) != -1) {
                    cnf.orList[whichAnd][whichOr].operand2 = Right;
                    cnf.orList[whichAnd][whichOr].whichAttr2 =
                            right.FindIndex(tempOr->left->right->value);
                    cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                    typeRight = right.FindType(tempOr->left->right->value);
                }
                else {
                    cout << "ERROR: Could not find attribute " << tempOr->left->right->value << endl;
                    exit (1);
                }
            }
            else if (tempOr->left->right->code == STRING) {
                cnf.orList[whichAnd][whichOr].operand2 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr2 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->right->value, String);
                typeRight = String;
            }
            else if (tempOr->left->right->code == DOUBLE) {
                cnf.orList[whichAnd][whichOr].operand2 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr2 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->right->value, Double);
                typeRight = Double;
            }
            else if (tempOr->left->right->code == INT) {
                cnf.orList[whichAnd][whichOr].operand2 = Literal;
                cnf.orList[whichAnd][whichOr].whichAttr2 = numberOfFieldsInLiteral;
                cnf.orList[whichAnd][whichOr].whichAttrVal2 = tempOr->left->right->value;
                AddLiteralToFile(numberOfFieldsInLiteral, outputRecFile, outputSchemaFile,
                                 tempOr->left->right->value, Int);
                typeRight = Int;
            }
            else {
                cout << "Invalid type" << endl;
                exit(1);
            }

            if (typeLeft != typeRight) {
                cerr << "ERROR! Type mismatch in CNF. " << tempOr->left->left->value << " and "
                     << tempOr->left->right->value << " were found to not match.\n";
                exit(1);
            }

            // Save type info, and then finally set up the comparison operator
            cnf.orList[whichAnd][whichOr].attrType = typeLeft;
            if (tempOr->left->code == LESS_THAN) {
                cnf.orList[whichAnd][whichOr].compOp = LessThan;
            } else if (tempOr->left->code == GREATER_THAN) {
                cnf.orList[whichAnd][whichOr].compOp = GreaterThan;
            } else if (tempOr->left->code == EQUALS) {
                cnf.orList[whichAnd][whichOr].compOp = Equals;
            } else {
                cerr << "BAD: found a comparison op I don't recognize.\n";
                exit(1);
            }

            whichOr++;
            tempOr = tempOr->rightOr;
        }

        cnf.orLens[whichAnd] = whichOr;

        whichAnd++;
        parseTree = parseTree->rightAnd;
    }

    cnf.numberOfAnds = whichAnd;

    // The very last thing to do is to setup the literal record;
    // First, close the file where it's info is stored.
    fclose(outputRecFile);

    fprintf(outputSchemaFile, "END\n");
    fclose(outputSchemaFile);

    // Open the record file in read mode
    outputRecFile = fopen("outputRecFile", "r");

    // Read in the record's schema
    Schema outSchema("outputSchemaFile", "tempSchema");

    // Get the record
    literal.SuckNextRecord(outSchema, outputRecFile);

    fclose(outputRecFile);

    remove("outputRecFile");
    remove("outputSchemaFile");
}

void CNF::Print() {
    for (int i = 0; i < numberOfAnds; i++) {
        cout << "(";
        for (int j = 0; j < orLens[i]; j++) {
            orList[i][j].PrintStr();
            if (j < orLens[i] - 1) cout << " OR ";
        }
        cout << ")";

        if (i < numberOfAnds - 1) cout << " AND ";
        else cout << endl;
    }
}