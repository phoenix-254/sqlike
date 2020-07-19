#ifndef SQLIKE_RECORD_H
#define SQLIKE_RECORD_H

#include <fstream>
#include <string>

#include "Schema.h"

class Record {
private:
    char *bits;

    void SetBits(char *bits);

    void Clear();

    static void MemoryValidation(const char *arr);

public:
    Record();

    ~Record();

    char* GetBits();

    void CopyBits(char *src, int len);

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
     * Reads the record from a given text. Similar to SuckNextRecord.
     */
    int ComposeRecord(Schema &schema, const char *src);

    /*
     * Project away various attributes.
     * attrIndexesToKeep - sorted array of all the attributes to keep
     * numAttrsToKeep - number of attributes to keep
     * numAttrsNow - number of attributes currently in the record
     */
    void Project(const int *attrsToKeep, int numAttrsToKeep, int numAttrsNow);

    /*
     * Creates a new record by merging the two records given in the parameters.
     * Useful for join operation.
     */
    void MergeRecords(Record *left, Record *right, int numAttrsLeft, int numAttrsRight,
            const int *attrsToKeep, int numAttrsToKeep, int startOfRight);

    // Prints the contents of the current record.
    void Print(Schema &schema);

    // Returns the size of this record.
    int GetSize();

    // Returns the number of attributes for the record.
    int GetAttrCount();
};

#endif //SQLIKE_RECORD_H
