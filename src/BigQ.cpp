#include <algorithm>
#include <queue>

#include "BigQ.h"

void *Worker(void *bigQ) {
    auto *myBigQ = (BigQ*) bigQ;
    myBigQ->ExecuteSortPhase();
    myBigQ->ExecuteMergePhase();
    return nullptr;
}

BigQ::BigQ(Pipe &input, Pipe &output, OrderMaker &sortOrder, int runLength) {
    args->input = input;
    args->output = output;
    args->sortOrder = sortOrder;
    args->runLength = runLength;

    tempFile = new File();
    tempFile->Open(0, tempFileName);

    pthread_t worker;
    pthread_create(&worker, nullptr, Worker, (void *) this);
}

// Reads data from the input pipe and sort them into run-length pages.
void BigQ::ExecuteSortPhase() {
    int currentSize = 0, capacity = args->runLength * PAGE_SIZE;

    Page tempPage;
    off_t pageIndex = 1;

    Record tempRec;
    Record *copyRec;

    vector<Record*> records;

    while (args->input.Remove(&tempRec)) {
        copyRec = new Record();
        copyRec->Consume(&tempRec);

        currentSize += copyRec->GetSize();

        // If more records can be added.
        if (currentSize <= capacity) {
            records.push_back(copyRec);
        }
        // If not.
        else {
            // Sort the current record list.
            sort(records.begin(), records.end(), [this](Record *left, Record *right) {
                return comparisonEngine->Compare(left, right, &(args->sortOrder)) < 0;
            });

            // Write records to file using pages.
            for (auto & rec : records) {
                if (!tempPage.Append(rec)) {
                    tempFile->AddPage(&tempPage, pageIndex++);
                    tempPage.EmptyItOut();
                    tempPage.Append(rec);
                }
            }

            // Write off the last page records.
            if (tempPage.GetNumberOfRecs() > 0) {
                tempFile->AddPage(&tempPage, pageIndex++);
                tempPage.EmptyItOut();
            }

            runIndexes.push_back(pageIndex);

            // Clear records list and update current-size
            for (auto & rec : records) delete rec;
            records.clear();
            records.push_back(copyRec);
            currentSize = sizeof(int) + copyRec->GetSize();
        }
    }

    // Write off the last bunch of records which never exceeded capacity.
    // Sort the current record list.
    sort(records.begin(), records.end(), [this](Record *left, Record *right) {
        return comparisonEngine->Compare(left, right, &(args->sortOrder)) < 0;
    });

    // Write records to file using pages.
    for (auto & rec : records) {
        if (!tempPage.Append(rec)) {
            tempFile->AddPage(&tempPage, pageIndex++);
            tempPage.EmptyItOut();
            tempPage.Append(rec);
        }
    }

    // Write off the last page records.
    if (tempPage.GetNumberOfRecs() > 0) {
        tempFile->AddPage(&tempPage, pageIndex++);
        tempPage.EmptyItOut();
    }

    runIndexes.push_back(pageIndex);

    for (auto & rec : records) delete rec;
    records.clear();
}

// Construct priority queue over sorted runs and dump sorted data into the out pipe.
void BigQ::ExecuteMergePhase() {
}

BigQ::~BigQ() {
    delete args;

    tempFile->Close();
    delete tempFile;
    remove(tempFileName);

    delete comparisonEngine;
}