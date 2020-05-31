#ifndef SQLIKE_SORTED_H
#define SQLIKE_SORTED_H

#include "GenericDBFile.h"

class Sorted : public GenericDBFile {
public:
    Sorted();

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
