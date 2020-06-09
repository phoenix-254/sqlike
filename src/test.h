#ifndef SQLIKE_TEST_H
#define SQLIKE_TEST_H

#include <iostream>
#include <map>

#include "Config.h"
#include "Function.h"
#include "Pipe.h"
#include "Record.h"

using namespace std;

// defined in parser
extern int yyparse();
extern void init_lexical_parser(char *);
extern void close_lexical_parser();

// defined in parser_func
extern int yyfuncparse();
extern void init_lexical_parser_func(char *);
extern void close_lexical_parser_func();

extern struct AndList *final;
extern struct FuncOperator *finalfunc;
extern FILE *yyin;

typedef struct {
    Pipe *pipe;

    OrderMaker *order;

    bool print, write;
} TestUtil;

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

    string GetBinFilePath() {
        string binFilePath(BIN_FILES_PATH);
        binFilePath.append(relationName);
        binFilePath.append(".bin");
        return binFilePath;
    }

    string GetTblFilePath() {
        string tblFilePath(TBL_FILES_PATH);
        tblFilePath.append(relationName);
        tblFilePath.append(".tbl");
        return tblFilePath;
    }

    Schema* GetSchema() {
        return relationSchema;
    }

    void GetCnf(char *input, CNF &cnf, Record &literal) {
        init_lexical_parser(input);
        if (yyparse() != 0) {
            cout << "ERROR : Invalid expression" << endl;
            exit(1);
        }
        cnf.GrowFromParseTree(final, *(GetSchema()), literal);
        close_lexical_parser();
    }

    void GetCnf(char *input, Function &func) {
        init_lexical_parser_func(input);
        if (yyfuncparse() != 0) {
            cout << "ERROR : Invalid expression" << endl;
            exit(1);
        }
        func.GrowFromParseTree(finalfunc, *(GetSchema()));
        close_lexical_parser_func();
    }

    void GetCnf(CNF &cnf, Record &literal) {
        cout << endl << ">>>> Enter in your CNF and press (ctrl+d) for EOF: " << endl;

        if (yyparse() != 0) {
            cout << "ERROR : Invalid expression" << endl;
            exit(1);
        }
        cnf.GrowFromParseTree(final, *(GetSchema()), literal);
    }

    void GetFileCnf(const char *filePath, CNF &cnf, Record &literal) {
        yyin = fopen(filePath, "r");
        if (yyin == nullptr) {
            cout << "Unable to open file " << filePath << " for parsing." << endl;
            exit(1);
        }
        if (yyparse() != 0) {
            cout << "ERROR : Invalid expression" << endl;
            exit(1);
        }
        cnf.GrowFromParseTree(final, *(GetSchema()), literal);
    }

    void GetSortOrder(CNF &cnf, OrderMaker &orderMaker) {
        cout << ">>>> Specify sort ordering and press (ctrl+d) for EOF : " << endl;

        if (yyparse() != 0) {
            cout << "ERROR : Invalid expression" << endl;
            exit(1);
        }
        cout << endl;

        Record literal;
        cnf.GrowFromParseTree(final, *(GetSchema()), literal); // constructs CNF predicate

        OrderMaker dummy;
        cnf.GetSortOrder(orderMaker, dummy);
    }
};

void GetCnf(char *input, Schema &schema, CNF &cnf, Record &literal) {
    init_lexical_parser(input);
    if (yyparse() != 0) {
        cout << "ERROR : Invalid expression" << endl;
        exit(1);
    }
    cnf.GrowFromParseTree(final, schema, literal);
    close_lexical_parser();
}

void GetCnf(char *input, Schema &schema, Function &func) {
    init_lexical_parser_func(input);
    if (yyfuncparse() != 0) {
        cout << "ERROR : Invalid expression" << endl;
        exit(1);
    }
    func.GrowFromParseTree(finalfunc, schema);
    close_lexical_parser_func();
}

void GetCnf(char *input, Schema &left, Schema &right, CNF &cnf, Record &literal) {
    init_lexical_parser(input);
    if (yyparse() != 0) {
        cout << "ERROR : Invalid expression" << endl;
        exit(1);
    }
    cnf.GrowFromParseTree(final, left, right, literal);
    close_lexical_parser();
}

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
