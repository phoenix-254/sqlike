#include "RelationalOp.h"

void SelectFile::Run(DBFile &inputFile, Pipe &outputPipe, CNF &selectionOp, Record &literal) {

}

void SelectFile::WaitUntilDone() {

}

void SelectFile::Use_n_Pages(int n) {

}

void SelectPipe::Run(Pipe &inputPipe, Pipe &outputPipe, CNF &selectionOp, Record &literal) {

}

void SelectPipe::WaitUntilDone() {

}

void SelectPipe::Use_n_Pages(int n) {

}

void Project::Run(Pipe &inputPipe, Pipe &outputPipe, int *attrsToKeep, int inputAttrCount, int outputAttrCount) {

}

void Project::WaitUntilDone() {

}

void Project::Use_n_Pages(int n) {

}

void Join::Run(Pipe &inputPipeLeft, Pipe &inputPipeRight, Pipe &outputPipe, CNF &selectionOp, Record &literal) {

}

void Join::WaitUntilDone() {

}

void Join::Use_n_Pages(int n) {

}

void DuplicateRemoval::Run(Pipe &inputPipe, Pipe &outputPipe, Schema &schema) {

}

void DuplicateRemoval::WaitUntilDone() {

}

void DuplicateRemoval::Use_n_Pages(int n) {

}

void Sum::Run(Pipe &inputPipe, Pipe &outputPipe, Function &computeMe) {

}

void Sum::WaitUntilDone() {

}

void Sum::Use_n_Pages(int n) {

}

void GroupBy::Run(Pipe &inputPipe, Pipe &outputPipe, OrderMaker &groupAttrs, Function &computeMe) {

}

void GroupBy::WaitUntilDone() {

}

void GroupBy::Use_n_Pages(int n) {

}

void WriteOut::Run(Pipe &inputPipe, FILE *outputFile, Schema &schema) {

}

void WriteOut::WaitUntilDone() {

}

void WriteOut::Use_n_Pages(int n) {

}
