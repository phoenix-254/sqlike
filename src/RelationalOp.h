#ifndef SQLIKE_RELATIONALOP_H
#define SQLIKE_RELATIONALOP_H

#include "DBFile.h"
#include "Function.h"
#include "Pipe.h"
#include "Record.h"

class RelationalOp {
public:
    // Blocks the caller until the particular relational operator has completed
    virtual void WaitUntilDone() = 0;

    // Tell us the amount of internal memory this operation can utilise
    virtual void Use_n_Pages(int n) = 0;
};

class SelectFile : public RelationalOp {
public:
    void Run(DBFile &inputFile, Pipe &outputPipe, CNF &selectionOp, Record &literal);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

class SelectPipe : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, CNF &selectionOp, Record &literal);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

class Project : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, int *attrsToKeep, int inputAttrCount, int outputAttrCount);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

class Join : public RelationalOp {
public:
    void Run(Pipe &inputPipeLeft, Pipe &inputPipeRight, Pipe &outputPipe, CNF &selectionOp, Record &literal);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

class DuplicateRemoval : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, Schema &schema);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

class Sum : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, Function &computeMe);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

class GroupBy : public RelationalOp {
public:
    void Run(Pipe &inputPipe, Pipe &outputPipe, OrderMaker &groupAttrs, Function &computeMe);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

class WriteOut : public RelationalOp {
public:
    void Run(Pipe &inputPipe, FILE *outputFile, Schema &schema);

    void WaitUntilDone() override;

    void Use_n_Pages(int n) override;
};

#endif //SQLIKE_RELATIONALOP_H
