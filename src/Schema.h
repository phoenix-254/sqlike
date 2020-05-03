#ifndef SQLIKE_SCHEMA_H
#define SQLIKE_SCHEMA_H

#include <string>
#include <vector>

#include "Const.h"

using namespace std;

struct Attribute {
    string name;
    Type type;
};

class Schema {
private:
    vector<Attribute> attrs;

    string fileName;

    Type GetType(string attrStr);
    string GetTypeStr(Type type);

public:
    Schema(string fileName, string relationName);

    ~Schema();

    int FindIndex(string attrName);

    Type FindType(string attrName);

    // Getter Functions - START.
    int GetAttributeCount();

    vector<Attribute> GetAttributes();

    string GetFileName();
    // Getter Functions - END.

    void Print();
};

#endif //SQLIKE_SCHEMA_H
