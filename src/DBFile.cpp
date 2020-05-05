#include "DBFile.h"

DBFile::DBFile() {}

DBFile::~DBFile() {}

int DBFile::Create(const char *filePath, fileType type, void *startUp) {}

int DBFile::Open(const char *filePath) {}

int DBFile::Close() {}

void DBFile::Load(Schema &schema, const char *loadPath) {}

void DBFile::MoveFirst() {}

void DBFile::Add(Record &addMe) {}

int DBFile::GetNext(Record &fetchMe) {}

int DBFile::GetNext(Record &fetchMe, CNF &cnf, Record &literal) {}