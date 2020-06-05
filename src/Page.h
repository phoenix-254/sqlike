#ifndef SQLIKE_PAGE_H
#define SQLIKE_PAGE_H

#include "Record.h"
#include "TwoWayList.cpp"

class Page {
private:
    TwoWayList<Record> *records;

    int numberOfRecs, currSizeInBytes;

public:
    Page();

    ~Page();

    // Takes a page and writes its binary representation to bits
    void ToBinary(char *bits);

    // Takes a binary representation of a page and gets the records from it
    void FromBinary(char *bits);

    // Returns first record from this page, returns 0 if there are no records.
    // Note: It deletes the record returned.
    int GetFirst(Record *record);

    // Appends the new record to the end of the current page.
    // Returns 1 if successfully appended, 0 otherwise.
    // Note: The record is consumed, hence it won't have any value afterwards.
    int Append(Record *record);

    // Clear page contents.
    void EmptyItOut();

    // Returns the number of records this page is holding.
    int GetNumberOfRecs() const;
};

#endif //SQLIKE_PAGE_H
