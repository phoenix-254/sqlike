#ifndef SQLIKE_BIGQ_H
#define SQLIKE_BIGQ_H

#include "Comparison.h"
#include "ComparisonEngine.h"
#include "File.h"
#include "Pipe.h"

class BigQ {
private:
    Pipe *input, *output;
    OrderMaker *sortOrder;
    int runLength;

    File *tempFile;
    const char *tempFileName = "temp.bin";

    ComparisonEngine *comparisonEngine;

    vector<off_t> runIndexes;

public:
    BigQ(Pipe &input, Pipe &output, OrderMaker &sortOrder, int runLength);
    ~BigQ();

    void ExecuteSortPhase();

    void ExecuteMergePhase();
};

#endif //SQLIKE_BIGQ_H
