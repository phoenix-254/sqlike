#include "Heap.h"
#include "Sorted.h"

void Sorted::Read() {
    if (mode != READ) {
        mode = READ;
        Merge();

        delete inputPipe;delete outputPipe;delete bigQ;
        inputPipe = nullptr, outputPipe = nullptr, bigQ = nullptr;

        // Reset query-order and cnf-order.
        queryOrder = cnfOrder = nullptr;
    }
}

void Sorted::Write() {
    if (mode != WRITE) {
        mode = WRITE;

        inputPipe = new Pipe(PIPE_BUFFER_SIZE);
        outputPipe = new Pipe(PIPE_BUFFER_SIZE);

        // Invoke BigQ constructor which will sort the records from input pipe and
        // write the merged records into output pipe.
        bigQ = new BigQ(*inputPipe, *outputPipe, *sortOrder, runLength);

        // Reset query-order and cnf-order.
        queryOrder = cnfOrder = nullptr;
    }
}

// Merges the internal data hold by BigQ with the already sorted data hold in File instance
// and saves the result in File instance. Similar to merge phase of merge-sort.
// This clears off the BigQ once done.
void Sorted::Merge() {
    inputPipe->ShutDown();

    if (file->GetLength() > 0) {
        MoveFirst();
    }

    // File to hold merged records.
    Heap *mergedFile = new Heap();
    mergedFile->Create(tempMergeFileLocation.c_str(), HEAP, nullptr);

    auto *fileRecord = new Record();
    bool fileHasRecord = GetNext(*fileRecord);

    auto *pipeRecord = new Record();
    bool pipeHasRecord = outputPipe->Remove(pipeRecord);

    while (fileHasRecord && pipeHasRecord) {
        // If fileRecord < pipeRecord
        if (comparisonEngine->Compare(fileRecord, pipeRecord, sortOrder) < 0) {
            mergedFile->Add(*fileRecord);
            fileHasRecord = GetNext(*fileRecord);
        }
        else {
            mergedFile->Add(*pipeRecord);
            pipeHasRecord = outputPipe->Remove(pipeRecord);
        }
    }

    while (fileHasRecord) {
        mergedFile->Add(*fileRecord);
        fileHasRecord = GetNext(*fileRecord);
    }

    while (pipeHasRecord) {
        mergedFile->Add(*pipeRecord);
        pipeHasRecord = outputPipe->Remove(pipeRecord);
    }

    delete fileRecord;
    delete pipeRecord;

    outputPipe->ShutDown();

    mergedFile->CloseFile();
    delete mergedFile;

    // Update pointer of current file to point to the newly merged file.
    remove(fileLocation.c_str());
    rename(tempMergeFileLocation.c_str(), fileLocation.c_str());
    file->Open(1, fileLocation.c_str());

    MoveFirst();
}

// Builds an OrderMaker instance using the file's current sort-order and CNF passed
// so that we can make use of a Binary Search for finding the literal record efficiently.
void Sorted::BuildQueryOrder(CNF &cnf) {
    queryOrder = new OrderMaker();
    cnfOrder = new OrderMaker();

    // Iterate through all attributes of file's sort order from start.
    for (int i = 0; i < sortOrder->numberOfAttrs; i++) {
        bool found = false;

        // Iterate through all the disjunctions of CNF and find the ones
        // acceptable in query.
        for (int j = 0; j < cnf.numberOfAnds; j++) {
            // If disjunction length is not 1 or if the comparison operator is other than equals,
            // skip this disjunction.
            if (cnf.orLens[j] != 1 || cnf.orList[j][0].compOp != Equals) {
                continue;
            }

            // Check if the disjunction is comparing an attribute with a literal value, and the other
            // operand is matching the attribute we are seeking.
            if (cnf.orList[j][0].operand1 == Literal && cnf.orList[j][0].whichAttr2 == sortOrder->whichAttrs[i]) {
                found = true;

                queryOrder->whichAttrs[queryOrder->numberOfAttrs] = cnf.orList[j][0].whichAttr2;
                cnfOrder->whichAttrs[cnfOrder->numberOfAttrs] = cnf.orList[j][0].whichAttr1;
            }
            else if (cnf.orList[j][0].operand2 == Literal && cnf.orList[j][0].whichAttr1 == sortOrder->whichAttrs[i]) {
                found = true;

                queryOrder->whichAttrs[queryOrder->numberOfAttrs] = cnf.orList[j][0].whichAttr1;
                cnfOrder->whichAttrs[cnfOrder->numberOfAttrs] = cnf.orList[j][0].whichAttr2;
            }
            else {
                found = false;
            }

            if (found) {
                queryOrder->whichTypes[queryOrder->numberOfAttrs] = sortOrder->whichTypes[i];
                cnfOrder->whichTypes[cnfOrder->numberOfAttrs] = sortOrder->whichTypes[i];

                queryOrder->numberOfAttrs++;
                cnfOrder->numberOfAttrs++;

                break;
            }
        }

        if (!found) break;
    }
}

Sorted::Sorted() {
    Init();
}

Sorted::Sorted(OrderMaker *orderMaker, int runLength) {
    this->sortOrder = orderMaker;
    this->runLength = runLength;

    Init();
}

void Sorted::Init() {
    mode = READ;

    file = new File();

    bufferedPage = new Page();

    currentPagePtr = 0;

    comparisonEngine = new ComparisonEngine();

    queryOrder = cnfOrder = nullptr;
}

Sorted::~Sorted() {
    delete file;
    delete comparisonEngine;
    delete queryOrder;
}

int Sorted::Create(const char *filePath, fileType type, void *startUp) {
    fileLocation = string(filePath);

    size_t index = fileLocation.find_last_of("/\\");
    tempMergeFileLocation = fileLocation.substr(0, index + 1);
    tempMergeFileLocation.append("merge.bin");

    // Parse startup arguments.
    typedef struct { OrderMaker *sortOrder; int runLength; } *Args;
    Args args = (Args) startUp;
    sortOrder = args->sortOrder;
    runLength = args->runLength;

    // Create bin file.
    file->Open(0, filePath);

    return 1;
}

int Sorted::Open(const char *filePath) {
    this->fileLocation = string(filePath);

    size_t index = fileLocation.find_last_of("/\\");
    tempMergeFileLocation = fileLocation.substr(0, index + 1);
    tempMergeFileLocation.append("merge.bin");

    file->Open(1, fileLocation.c_str());
    return 1;
}

int Sorted::Close() {
    // Write off meta data.
    string metaFilePath(fileLocation);
    metaFilePath.append(".meta");

    ofstream metaFile;
    metaFile.open(metaFilePath.c_str());
    if (!metaFile.is_open()) {
        cout << "ERROR : Unable to create meta file " << metaFilePath << endl;
        exit(1);
    }

    metaFile << FILE_TYPE_SORTED << "\n";
    metaFile << runLength << "\n";
    metaFile << sortOrder->ToString() << "\n";
    metaFile.close();

    // Write off actual data.
    Read();

    return file->Close();
}

void Sorted::Load(Schema &schema, const char *loadPath) {
    Write();

    int count = 0;

    FILE *tblFile = fopen(loadPath, "r");
    if (tblFile != nullptr) {
        // Read all the records from the tbl file and add it to our page object one by one.
        Record next;
        while (next.SuckNextRecord(schema, tblFile)) {
            inputPipe->Insert(&next);
            count++;
        }
        fclose(tblFile);
    }
    else {
        cout << "Unable to open file " << tblFile << endl;
        exit(1);
    }

    cout << "Loaded " << count << " records from " << loadPath << endl;
}

void Sorted::MoveFirst() {
    Read();

    currentPagePtr = 0;
    bufferedPage->EmptyItOut();
    if (file->GetLength() > 0) {
        file->GetPage(bufferedPage, currentPagePtr++);
    }
}

void Sorted::Add(Record &addMe) {
    Write();
    inputPipe->Insert(&addMe);
}

int Sorted::GetNext(Record &fetchMe) {
    Read();

    /*
     * Check if there are any records left to be read on the current readPage, and just update the
     * fetchMe with the value of next record from this readPage.
     * If not, update readPage to point to the next page in the file, and then update the value
     * of fetchMe for this new readPage.
     *
     * Return 0 if reached the end of the file, else return 1.
     */
    if (!bufferedPage->GetFirst(&fetchMe)) {
        if (currentPagePtr < file->GetLength() - 1) {
            file->GetPage(bufferedPage, currentPagePtr++);
            bufferedPage->GetFirst(&fetchMe);
        }
        else
            return 0;
    }
    return 1;
}

int Sorted::GetNext(Record &fetchMe, CNF &cnf, Record &literal) {
    Read();

    /*
     * We'll store the query-order generated and reuse it for the current session of our
     * sorted file, since we can assume that the caller has passed the same parameters for
     * CNF in subsequent calls to GetNext, and in practise it will never be the case that
     * the caller switches parameters without any intervening call to some other function.
     */
    bool isCnfUpdated = false;
    if (queryOrder == nullptr || cnfOrder == nullptr) {
        isCnfUpdated = true;
        BuildQueryOrder(cnf);
    }

    if (queryOrder->numberOfAttrs > 0) {
        return GetNextBinarySearch(fetchMe, cnf, literal, isCnfUpdated);
    } else {
        return GetNextLinearSearch(fetchMe, cnf, literal);
    }
}

int Sorted::GetNextBinarySearch(Record &fetchMe, CNF &cnf, Record &literal, bool isCnfUpdated) {
    if (isCnfUpdated) {
        off_t index = BinarySearch(currentPagePtr, file->GetLength() - 2, literal);
        if (index == -1) {
            return 0;
        } else {
            currentPagePtr = index;
            bufferedPage->EmptyItOut();
            file->GetPage(bufferedPage, currentPagePtr++);
        }
    }

    while (GetNext(fetchMe)) {
        if (comparisonEngine->Compare(&fetchMe, queryOrder, &literal, cnfOrder) == 0
            && comparisonEngine->Compare(&fetchMe, &literal, &cnf)) {
            return 1;
        }
    }

    return 0;
}

off_t Sorted::BinarySearch(off_t low, off_t high, Record &literal) {
    // Base condition.
    if (low == high)
        return low;

    off_t mid = (low + high) / 2;

    Page tempPage;
    file->GetPage(&tempPage, mid);

    Record tempRec;
    tempPage.GetFirst(&tempRec);

    if (comparisonEngine->Compare(&tempRec, queryOrder, &literal, cnfOrder) < 0) {
        return BinarySearch(mid + 1, high, literal);
    }

    return BinarySearch(low, mid - 1, literal);
}

/*
 * Infinitely look for the next record satisfying the given CNF until we find one.
 * In each iteration of the while loop there are 3 possibilities.
 * 1. Reached to the end of the file - return 0.
 * 2. Found the record matching CNF - return 1.
 * 3. Record does not satisfy the CNF - move to next record.
 */
int Sorted::GetNextLinearSearch(Record &fetchMe, CNF &cnf, Record &literal) {
    while (true) {
        int hasRecord = GetNext(fetchMe);
        if (!hasRecord) return 0;

        if (comparisonEngine->Compare(&fetchMe, &literal, &cnf)) {
            return 1;
        }
    }
}
