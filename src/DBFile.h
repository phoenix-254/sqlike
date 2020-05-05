#ifndef SQLIKE_DBFILE_H
#define SQLIKE_DBFILE_H

#include "Comparison.h"
#include "Record.h"
#include "Schema.h"

typedef enum {
    HEAP,
    SORTED,
    TREE
} fileType;

class DBFile {
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
