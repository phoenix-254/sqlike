#ifndef SQLIKE_TEST_H
#define SQLIKE_TEST_H

#include <map>

#include "Comparison.h"
#include "Config.h"
#include "Schema.h"

using namespace std;

extern int yyparse();

extern struct AndList *final;

class Relation {
private:
    const char *relationName;
    Schema *relationSchema;

public:
    explicit Relation(const char *relName) : relationName(relName) {
        relationSchema = new Schema(CATALOG_FILE_PATH, relName);
    }

    const char* GetRelationName() {
        return relationName;
    }

    const char* GetBinFilePath() {
        char binFilePath[100];
        sprintf(binFilePath, "%s%s.bin", BIN_FILES_PATH, relationName);
        return binFilePath;
    }

    const char* GetTblFilePath() {
        char tblFilePath[100];
        sprintf(tblFilePath, "%s%s.tbl", TBL_FILES_PATH, relationName);
        return tblFilePath;
    }

    Schema* GetSchema() {
        return relationSchema;
    }

    void GetCnf(CNF &cnf, Record &literal) {
        cout << endl << ">>>> Enter in your CNF and press (ctrl+d) for EOF: " << endl;
        if (yyparse() != 0) {
            cout << "ERROR : Invalid expression" << endl;
            exit(1);
        }
        cnf.GrowFromParseTree(final, *(GetSchema()), literal);
    }
};


map<int, Relation*> relations;

void Setup() {
    for (auto & tbl : TABLE_MAP) {
        relations[tbl.first] = new Relation(tbl.second);
    }
}

void Cleanup() {
    relations.clear();
}

#endif //SQLIKE_TEST_H
