#include <iostream>
#include <ostream>
#include <string>
#include <cstring>

#include "DBFile.h"
#include "Heap.h"
#include "Sorted.h"

using namespace std;

DBFile::DBFile() {
    file = nullptr;
}

DBFile::~DBFile() {
    delete file;
}

int DBFile::Create(const char *filePath, fileType type, void *startUp) {
    if (filePath == nullptr || type < HEAP || type > TREE) return 0;

    if (type == HEAP) {
        file = new Heap();
    }
    else if (type == SORTED) {
        file = new Sorted();
    }
    else {
        cout << "Invalid file type!" << endl;
        exit(1);
    }

    return 1;
}

int DBFile::Open(const char *filePath) {
    if (filePath == nullptr) return 0;

    // Get file-type info from meta file.
    ifstream metaFile;
    string metaFilePath(filePath);
    metaFilePath.append(".meta");

    metaFile.open(metaFilePath.c_str());

    if (metaFile.is_open()) {
        string type;
        getline(metaFile, type);

        if (strcmp(type.c_str(), FILE_TYPE_HEAP) == 0) {
            file = new Heap();
        }
        else if (strcmp(type.c_str(), FILE_TYPE_SORTED) == 0) {
            file = new Sorted();
        }
        else {
            cout << "Invalid file type value \"" << type << "\" stored in meta file." << endl;
            exit(1);
        }

        metaFile.close();
    }
    else {
        cout << "ERROR : Unable to create meta file " << metaFilePath << endl;
        return 0;
    }

    return 1;
}

int DBFile::Close() {
    return file->Close();
}

void DBFile::Load(Schema &schema, const char *loadPath) {
    file->Load(schema, loadPath);
}

void DBFile::MoveFirst() {
    file->MoveFirst();
}

void DBFile::Add(Record &addMe) {
    file->Add(addMe);
}

int DBFile::GetNext(Record &fetchMe) {
    return file->GetNext(fetchMe);
}

int DBFile::GetNext(Record &fetchMe, CNF &cnf, Record &literal) {
    return file->GetNext(fetchMe, cnf, literal);
}