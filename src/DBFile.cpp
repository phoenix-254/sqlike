#include <iostream>
#include <ostream>
#include <string>

#include "DBFile.h"

using namespace std;

void DBFile::WriteToFile() {
    file->AddPage(writePage, writePtr++);
    writePage->EmptyItOut();
}

DBFile::DBFile() {
    file = new File();

    readPage = new Page();
    writePage = new Page();

    readPtr = writePtr = 0;

    comparisonEngine = new ComparisonEngine();
}

DBFile::~DBFile() {
    delete file;
    delete readPage;
    delete writePage;
    delete comparisonEngine;
}

int DBFile::Create(const char *filePath, fileType type, void *startUp) {
    if (filePath == nullptr || type < HEAP || type > TREE) return 0;

    // Reset read and write pointers.
    readPtr = writePtr = 0;

    // Create bin file.
    file->Open(0, filePath);

    // Create meta file, and store the file-type into it.
    string metaFilePath(filePath);
    metaFilePath.append(".meta");
    ofstream metaFile;
    metaFile.open(metaFilePath.c_str());
    if (!metaFile.is_open()) {
        cout << "ERROR : Unable to create meta file " << metaFilePath << endl;
        exit(1);
    }
    metaFile << type;
    metaFile.close();

    return 1;
}

int DBFile::Open(const char *filePath) {
    if (filePath == nullptr) return 0;

    // Opens bin file.
    file->Open(1, filePath);
    return 1;
}

int DBFile::Close() {
    // Write off the last page.
    if (writePage->GetNumberOfRecs() > 0) WriteToFile();
    return file->Close();
}

void DBFile::Load(Schema &schema, const char *loadPath) {
    int count = 0;

    FILE *tblFile = fopen(loadPath, "r");
    if (tblFile != nullptr) {
        // Read all the records from the tbl file and add it to our page object one by one.
        Record next;
        while (next.SuckNextRecord(schema, tblFile)) {
            Add(next);
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

void DBFile::MoveFirst() {
    readPtr = 0;
    file->GetPage(readPage, readPtr++);
}

void DBFile::Add(Record &addMe) {
    /*
     * Check if there's enough space left on this page.
     * If not, write this page to the file, empty the page and add this record to it, Else just add the record.
     */
    if (!writePage->Append(&addMe)) {
        WriteToFile();
        writePage->Append(&addMe);
    }
}

// Gets the next record from the file.
int DBFile::GetNext(Record &fetchMe) {
    /*
     * Check if there are any records left to be read on the current readPage, and just update the
     * fetchMe with the value of next record from this readPage.
     * If not, update readPage to point to the next page in the file, and then update the value
     * of fetchMe for this new readPage.
     *
     * Return 0 if reached the end of the file, else return 1.
     */
    if (!readPage->GetFirst(&fetchMe)) {
        if (readPtr < file->GetLength() - 1) {
            file->GetPage(readPage, readPtr++);
            readPage->GetFirst(&fetchMe);
        }
        else
            return 0;
    }
    return 1;
}

int DBFile::GetNext(Record &fetchMe, CNF &cnf, Record &literal) {
    /*
     * Infinitely look for the next record satisfying the given CNF until we find one.
     * In each iteration of the while loop there are 3 possibilities.
     * 1. Reached to the end of the file - return 0.
     * 2. Found the record matching CNF - return 1.
     * 3. Record does not satisfy the CNF - move to next record.
     */
    while (true) {
        int hasRecord = GetNext(fetchMe);
        if (!hasRecord) return 0;

        if (comparisonEngine->Compare(&fetchMe, &literal, &cnf)) {
            return 1;
        }
    }
}