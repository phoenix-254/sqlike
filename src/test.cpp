#include <cmath>
#include <string>
#include <sstream>

#include "BigQ.h"
#include "DBFile.h"
#include "test.h"

Relation *rel;

void Test1();
void Test2();
void Test3();

int AddData(FILE *tblFileSrc, int numberOfRecs, int &res) {
    DBFile dbFile;
    dbFile.Open(rel->GetBinFilePath().c_str());

    Record temp;
    int processedRecs = 0, threshold = 20000;
    while ((res = temp.SuckNextRecord(*(rel->GetSchema()), tblFileSrc) && ++processedRecs < numberOfRecs)) {
        dbFile.Add(temp);

        if (processedRecs == threshold) cerr << "\t";
        if (processedRecs % threshold == 0) cerr << ".";
    }

    dbFile.Close();

    return processedRecs;
}

void Test1() {
    int runLength = 0;
    cout << ">>>> Specify run length: " << endl;
    cin >> runLength;

    // Determine how many records to add. (Moved this up since we cannot take
    // user input after taking the sort-order and user inputs EOF.)
    int option = 0;
    while (option < 1 || option > 2) {
        cout << ">>>> Select how many records to add in : " << rel->GetBinFilePath() << endl;
        cout << "\t1. Add few records (1 to 1k)" << endl;
        cout << "\t2. Add lots of records (1k to 1e+06)" << endl;

        cin >> option;

        if (option < 1 || option > 2)
            cout << "Invalid choice, please try again!" << endl;
    }

    // sort order for the records.
    OrderMaker sortOrder;
    rel->GetSortOrder(sortOrder);

    cout << ">>>> Creating sorted db file : " << rel->GetBinFilePath() << endl;
    struct { OrderMaker *sortOrder; int runLength; } startup = { &sortOrder, runLength };
    DBFile dbFile;
    dbFile.Create(rel->GetBinFilePath().c_str(), SORTED, &startup);
    dbFile.Close();

    cout << endl << ">>>> Reading from tbl file : " << rel->GetTblFilePath() << endl;

    FILE *tblFile = fopen(rel->GetTblFilePath().c_str(), "r");
    srand48(time(nullptr));

    int totalRecs = 0, processedRecs = 1, res = 1;
    while (processedRecs && res) {
        processedRecs = AddData(tblFile, lrand48() % (int) pow(1e3, option) + (option - 1) * 1000, res);
        totalRecs += processedRecs;
        if (processedRecs) {
            cout << "\n\tAdded " << processedRecs << " records. So far total is " << totalRecs << "." << endl;
        }
    }

    fclose(tblFile);

    cout << endl << "Create finished. Total " << totalRecs << " records inserted." << endl;
}

void Test2() {
    cout << ">>>> Scanning a db file : " << rel->GetBinFilePath() << "\n";

    DBFile dbFile;
    dbFile.Open(rel->GetBinFilePath().c_str());
    dbFile.MoveFirst();

    Record temp;
    int counter = 0;
    while (dbFile.GetNext(temp) && ++counter) {
        temp.Print(*(rel->GetSchema()));
        if (counter % 10000 == 0) {
            cerr << ".";
        }
    }

    dbFile.Close();

    cout << "Scanned " << counter << " records." << endl;
}

void Test3() {
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
    while (dbFile.GetNext(temp, cnf, literal) && ++counter) {
        temp.Print(*(rel->GetSchema()));
        if (counter % 10000 == 0) {
            cerr << ".";
        }
    }

    dbFile.Close();

    cout << "Scanned " << counter << " records." << endl;
}

int main() {
    Setup();

    int testToRun = 0;
    while (testToRun < 1 || testToRun > 3) {
        cout << ">>>> Select Test: " << endl;
        cout << "\t1. Create a sorted db file" << endl;
        cout << "\t2. Scan a db file" << endl;
        cout << "\t3. Run a query" << endl;

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
    void (*testPtr[]) () = {&Test1, &Test2, &Test3};
    // As array index starts from 0, hence deducted 1.
    test = testPtr[testToRun - 1];
    test();

    Cleanup();
}
