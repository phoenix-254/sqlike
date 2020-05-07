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
    readPtr = writePtr = 1;
}

DBFile::~DBFile() {
    delete file;
    delete readPage;
    delete writePage;
}

int DBFile::Create(const char *filePath, fileType type, void *startUp) {
    if (filePath == nullptr || type < HEAP || type > TREE) return 0;

    // Reset read and write pointers.
    readPtr = writePtr = 1;

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

    readPtr = 1;

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

    cout << "Loaded " << count << " records from " << loadPath << "!" << endl;
}

void DBFile::MoveFirst() {}

void DBFile::Add(Record &addMe) {
    /*
     * Check if there's enough space left on this page.
     * If not, write this page to the file, empty the page and add this record to it, Else do nothing.
     */
    if (!writePage->Append(&addMe)) {
        WriteToFile();
        writePage->Append(&addMe);
    }
}

int DBFile::GetNext(Record &fetchMe) {}

int DBFile::GetNext(Record &fetchMe, CNF &cnf, Record &literal) {}