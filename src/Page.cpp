#include <cstring>
#include <iostream>

#include "Page.h"

using namespace std;

Page::Page() {
    numberOfRecs = 0;
    currSizeInBytes = sizeof(int);

    records = new (std::nothrow) TwoWayList<Record>;
    if (records == nullptr) {
        cout << "ERROR : Not enough memory. EXIT !!!\n";
        exit(1);
    }
}

Page::~Page() {
    delete records;
}

void Page::ToBinary(char *bits) {
    // Write the number of records on the page
    ((int *) bits)[0] = numberOfRecs;

    records->MoveToStart();

    char *currPosition = bits + sizeof(int);

    // Copy the records one-by-one
    for (int index = 0; index < numberOfRecs; index++) {
        char *src = records->Current(0)->GetBits();
        memcpy(currPosition, src, ((int *) src)[0]);
        currPosition += ((int *) src)[0];

        // Traverse the list
        records->Advance();
    }
}

void Page::FromBinary(char *bits) {
    // Read the number of records on the page, and perform sanity check
    numberOfRecs = ((int *) bits)[0];
    if (numberOfRecs > 1000000 || numberOfRecs < 0) {
        cerr << "This is probably an error.  Found " << numberOfRecs << " records on a page.\n";
        exit(1);
    }

    // Now, get the binary representations of each
    char *currPosition = bits + sizeof(int);

    // First, empty out the list of current records
    records->MoveToStart();
    while (records->RightLength()) {
        Record temp;
        records->Remove(&temp);
    }

    // Now, loop through and re-populate it
    auto *temp = new (std::nothrow) Record();
    if (temp == nullptr){
        cout << "ERROR : Not enough memory. EXIT !!!\n";
        exit(1);
    }

    currSizeInBytes = sizeof(int);
    for (int index = 0; index < numberOfRecs; index++) {
        int len = ((int *) currPosition)[0];
        currSizeInBytes += len;

        // Create record copy, add it and move along
        temp->CopyBits(currPosition, len);
        records->Insert(temp);
        records->Advance();
        currPosition += len;
    }

    delete temp;
}

int Page::GetFirst(Record *record) {
    records->MoveToStart();
    if (!records->RightLength()) return 0;

    // Remove the record.
    records->Remove(record);
    numberOfRecs--;

    char *bits = record->GetBits();
    currSizeInBytes -= ((int *) bits)[0];

    return 1;
}

int Page::Append(Record *record) {
    char *bits = record->GetBits();

    // First, see if we can fit the record
    if (currSizeInBytes + ((int *) bits)[0] > PAGE_SIZE) return 0;

    // Move to the last record, and add it.
    records->MoveToFinish();
    currSizeInBytes += ((int *) bits)[0];
    records->Insert(record);
    numberOfRecs++;

    return 1;
}

void Page::EmptyItOut() {
    while (true) {
        Record temp;
        if (!GetFirst(&temp)) break;
    }

    numberOfRecs = 0;
    currSizeInBytes = sizeof(int);
}

int Page::GetNumberOfRecs() const {
    return numberOfRecs;
}
