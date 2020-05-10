#include <iostream>

#include "DBFile.h"
#include "a1-test.h"

Relation *rel;

void LoadTest() {
    cout << endl << ">>>> Loading " << rel->GetRelationName() << endl;

    DBFile dbFile;
    dbFile.Create(rel->GetBinFilePath().c_str(), HEAP, nullptr);
    dbFile.Load(*(rel->GetSchema()), rel->GetTblFilePath().c_str());
    dbFile.Close();
}

void ScanTest() {
    cout << endl << ">>>> Scanning " << rel->GetRelationName() << endl;

    DBFile dbFile;
    dbFile.Open(rel->GetBinFilePath().c_str());
    dbFile.MoveFirst();

    Record temp;
    int counter = 0;
    while (dbFile.GetNext(temp) == 1) {
        counter++;
        temp.Print(*(rel->GetSchema()));
        if (counter % 10000 == 0) {
            cout << counter << endl;
        }
    }

    cout << "Scanned " << counter << " records." << endl;

    dbFile.Close();
}

void ScanAndFilterTest() {
    CNF cnf{};
    Record literal;
    rel->GetCnf(cnf, literal);

    cout << endl << ">>>> Scanning " << rel->GetRelationName() << " With Condition : ";
    cnf.Print();

    DBFile dbFile;
    dbFile.Open(rel->GetBinFilePath().c_str());
    dbFile.MoveFirst();

    Record temp;
    int counter = 0;
    while (dbFile.GetNext(temp, cnf, literal) == 1) {
        counter++;
        temp.Print(*(rel->GetSchema()));
        if (counter % 10000 == 0) {
            cout << counter << endl;
        }
    }

    cout << "Scanned " << counter << " records." << endl;

    dbFile.Close();
}

int main() {
    Setup();

    int testToRun = 0;
    while (testToRun < 1 || testToRun > 3) {
        cout << ">>>> Select Test: " << endl;
        cout << "\t1. Load file" << endl;
        cout << "\t2. Scan all the records" << endl;
        cout << "\t3. Scan filtered records" << endl;

        cin >> testToRun;
        if (testToRun < 1 || testToRun > 3)
            cout << "Invalid choice, please try again!" << endl << endl;
    }

    int tableIndex = 0;
    while (tableIndex < 1 || tableIndex > 8) {
        cout << ">>>> Select Table: " << endl;
        for (auto & t : TABLE_MAP) {
            cout << "\t" << t.first << ". " << t.second << endl;
        }

        cin >> tableIndex;
        if (tableIndex < 1 || tableIndex > 8)
            cout << "Invalid choice, please try again!" << endl << endl;
    }

    rel = relations.find(tableIndex)->second;

    // Making array of all test functions above and calling appropriate function based on
    // what user has selected.
    void (*test) ();
    void (*testPtr[]) () = {&LoadTest, &ScanTest, &ScanAndFilterTest};
    // As array index starts from 0, hence deducted 1.
    test = testPtr[testToRun - 1];
    test();

    Cleanup();
}
