#ifndef SQLIKE_HEAP_H
#define SQLIKE_HEAP_H

#include "Comparison.h"
#include "ComparisonEngine.h"
#include "File.h"
#include "GenericDBFile.h"

class Heap : public GenericDBFile {
private:
    File *file;
    string fileLocation;

    Page *readPage, *writePage;

    // Pointer to the current read and write pages.
    off_t readPtr, writePtr;

    ComparisonEngine *comparisonEngine;

    void WriteToFile();

public:
    Heap();

    ~Heap() override;

    int Create(const char *filePath, fileType type, void *startUp) override;

    int Open(const char *filePath) override;

    int Close() override;

    void CloseFile();

    void Load(Schema &schema, const char *loadPath) override;

    void MoveFirst() override;

    void Add(Record &addMe) override;

    int GetNext(Record &fetchMe) override;

    int GetNext(Record &fetchMe, CNF &cnf, Record &literal) override;
};

#endif //SQLIKE_HEAP_H
