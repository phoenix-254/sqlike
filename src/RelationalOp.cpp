#include "BigQ.h"
#include "ComparisonEngine.h"
#include "RelationalOp.h"

#include <vector>

void RelationalOp::WaitUntilDone() const {
    pthread_join(thread, nullptr);
}

void RelationalOp::Use_n_Pages(int n) {
    this->runLength = n;
}

void *SelectFileExecute(void *args) {
    auto *myArgs = (SelectFileArgs*) args;

    Record temp;
    while (myArgs->inputFile->GetNext(temp, *myArgs->selectionOp, *myArgs->literal)) {
        myArgs->outputPipe->Insert(&temp);
    }

    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void SelectFile::Run(DBFile &inputFile, Pipe &outputPipe, CNF &selectionOp, Record &literal) {
    auto *args = new SelectFileArgs();
    args->inputFile = &inputFile;
    args->outputPipe = &outputPipe;
    args->selectionOp = &selectionOp;
    args->literal = &literal;

    pthread_create(&thread, nullptr, SelectFileExecute, (void *) args);
}

void *SelectPipeExecute(void *args) {
    auto *myArgs = (SelectPipeArgs*) args;

    Record temp;
    ComparisonEngine comparisonEngine;
    while (myArgs->inputPipe->Remove(&temp)) {
        if (comparisonEngine.Compare(&temp, myArgs->literal, myArgs->selectionOp)) {
            myArgs->outputPipe->Insert(&temp);
        }
    }

    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void SelectPipe::Run(Pipe &inputPipe, Pipe &outputPipe, CNF &selectionOp, Record &literal) {
    auto *args = new SelectPipeArgs();
    args->inputPipe = &inputPipe;
    args->outputPipe = &outputPipe;
    args->selectionOp = &selectionOp;
    args->literal = &literal;

    pthread_create(&thread, nullptr, SelectPipeExecute, (void *) args);
}

void *ProjectExecute(void *args) {
    auto *myArgs = (ProjectArgs*) args;

    Record temp;
    while (myArgs->inputPipe->Remove(&temp)) {
        temp.Project(myArgs->attrsToKeep, myArgs->outputAttrCount, myArgs->inputAttrCount);
        myArgs->outputPipe->Insert(&temp);
    }

    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void Project::Run(Pipe &inputPipe, Pipe &outputPipe, int *attrsToKeep, int inputAttrCount, int outputAttrCount) {
    auto *args = new ProjectArgs();
    args->inputPipe = &inputPipe;
    args->outputPipe = &outputPipe;
    args->attrsToKeep = attrsToKeep;
    args->inputAttrCount = inputAttrCount;
    args->outputAttrCount = outputAttrCount;

    pthread_create(&thread, nullptr, ProjectExecute, (void *) args);
}

void *JoinExecute(void *args) {
    auto *myArgs = (JoinArgs*) args;

    OrderMaker leftOrderMaker, rightOrderMaker;
    bool cnfHasEqualityCheck = myArgs->selectionOp->GetSortOrder(leftOrderMaker, rightOrderMaker);

    if (cnfHasEqualityCheck) {
        Pipe leftOutputPipe(PIPE_BUFFER_SIZE);
        BigQ leftBigQ(*myArgs->inputPipeLeft, leftOutputPipe, leftOrderMaker, myArgs->runLength);

        Pipe rightOutputPipe(PIPE_BUFFER_SIZE);
        BigQ rightBigQ(*myArgs->inputPipeRight, rightOutputPipe, rightOrderMaker, myArgs->runLength);

        PerformSortMergeJoin(&leftOutputPipe, &rightOutputPipe, myArgs->outputPipe,
                &leftOrderMaker, &rightOrderMaker);
    }
    else {
        PerformBlockNestedLoopsJoin(myArgs->inputPipeLeft, myArgs->inputPipeRight, myArgs->outputPipe, myArgs->runLength);
    }

    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void PerformSortMergeJoin(Pipe *leftInputPipe, Pipe *rightInputPipe, Pipe *outputPipe,
        OrderMaker *leftOrderMaker, OrderMaker *rightOrderMaker) {
    Record tempRecLeft, tempRecRight;

    bool hasDataInLeftPipe = leftInputPipe->Remove(&tempRecLeft);
    bool hasDataInRightPipe = rightInputPipe->Remove(&tempRecRight);

    ComparisonEngine comparisonEngine;

    // Iterate while there is data in both the input pipes.
    while (hasDataInLeftPipe && hasDataInRightPipe) {
        int compVal = comparisonEngine.Compare(&tempRecLeft, leftOrderMaker, &tempRecRight, rightOrderMaker);

        if (compVal == 0) {
            vector<Record*> leftPipeRecs;

            auto *leftRec = new Record();
            leftRec->Consume(&tempRecLeft);
            leftPipeRecs.push_back(leftRec);

            int lastRecIndex = 0;
            while ((hasDataInLeftPipe = leftInputPipe->Remove(&tempRecLeft)) &&
                comparisonEngine.Compare(leftPipeRecs[lastRecIndex], &tempRecLeft, leftOrderMaker) == 0) {
                leftRec = new Record();
                leftRec->Consume(&tempRecLeft);
                leftPipeRecs.push_back(leftRec);
            }

            vector<Record*> rightPipeRecs;

            auto *rightRec = new Record();
            rightRec->Consume(&tempRecRight);
            rightPipeRecs.push_back(rightRec);

            lastRecIndex = 0;
            while ((hasDataInRightPipe = rightInputPipe->Remove(&tempRecRight)) &&
                   comparisonEngine.Compare(rightPipeRecs[lastRecIndex], &tempRecRight, rightOrderMaker) == 0) {
                rightRec = new Record();
                rightRec->Consume(&tempRecRight);
                rightPipeRecs.push_back(rightRec);
            }

            int numAttrsLeft = leftPipeRecs[0]->GetAttrCount(), numAttrsRight = rightPipeRecs[0]->GetAttrCount();
            int numAttrsTotal = numAttrsLeft + numAttrsRight;

            int *attrsToKeep = new int[numAttrsTotal];
            for (int i = 0; i < numAttrsLeft; i++) attrsToKeep[i] = i;
            for (int i = 0; i < numAttrsRight; i++) attrsToKeep[numAttrsLeft + i] = i;

            auto *mergedRec = new Record();
            for (auto &left : leftPipeRecs) {
                for (auto &right : rightPipeRecs) {
                    mergedRec->MergeRecords(left, right, numAttrsLeft, numAttrsRight,
                            attrsToKeep, numAttrsTotal, numAttrsLeft);
                    outputPipe->Insert(mergedRec);
                }
            }
        }
        else if (compVal < 0) {
            hasDataInLeftPipe = leftInputPipe->Remove(&tempRecLeft);
        }
        else {
            hasDataInRightPipe = rightInputPipe->Remove(&tempRecRight);
        }
    }

    // Clear left pipe.
    while (hasDataInLeftPipe) {
        hasDataInLeftPipe = leftInputPipe->Remove(&tempRecLeft);
    }
    leftInputPipe->ShutDown();

    // Clear right pipe.
    while (hasDataInRightPipe) {
        hasDataInRightPipe = rightInputPipe->Remove(&tempRecRight);
    }
    rightInputPipe->ShutDown();
}

void LoadRecordsFromPageBlock(vector<Record*> *recs, Page *pageBlock, int blockLen) {
    auto *temp = new Record();
    for (int i = 0; i <= blockLen; i++) {
        while (pageBlock[i].GetFirst(temp)) {
            recs->push_back(temp);
            temp = new Record();
        }
    }
    delete temp;
}

void JoinRecords(vector<Record*> *leftRecs, vector<Record*> *rightRecs, Pipe *output) {
    auto *mergedRec = new Record();
    for (auto *leftRec : *leftRecs) {
        for (auto *rightRec : *rightRecs) {
            mergedRec->MergeRecords(leftRec, rightRec);
            output->Insert(mergedRec);
        }
    }
}

void PerformBlockNestedLoopsJoin(Pipe *leftInputPipe, Pipe *rightInputPipe, Pipe *outputPipe, int runLength) {
    // Create temporary HEAP type DB Files which we'll use to store our input pipe records.
    Record temp;
    DBFile leftDbFile, rightDbFile;
    char *tempLeftDbFile = "tempLeftDbFile.bin", *tempRightDbFile = "tempRightDbFile.bin";

    leftDbFile.Create(tempLeftDbFile, HEAP, nullptr);
    while (leftInputPipe->Remove(&temp)) {
        leftDbFile.Add(temp);
    }

    rightDbFile.Create(tempRightDbFile, HEAP, nullptr);
    while (rightInputPipe->Remove(&temp)) {
        rightDbFile.Add(temp);
    }

    // Nested Join to merge all the records from left and right db files.
    Record leftRec, rightRec, mergedRec;
    // Buffer page for storing current run records.
    Page *recordsBlock = new (std::nothrow) Page[runLength];
    if (recordsBlock == nullptr) {
        exit(1);
    }

    leftDbFile.MoveFirst();
    bool hasDataInLeftDbFile = leftDbFile.GetNext(leftRec);

    while (hasDataInLeftDbFile) {
        int index = 0;
        while (hasDataInLeftDbFile) {
            if (!recordsBlock[index].Append(&leftRec)) {
                if (index + 1 < runLength) {
                    index++;
                    recordsBlock[index].Append(&leftRec);
                } else {
                    break;
                }
            }
            hasDataInLeftDbFile = leftDbFile.GetNext(leftRec);
        }
        
        // Load records from page into vector
        vector<Record*> leftBlockRecs;
        LoadRecordsFromPageBlock(&leftBlockRecs, recordsBlock, index);

        rightDbFile.MoveFirst();
        bool hasDataInRightDbFile = rightDbFile.GetNext(rightRec);

        while (hasDataInRightDbFile) {
            index = 0;
            while (hasDataInRightDbFile) {
                if (!recordsBlock[index].Append(&rightRec)) {
                    if (index + 1 < runLength) {
                        index++;
                        recordsBlock[index].Append(&rightRec);
                    } else {
                        break;
                    }
                }
                hasDataInRightDbFile = rightDbFile.GetNext(rightRec);
            }

            // Load records from page into vector
            vector<Record*> rightBlockRecs;
            LoadRecordsFromPageBlock(&rightBlockRecs, recordsBlock, index);

            // Join left and right block of pages.
            JoinRecords(&leftBlockRecs, &rightBlockRecs, outputPipe);
        }
    }

    // Clear
    remove(tempLeftDbFile);
    remove(tempRightDbFile);
}

void Join::Run(Pipe &inputPipeLeft, Pipe &inputPipeRight, Pipe &outputPipe, CNF &selectionOp, Record &literal) {
    auto *args = new JoinArgs();
    args->inputPipeLeft = &inputPipeLeft;
    args->inputPipeRight = &inputPipeRight;
    args->outputPipe = &outputPipe;
    args->selectionOp = &selectionOp;
    args->literal = &literal;
    args->runLength = runLength;

    pthread_create(&thread, nullptr, JoinExecute, (void *) args);
}

void DuplicateRemoval::Run(Pipe &inputPipe, Pipe &outputPipe, Schema &schema) {

}

void *SumExecute(void *args) {
    auto *myArgs = (SumArgs*) args;

    double sum = 0;

    int intVal = 0;
    double doubleVal = 0;

    Record temp;
    while (myArgs->inputPipe->Remove(&temp)) {
        intVal = 0; doubleVal = 0;

        myArgs->function->Apply(temp, intVal, doubleVal);
        sum += (intVal + doubleVal);
    }

    Attribute attr = {"sum", Double};
    Schema sumSchema("sumSchema", 1, &attr);
    temp.ComposeRecord(sumSchema, (std::to_string(sum) + "|").c_str());

    myArgs->outputPipe->Insert(&temp);
    myArgs->outputPipe->ShutDown();

    return nullptr;
}

void Sum::Run(Pipe &inputPipe, Pipe &outputPipe, Function &computeMe) {
    auto *args = new SumArgs();
    args->inputPipe = &inputPipe;
    args->outputPipe = &outputPipe;
    args->function = &computeMe;

    pthread_create(&thread, nullptr, SumExecute, (void *) args);
}

void GroupBy::Run(Pipe &inputPipe, Pipe &outputPipe, OrderMaker &groupAttrs, Function &computeMe) {

}

void WriteOut::Run(Pipe &inputPipe, FILE *outputFile, Schema &schema) {

}
