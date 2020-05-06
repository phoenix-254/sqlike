#include <iostream>
#include <ostream>
#include <string>

#include "DBFile.h"

using namespace std;

void DBFile::WriteToFile() {
    file->AddPage(writePage, writeIndex++);
    writePage->EmptyItOut();
}

DBFile::DBFile() {
    file = new File();
    readPage = new Page();
    writePage = new Page();
    readIndex = writeIndex = 1;
}

DBFile::~DBFile() {
    delete file;
    delete readPage;
    delete writePage;
}

int DBFile::Create(const char *filePath, fileType type, void *startUp) {
    if (filePath == nullptr || type < HEAP || type > TREE) return 0;

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

int DBFile::Open(const char *filePath) {}

int DBFile::Close() {
    return file->Close();
}

void DBFile::Load(Schema &schema, const char *loadPath) {
    int numOfRecs = 0;

    FILE *tblFile = fopen(loadPath, "r");
    if (tblFile != nullptr) {
        Record next;
        while (next.SuckNextRecord(schema, tblFile)) {
            Add(next);
            numOfRecs++;
        }
        fclose(tblFile);
    }
    else {
        cout << "Unable to open file " << tblFile << endl;
        exit(1);
    }

    cout << "Loaded " << numOfRecs << " from " << loadPath << "!" << endl;
}

void DBFile::MoveFirst() {}

void DBFile::Add(Record &addMe) {
    if (!writePage->Append(&addMe)) {
        WriteToFile();
        writePage->Append(&addMe);
    }
}

int DBFile::GetNext(Record &fetchMe) {}

int DBFile::GetNext(Record &fetchMe, CNF &cnf, Record &literal) {}