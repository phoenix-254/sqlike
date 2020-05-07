#include <iostream>

#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Config.h"

using namespace std;

extern int yyparse();

extern struct AndList *final;

int main() {
    int tableIndex = 0;
    while (tableIndex < 1 || tableIndex > 8) {
        cout << ">>>> Select Table" << endl;
        for (auto & t : TABLE_MAP) {
            cout << "\t" << t.first << ". " << t.second << endl;
        }

        cin >> tableIndex;
        if (tableIndex < 1 || tableIndex > 8)
            cout << "Invalid choice, please try again!" << endl << endl;
    }

    const char *tableName = TABLE_MAP.find(tableIndex)->second;
    cout << endl << ">>>> Printing Schema For " << tableName << endl;
    Schema schema(CATALOG_FILE_PATH, tableName);
    schema.Print();

    cout << endl << ">>>> Enter in your CNF and press (ctrl+d) for EOF: " << endl;
    if (yyparse() != 0) {
        cout << "ERROR : Invalid expression" << endl;
        exit(1);
    }

    CNF cnf{};
    Record literal;
    cnf.GrowFromParseTree(final, schema, literal);
    cout << endl << ">>>> CNF:" << endl;
    cnf.Print();

    char tblFilePath[100];
    sprintf(tblFilePath, "%s%s.tbl", TBL_FILES_PATH, tableName);
    FILE *tblFile = fopen(tblFilePath, "r");

    Record temp;
    ComparisonEngine comparisonEngine;
    cout << endl << ">>>> Printing Records" << endl;
    while (temp.SuckNextRecord(schema, tblFile)) {
        if (comparisonEngine.Compare(&temp, &literal, &cnf)) {
            temp.Print(schema);
        }
    }

    return 0;
}