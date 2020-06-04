#include "Heap.h"
#include "Sorted.h"

void Sorted::Read() {
    if (mode != READ) {
        mode = READ;
        Merge();

        delete inputPipe;delete outputPipe;delete bigQ;
        inputPipe = nullptr, outputPipe = nullptr, bigQ = nullptr;
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
    string mergedBinFilePath("../bin/1gb/");
    mergedBinFilePath.append("merge.bin");
    Heap *mergedFile = new Heap();
    mergedFile->Create(mergedBinFilePath.c_str(), HEAP, nullptr);

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
    rename(mergedBinFilePath.c_str(), fileLocation.c_str());
    file->Open(1, fileLocation.c_str());

    MoveFirst();
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
}

Sorted::~Sorted() {
    delete file;
    delete comparisonEngine;
}

int Sorted::Create(const char *filePath, fileType type, void *startUp) {
    this->fileLocation = string(filePath);

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
    file->Open(1, filePath);
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

    metaFile << FILE_TYPE_SORTED << "\n" << runLength;
    metaFile.write((char *) &sortOrder, sizeof(sortOrder));
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

}
