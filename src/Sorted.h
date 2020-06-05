#ifndef SQLIKE_SORTED_H
#define SQLIKE_SORTED_H

#include "BigQ.h"
#include "GenericDBFile.h"
#include "File.h"

class Sorted : public GenericDBFile {
private:
    enum Mode {
        READ,
        WRITE
    } mode;

    File *file;
    string fileLocation, tempMergeFileLocation;

    BigQ *bigQ;

    Pipe *inputPipe, *outputPipe;

    Page *bufferedPage;
    off_t currentPagePtr;

    ComparisonEngine *comparisonEngine;

    OrderMaker *sortOrder;
    int runLength;

    // To be used in GetNext with CNF.
    OrderMaker *queryOrder;
    OrderMaker *cnfOrder;

    void Init();

    void Read();
    void Write();

    void Merge();

    void BuildQueryOrder(CNF &cnf);

    off_t BinarySearch(off_t low, off_t high, Record &literal);

    int GetNextBinarySearch(Record &fetchMe, CNF &cnf, Record &literal, bool isCnfUpdated);

    int GetNextLinearSearch(Record &fetchMe, CNF &cnf, Record &literal);

public:
    Sorted();
    Sorted(OrderMaker *orderMaker, int runLength);

    ~Sorted() override;

    int Create(const char *filePath, fileType type, void *startUp) override;

    int Open(const char *filePath) override;

    int Close() override;

    void Load(Schema &schema, const char *loadPath) override;

    void MoveFirst() override;

    void Add(Record &addMe) override;

    int GetNext(Record &fetchMe) override;

    int GetNext(Record &fetchMe, CNF &cnf, Record &literal) override;
};

#endif //SQLIKE_SORTED_H
