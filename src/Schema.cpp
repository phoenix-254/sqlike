#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include "Schema.h"

Type Schema::GetType(const string& attrStr) {
    if (attrStr == "Int") return Int;
    else if (attrStr == "Double") return Double;
    else if (attrStr == "String") return String;
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

Schema::Schema(const string& fileName, const string& relationName) {
    ifstream file(fileName);

    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (line == "BEGIN") {
                // Move to next line and read relation name.
                getline(file, line);
                this->fileName = line;

                // Check if line equal to the relation name.
                if (line == relationName) {
                    // Skip a line containing relation file name
                    getline(file, line);

                    while (getline(file, line) && line != "END") {
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

int Schema::FindIndex(const string& attrName) {
    for (int index = 0; index < attrs.size(); index++) {
        if (attrName == (attrs[index].name)) {
            return index;
        }
    }

    // Not found.
    return -1;
}

Type Schema::FindType(const string& attrName) {
    for (auto & attr : attrs) {
        if (attrName == (attr.name)) {
            return attr.type;
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
    for (auto & attr : attrs) {
        cout << attr.name << " " << GetTypeStr(attr.type) << endl;
    }
}