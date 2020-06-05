#include <algorithm>
#include <queue>

#include "BigQ.h"

Run::Run(File *tempFile, off_t start, off_t end) {
    tempFilePtr = tempFile;

    currPageIndex = start;
    endPageIndex = end;

    bufferPage = new Page();
    tempFilePtr->GetPage(bufferPage, currPageIndex++);

    currentRec = new Record();
    bufferPage->GetFirst(currentRec);
}

int Run::Next(Record *current) {
    current->Consume(currentRec);
    if (!bufferPage->GetFirst(currentRec)) {
        if (currPageIndex < endPageIndex) {
            bufferPage->EmptyItOut();
            tempFilePtr->GetPage(bufferPage, currPageIndex++);
            bufferPage->GetFirst(currentRec);
        }
        else {
            return 0;
        }
    }

    return 1;
}

Run::~Run() {
    delete tempFilePtr;
    delete bufferPage;
    delete currentRec;
}

// Worker thread.
void *Worker(void *bigQ) {
    auto *myBigQ = (BigQ*) bigQ;
    myBigQ->ExecuteSortPhase();
    myBigQ->ExecuteMergePhase();
    return nullptr;
}

BigQ::BigQ(Pipe &input, Pipe &output, OrderMaker &sortOrder, int runLength) {
    this->input = &input;
    this->output = &output;
    this->sortOrder = &sortOrder;
    this->runLength = runLength;

    tempFile = new File();
    tempFile->Open(0, tempFileName);

    comparisonEngine = new ComparisonEngine();

    pthread_t worker;
    pthread_create(&worker, nullptr, Worker, (void *) this);
}

// Reads data from the input pipe and sort them into run-length pages.
void BigQ::ExecuteSortPhase() {
    int currentSize = 0, capacity = runLength * PAGE_SIZE;

    Page tempPage;
    off_t pageIndex = 0;

    Record tempRec;
    Record *copyRec;

    vector<Record*> records;

    // Run till the input pipe contains records.
    while (input->Remove(&tempRec)) {
        copyRec = new Record();
        copyRec->Consume(&tempRec);

        currentSize += copyRec->GetSize();

        // If more records can be added.
        if (currentSize <= capacity) {
            records.push_back(copyRec);
        }
        // If not.
        else {
            // Sort the current record list in ascending order.
            sort(records.begin(), records.end(), [this](Record *left, Record *right) {
                return comparisonEngine->Compare(left, right, sortOrder) < 0;
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

            // Write off sentinel.
            runIndexes.push_back(pageIndex);

            // Clear records list and update current-size
            for (auto & rec : records) delete rec;
            records.clear();
            records.push_back(copyRec);
            currentSize = sizeof(int) + copyRec->GetSize();
        }
    }

    // Write off the last bunch of records which never exceeded capacity.
    // Sort the current record list in ascending order.
    sort(records.begin(), records.end(), [this](Record *left, Record *right) {
        return comparisonEngine->Compare(left, right, sortOrder) < 0;
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

    // Write off sentinel.
    runIndexes.push_back(pageIndex);

    // Free memory.
    for (auto & rec : records) delete rec;
    records.clear();
}

// Construct priority queue over sorted runs and dump sorted data into the out pipe.
void BigQ::ExecuteMergePhase() {
    // Custom comparator that defines the order for our priority queue.
    auto comparator = [this](Run *left, Run *right) {
        return comparisonEngine->Compare(left->currentRec, right->currentRec, sortOrder) >=0;
    };

    priority_queue<Run*, vector<Run*>, decltype(comparator)> PQ(comparator);

    off_t prev = 0;
    for (auto & index : runIndexes) {
        PQ.push(new Run(tempFile, prev, index));
        prev = index;
    }

    Record tempRec;
    Run *tempRun;
    while (!PQ.empty()) {
        tempRun = PQ.top();
        PQ.pop();

        // Next returns 1 when there are more records left to be sorted from this run, and 0 otherwise.
        if (tempRun->Next(&tempRec)) {
            // Push the Run into priority queue again if Next returns 1.
            PQ.push(tempRun);
        }

        // Insert into output pipe.
        output->Insert(&tempRec);
    }
    delete tempRun;

    output->ShutDown();

    tempFile->Close();
    remove(tempFileName);
}

BigQ::~BigQ() {
    delete tempFile;
    delete comparisonEngine;
}