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

    static Type GetType(const string& attrStr);
    static string GetTypeStr(Type type);

public:
    Schema(const string& fileName, const string& relationName);

    Schema(const string& fileName, int numAttrs, Attribute *attrs);

    ~Schema();

    int FindIndex(const string& attrName);

    Type FindType(const string& attrName);

    // Getter Functions - START.
    int GetAttributeCount();

    vector<Attribute> GetAttributes();

    string GetFileName();
    // Getter Functions - END.

    void Print();
};

#endif //SQLIKE_SCHEMA_H
