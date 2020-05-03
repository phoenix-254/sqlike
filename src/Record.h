#ifndef SQLIKE_RECORD_H
#define SQLIKE_RECORD_H

#include <fstream>
#include <string>

#include "Schema.h"

class Record {
private:
    char *bits;

    char* GetBits();

    void SetBits(char *bits);
    void CopyBits(char *src, int len);

    void Clear();

    static void MemoryValidation(const char *arr);

public:
    Record();

    ~Record();

    /*
     * Copies the given record in to current record and deletes consumeMe after this.
     */
    void Consume(Record *consumeMe);

    /*
     * Copies the given record in to current record.
     * Note: This is far more expensive than Consume, as this does bit-by-bit copy.
     */
    void Copy(Record *copyMe);

    /*
     * Reads the next record from a pointer to a file.
     * returns 0 if no data left or there is error
     * returns 1 if data is read successfully
     */
    int SuckNextRecord(Schema &schema, FILE *file);

    /*
     * Project away various attributes.
     * attrIndexesToKeep - sorted array of all the attributes to keep
     * numAttrsToKeep - number of attributes to keep
     * numAttrsNow - number of attributes currently in the record
     */
    void Project(int *attrsToKeep, int numAttrsToKeep, int numAttrsNow);

    /*
     * Creates a new record by merging the two records given in the parameters.
     * Useful for join operation.
     */
    void MergeRecords(Record *left, Record *right, int numAttrsLeft, int numAttrsRight,
            int *attrsToKeep, int numAttrsToKeep, int startOfRight);

    // Prints the contents of the current record.
    void Print(Schema &schema);
};

#endif //SQLIKE_RECORD_H
