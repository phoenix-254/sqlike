#include "ComparisonEngine.h"
#include "RelationalOp.h"

void RelationalOp::WaitUntilDone() const {
    pthread_join(thread, nullptr);
}

void RelationalOp::Use_n_Pages(int n) {
    this->runLength = n;
}

void *SelectFileExecute(void *args) {
    auto *myArgs = (SelectFileArgs*) args;

    Record temp;
    while (myArgs->inputFile->GetNext(temp, *myArgs->selectionOp, *myArgs->literal)) {
        myArgs->outputPipe->Insert(&temp);
    }

    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void SelectFile::Run(DBFile &inputFile, Pipe &outputPipe, CNF &selectionOp, Record &literal) {
    auto *args = new SelectFileArgs();
    args->inputFile = &inputFile;
    args->outputPipe = &outputPipe;
    args->selectionOp = &selectionOp;
    args->literal = &literal;

    pthread_create(&thread, nullptr, SelectFileExecute, (void *) args);
}

void SelectPipe::Run(Pipe &inputPipe, Pipe &outputPipe, CNF &selectionOp, Record &literal) {

}

void *ProjectExecute(void *args) {
    auto *myArgs = (ProjectArgs*) args;

    Record temp;
    while (myArgs->inputPipe->Remove(&temp)) {
        temp.Project(myArgs->attrsToKeep, myArgs->outputAttrCount, myArgs->inputAttrCount);
        myArgs->outputPipe->Insert(&temp);
    }

    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void Project::Run(Pipe &inputPipe, Pipe &outputPipe, int *attrsToKeep, int inputAttrCount, int outputAttrCount) {
    auto *args = new ProjectArgs();
    args->inputPipe = &inputPipe;
    args->outputPipe = &outputPipe;
    args->attrsToKeep = attrsToKeep;
    args->inputAttrCount = inputAttrCount;
    args->outputAttrCount = outputAttrCount;

    pthread_create(&thread, nullptr, ProjectExecute, (void *) args);
}

void Join::Run(Pipe &inputPipeLeft, Pipe &inputPipeRight, Pipe &outputPipe, CNF &selectionOp, Record &literal) {

}

void DuplicateRemoval::Run(Pipe &inputPipe, Pipe &outputPipe, Schema &schema) {

}

void *SumExecute(void *args) {
    auto *myArgs = (SumArgs*) args;

    double sum = 0;

    int intVal = 0;
    double doubleVal = 0;

    Record temp;
    while (myArgs->inputPipe->Remove(&temp)) {
        intVal = 0; doubleVal = 0;

        myArgs->function->Apply(temp, intVal, doubleVal);
        sum += (intVal + doubleVal);
    }

    Attribute attr = {"sum", Double};
    Schema sumSchema("sumSchema", 1, &attr);
    temp.ComposeRecord(sumSchema, (std::to_string(sum) + "|").c_str());

    myArgs->outputPipe->Insert(&temp);
    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void Sum::Run(Pipe &inputPipe, Pipe &outputPipe, Function &computeMe) {
    auto *args = new SumArgs();
    args->inputPipe = &inputPipe;
    args->outputPipe = &outputPipe;
    args->function = &computeMe;

    pthread_create(&thread, nullptr, SumExecute, (void *) args);
}

void GroupBy::Run(Pipe &inputPipe, Pipe &outputPipe, OrderMaker &groupAttrs, Function &computeMe) {

}

void WriteOut::Run(Pipe &inputPipe, FILE *outputFile, Schema &schema) {

}
