#include "DBFile.h"
#include "test.h"

Relation *rel;

void *Producer(void *arg) {
    Pipe *pipe = (Pipe *) arg;

    DBFile dbFile;
    dbFile.Open(rel->GetBinFilePath().c_str());
    dbFile.MoveFirst();
    cout << "Producer : Successfully opened DBFile " << rel->GetBinFilePath() << endl;

    Record temp;
    int count = 0;
    while (dbFile.GetNext(temp) == 1) {
        count++;
        if (count % 100000 == 0) {
            cerr << "Producer : " << count << endl;
        }

        pipe->Insert(&temp);
    }

    dbFile.Close();

    pipe->ShutDown();

    cout << "Producer : Successfully inserted " << count << " records into the pipe." << endl;
}

void *Consumer(void *arg) {
    auto *t = (TestUtil *) arg;

    ComparisonEngine comparisonEngine;

    DBFile dbFile;
    char outputFile[100];
    if (t->write) {
        sprintf(outputFile, "%s.bigq", rel->GetBinFilePath().c_str());
        dbFile.Create(outputFile, HEAP, nullptr);
    }

    Record recs[2];
    Record *prev = nullptr, *last = nullptr;

    int i = 0, err = 0;
    while (t->pipe->Remove(&recs[i % 2])) {
        prev = last;
        last = &recs[i % 2];

        if (prev && last) {
            if (comparisonEngine.Compare(prev, last, t->order) == 1) {
                err++;
            }

            if (t->write) {
                dbFile.Add(*prev);
            }
        }

        if (t->print) {
            if (last) last->Print(*(rel->GetSchema()));
        }

        i++;
    }

    cout << "Consumer : removed " << i << " records from the pipe." << endl;

    if (t->write) {
        if (last) dbFile.Add(*last);
        cerr << "Consumer : records removed written out as HEAP DBFile at " << outputFile << endl;
        dbFile.Close();
    }

    cerr << "Consumer : " << (i-err) << " records out of " << i << " records in sorted order." << endl;

    if (err) {
        cerr << "Consumer : " << err << " records failed sorted order test." << endl << endl;
    }
}

void Test(int selectedOption, int runLength) {
    // sort order for the records.
    OrderMaker sortOrder;
    rel->GetSortOrder(sortOrder);

    // Pipe cache size
    int bufferSize = 100;

    Pipe input(bufferSize);
    Pipe output(bufferSize);

    // thread to dump data into the input pipe (for BigQ's consumption)
    pthread_t thread1;
    pthread_create(&thread1, nullptr, Producer, (void *)&input);

    // thread to read sorted data from output pipe (dumped by BigQ)
    pthread_t thread2;
    TestUtil testUtil = {&output, &sortOrder, selectedOption == 2, selectedOption == 3};
    pthread_create(&thread2, nullptr, Consumer, (void *)&testUtil);

    //BigQ bq (input, output, sortorder, runlen);

    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);
}

int main() {
    Setup();

    int option = 0;
    while (option < 1 || option > 3) {
        cout << ">>>> Select Test: " << endl;
        cout << "\t1. Sort" << endl;
        cout << "\t2. Sort + Display" << endl;
        cout << "\t3. Sort + Write" << endl;

        cin >> option;
        if (option < 1 || option > 3)
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

    int runLength = 0;
    cout << "Specify run length: " << endl;
    cin >> runLength;

    Test(option, runLength);

    Cleanup();
}
