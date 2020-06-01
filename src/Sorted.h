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
    const char *fileLocation;

    BigQ *bigQ;

    Pipe *inputPipe, *outputPipe;

    Page *readPage, *writePage;

    // Pointer to the current read and write pages.
    off_t readPtr, writePtr;

    ComparisonEngine *comparisonEngine;

    OrderMaker *sortOrder;
    int runLength;

    void Init();

    void Read();
    void Write();

    void Merge();

public:
    Sorted();
    Sorted(OrderMaker *orderMaker, int runLength);

    ~Sorted();

    int Create(const char *filePath, fileType type, void *startUp);

    int Open(const char *filePath);

    int Close();

    void Load(Schema &schema, const char *loadPath);

    void MoveFirst();

    void Add(Record &addMe);

    int GetNext (Record &fetchMe);

    int GetNext (Record &fetchMe, CNF &cnf, Record &literal);
};

#endif //SQLIKE_SORTED_H
