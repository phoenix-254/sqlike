#ifndef SQLIKE_RELATIONALOP_H
#define SQLIKE_RELATIONALOP_H

#include "DBFile.h"
#include "Function.h"
#include "Pipe.h"
#include "Record.h"

class RelationalOp {
protected:
    int runLength;
    pthread_t thread;

public:
    // Blocks the caller until the particular relational operator has completed
    void WaitUntilDone() const;

    // Tell us the amount of internal memory this operation can utilise
    void Use_n_Pages(int n);
};

struct SelectFileArgs {
    DBFile *inputFile;
    Pipe *outputPipe;
    CNF *selectionOp;
    Record *literal;
};

void *SelectFileExecute(void *args);

class SelectFile : public RelationalOp {
public:
    void Run(DBFile &inputFile, Pipe &outputPipe, CNF &selectionOp, Record &literal);
};

class SelectPipe : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, CNF &selectionOp, Record &literal);
};

class Project : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, int *attrsToKeep, int inputAttrCount, int outputAttrCount);
};

class Join : public RelationalOp {
public:
    void Run(Pipe &inputPipeLeft, Pipe &inputPipeRight, Pipe &outputPipe, CNF &selectionOp, Record &literal);
};

class DuplicateRemoval : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, Schema &schema);
};

class Sum : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, Function &computeMe);
};

class GroupBy : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, OrderMaker &groupAttrs, Function &computeMe);
};

class WriteOut : public RelationalOp {
public:
    void Run(Pipe &inputPipe, FILE *outputFile, Schema &schema);
};

#endif //SQLIKE_RELATIONALOP_H
