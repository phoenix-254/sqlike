#ifndef SQLIKE_HEAP_H
#define SQLIKE_HEAP_H

#include "Comparison.h"
#include "ComparisonEngine.h"
#include "File.h"
#include "GenericDBFile.h"

class Heap : public GenericDBFile {
private:
    File *file;
    const char *filePath;

    Page *readPage, *writePage;

    // Pointer to the current read and write pages.
    off_t readPtr, writePtr;

    ComparisonEngine *comparisonEngine;

    void WriteToFile();

public:
    Heap();

    ~Heap();

    int Create(const char *filePath, fileType type, void *startUp);

    int Open(const char *filePath);

    int Close();

    void Load(Schema &schema, const char *loadPath);

    void MoveFirst();

    void Add(Record &addMe);

    int GetNext (Record &fetchMe);

    int GetNext (Record &fetchMe, CNF &cnf, Record &literal);
};

#endif //SQLIKE_HEAP_H
