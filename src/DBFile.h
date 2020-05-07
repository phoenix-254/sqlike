#ifndef SQLIKE_DBFILE_H
#define SQLIKE_DBFILE_H

#include "Comparison.h"
#include "File.h"
#include "Record.h"
#include "Schema.h"

typedef enum {
    HEAP,
    SORTED,
    TREE
} fileType;

class DBFile {
private:
    File *file;

    Page *readPage, *writePage;

    // Pointer to the current read and write pages.
    off_t readPtr, writePtr;

    void WriteToFile();

public:
    DBFile();

    ~DBFile();

    int Create(const char *filePath, fileType type, void *startUp);

    int Open(const char *filePath);

    int Close();

    void Load(Schema &schema, const char *loadPath);

    void MoveFirst();

    void Add(Record &addMe);

    int GetNext (Record &fetchMe);

    int GetNext (Record &fetchMe, CNF &cnf, Record &literal);
};

#endif //SQLIKE_DBFILE_H
