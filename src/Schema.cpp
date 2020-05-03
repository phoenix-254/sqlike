#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include "Schema.h"

Type Schema::GetType(string attrStr) {
    if (attrStr.compare("Int") == 0) return Int;
    else if (attrStr.compare("Double") == 0) return Double;
    else if (attrStr.compare("String") == 0) return String;
    else {
        cout << "Invalid type : " << attrStr  << endl;
        exit(1);
    }
}

string Schema::GetTypeStr(Type type) {
    if (type == Int) return "Int";
    else if (type == Double) return "Double";
    else if (type == String) return "String";
    else return "Invalid";
}

Schema::Schema(string fileName, string relationName) {
    cout << fileName << ", " << relationName << endl;

    ifstream file;
    file.open(fileName);

    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (line.compare("BEGIN") == 0) {
                // Move to next line and read relation name.
                getline(file, line);
                this->fileName = line;

                // Check if line equal to the relation name.
                if (line.compare(relationName) == 0) {
                    // Skip a line containing relation file name
                    getline(file, line);

                    while (getline(file, line) && line.compare("END") != 0) {
                        // Make attribute from a string e.g "r_regionkey Int"
                        istringstream buf(line);
                        istream_iterator<string> beg(buf), end;
                        vector<string> tokens(beg, end);

                        Attribute attr;
                        attr.name = tokens[0];
                        attr.type = GetType(tokens[1]);

                        attrs.push_back(attr);
                    }

                    break;
                }
                else {
                    continue;
                }
            }
        }

        file.close();
    }
    else {
        cout << "ERROR : Unable to open file " << fileName << endl;
        exit(1);
    }
}

Schema::~Schema() {
    attrs.clear();
}

int Schema::FindIndex(string attrName) {
    for (int index = 0; index < attrs.size(); index++) {
        if (attrName.compare(attrs[index].name)) {
            return index;
        }
    }

    // Not found.
    return -1;
}

Type Schema::FindType(string attrName) {
    for (int index = 0; index < attrs.size(); index++) {
        if (attrName.compare(attrs[index].name)) {
            return attrs[index].type;
        }
    }

    // Not found, return Int by default.
    return Int;
}

int Schema::GetAttributeCount() {
    return attrs.size();
}

vector<Attribute> Schema::GetAttributes() {
    return attrs;
}

string Schema::GetFileName() {
    return fileName;
}

void Schema::Print() {
    for (int index = 0; index < attrs.size(); index++) {
        cout << attrs[index].name << " " << GetTypeStr(attrs[index].type) << endl;
    }
}