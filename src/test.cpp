#include "BigQ.h"
#include "RelationalOp.h"
#include "test.h"

Attribute IA = {"int", Int};
Attribute SA = {"string", String};
Attribute DA = {"double", Double};

int ClearPipe(Pipe &pipe, Schema &schema, bool shouldPrint) {
    int count = 0;

    Record rec;
    while (pipe.Remove(&rec)) {
        if (shouldPrint) rec.Print(schema);
        count++;
    }

    return count;
}

// buffer size allowed for each pipe.
int pipeSize = 100;

// Pages of memory allowed for operations.
int buffSize = 100;

int numAttrsPart = 9, numAttrsSupp = 7, numAttrsPartSupp = 5;

SelectFile selFilePart, selFileSupp, selFilePartSupp;

DBFile dbFilePart, dbFileSupp, dbFilePartSupp;

Pipe pipePart(pipeSize), pipeSupp(pipeSize), pipePartSupp(pipeSize);

CNF cnfPart, cnfSupp, cnfPartSupp;

Record litPart, litSupp, litPartSupp;

void initSelFilePart(char *cnf, int numPages) {
    dbFilePart.Open(relations.find(TBL_PART_INDEX)->second->GetBinFilePath().c_str());
    GetCnf(cnf, *(relations.find(TBL_PART_INDEX)->second->GetSchema()), cnfPart, litPart);
    selFilePart.Use_n_Pages(numPages);
}

void initSelFileSupplier(char *cnf, int numPages) {
    dbFileSupp.Open(relations.find(TBL_SUPPLIER_INDEX)->second->GetBinFilePath().c_str());
    GetCnf(cnf, *(relations.find(TBL_SUPPLIER_INDEX)->second->GetSchema()), cnfSupp, litSupp);
    selFileSupp.Use_n_Pages(numPages);
}

void initSelFilePartSupp(char *cnf, int numPages) {
    dbFilePartSupp.Open(relations.find(TBL_PARTSUPP_INDEX)->second->GetBinFilePath().c_str());
    GetCnf(cnf, *(relations.find(TBL_PARTSUPP_INDEX)->second->GetSchema()), cnfPartSupp, litPartSupp);
    selFilePartSupp.Use_n_Pages(numPages);
}

// select * from partsupp where ps_supplycost < 1.03
// expected output: 31 records
void Query1() {
    cout << ">>>> Running Query 1" << endl;
    char *cnfInput = "(ps_supplycost < 1.03)";
    initSelFilePartSupp(cnfInput, 100);

    selFilePartSupp.Run(dbFilePartSupp, pipePartSupp, cnfPartSupp, litPartSupp);
    selFilePartSupp.WaitUntilDone();

    int numRecs = ClearPipe(pipePartSupp, *(relations.find(5)->second->GetSchema()), true);
    cout << endl << ">>>> Query 1 returned " << numRecs << " records." << endl;

    dbFilePartSupp.Close();
}

// select p_partkey(0), p_name(1), p_retailprice(7) from part where (p_retailprice > 931.01) AND (p_retailprice < 931.3);
// expected output: 22 records
void Query2() {
    cout << ">>>> Running Query 2" << endl;
    char *cnfInput = "(p_retailprice > 931.01) AND (p_retailprice < 931.3)";
    initSelFilePart(cnfInput, 100);

    Project projPart;
    Pipe out(pipeSize);
    int keepMe[] = {0, 1, 7};
    int numAttrsIn = numAttrsPart, numAttrsOut = 3;
    projPart.Use_n_Pages(buffSize);

    selFilePart.Run(dbFilePart, pipePart, cnfPart, litPart);
    projPart.Run(pipePart, out, keepMe, numAttrsIn, numAttrsOut);

    selFilePart.WaitUntilDone();
    projPart.WaitUntilDone();

    Attribute att3[] = {IA, SA, DA};
    Schema outSchema("outSchema", numAttrsOut, att3);

    int numRecs = ClearPipe(out, outSchema, true);
    cout << endl << ">>>> Query 2 returned " << numRecs << " records." << endl;

    dbFilePart.Close();
}

// select sum (s_acctbal + (s_acctbal * 1.05)) from supplier;
// expected output: 9.24623e+07
void Query3() {
    cout << ">>>> Running Query 3" << endl;
    char *cnfInput = "(s_suppkey = s_suppkey)";
    initSelFileSupplier(cnfInput, 100);

    Sum sum;
    Pipe out(1);
    Function func;
    char *sumInput = "(s_acctbal + (s_acctbal * 1.05))";
    GetCnf(sumInput, *(relations.find(TBL_SUPPLIER_INDEX)->second->GetSchema()), func);
    sum.Use_n_Pages(1);

    selFileSupp.Run(dbFileSupp, pipeSupp, cnfSupp, litSupp);
    sum.Run(pipeSupp, out, func);

    selFileSupp.WaitUntilDone();
    sum.WaitUntilDone();

    Schema outSchema("outSchema", 1, &DA);
    int numRecs = ClearPipe(out, outSchema, true);
    cout << endl << ">>>> Query 3 returned " << numRecs << " records." << endl;

    dbFileSupp.Close();
}

// select sum (ps_supplycost) from supplier, partsupp
// where s_suppkey = ps_suppkey;
// expected output: 4.00406e+08
void Query4() {
    cout << ">>>> Running Query 4" << endl;
    char *cnfInput = "(s_suppkey = s_suppkey)";
    initSelFileSupplier(cnfInput, 100);
    selFileSupp.Run(dbFileSupp, pipeSupp, cnfSupp, litSupp); // 10k recs qualified.

    char *cnfInput2 = "(ps_suppkey = ps_suppkey)";
    initSelFilePartSupp(cnfInput2, 100);

    Join join;
    Pipe pipe_S_PS(pipeSize);

    char *cnfInput3 = "(s_suppkey = ps_suppkey)";
    CNF cnf_S_PS;
    Record lit_S_PS;
    GetCnf(cnfInput3, *(relations.find(TBL_SUPPLIER_INDEX)->second->GetSchema()),
            *(relations.find(TBL_PARTSUPP_INDEX)->second->GetSchema()), cnf_S_PS, lit_S_PS);

    int numAttrsOut = numAttrsSupp + numAttrsPartSupp;
    Attribute ps_supplycost = {"ps_supplycost", Double};
    Attribute joinAttr[] = {IA, SA, SA, IA, SA, DA, SA, IA, IA, IA, ps_supplycost, SA};
    Schema joinSchema("joinSchema", numAttrsOut, joinAttr);

    Sum sum;
    Pipe out(1);
    Function func;
    char *cnfInput4 = "(ps_supplycost)";
    GetCnf(cnfInput4, joinSchema, func);
    sum.Use_n_Pages(1);

    selFilePartSupp.Run(dbFilePartSupp, pipePartSupp, cnfPartSupp, litPartSupp); // 161 recs qualified
    join.Run(pipeSupp, pipePartSupp, pipe_S_PS, cnf_S_PS, lit_S_PS);
    sum.Run(pipe_S_PS, out, func);

    selFilePartSupp.WaitUntilDone();
    join.WaitUntilDone();
    sum.WaitUntilDone();

    Schema outSchema("outSchema", 1, &DA);
    int numRecs = ClearPipe(out, outSchema, true);
    cout << endl << ">>>> Query 4 returned " << numRecs << " records." << endl;

    dbFileSupp.Close();
    dbFilePartSupp.Close();
}

// select distinct ps_suppkey from partsupp where ps_supplycost < 100.11;
// expected output: 9996 rows
void Query5() {
    cout << ">>>> Running Query 5" << endl;
    char *cnfInput = "(ps_supplycost < 100.11)";
    initSelFilePartSupp(cnfInput, 100);

    Project projPartSupp;
    Pipe projOutPipe(pipeSize);
    int keepMe[] = {1};
    int numAttrsIn = numAttrsPartSupp;
    int numAttrsOut = 1;
    projPartSupp.Use_n_Pages(buffSize);

    DuplicateRemoval dupRemoval;
    Pipe dupRemovalOutPipe(pipeSize);
    Schema dupRemovalSchema("dupRemovalSchema", 1, &IA);

    WriteOut writeOut;
    char *filePath = "ps.w.tmp";
    FILE *writeFile = fopen(filePath, "w");

    selFilePartSupp.Run(dbFilePartSupp, pipePartSupp, cnfPartSupp, litPartSupp);
    projPartSupp.Run(pipePartSupp, projOutPipe, keepMe, numAttrsIn, numAttrsOut);
    dupRemoval.Run(projOutPipe, dupRemovalOutPipe, dupRemovalSchema);
    writeOut.Run(dupRemovalOutPipe, writeFile, dupRemovalSchema);

    selFilePartSupp.WaitUntilDone();
    projPartSupp.WaitUntilDone();
    dupRemoval.WaitUntilDone();
    writeOut.WaitUntilDone();

    cout << ">>>> Query 5 finished. Output has been written to " << filePath << "!" << endl;

    dbFilePartSupp.Close();
}

// select sum (ps_supplycost) from supplier, partsupp
// where s_suppkey = ps_suppkey groupby s_nationkey;
// expected output: 25 rows
void Query6() {
    cout << ">>>> Running Query 6" << endl;
    char *cnfInput = "(s_suppkey = s_suppkey)";
    initSelFileSupplier(cnfInput, 100);
    selFileSupp.Run(dbFileSupp, pipeSupp, cnfSupp, litSupp); // 10k recs qualified.

    char *cnfInput2 = "(ps_suppkey = ps_suppkey)";
    initSelFilePartSupp(cnfInput2, 100);

    Join join;
    Pipe pipe_S_PS(pipeSize);

    char *cnfInput3 = "(s_suppkey = ps_suppkey)";
    CNF cnf_S_PS;
    Record lit_S_PS;
    GetCnf(cnfInput3, *(relations.find(TBL_SUPPLIER_INDEX)->second->GetSchema()),
           *(relations.find(TBL_PARTSUPP_INDEX)->second->GetSchema()), cnf_S_PS, lit_S_PS);

    int numAttrsOut = numAttrsSupp + numAttrsPartSupp;
    Attribute s_nationkey = {"s_nationkey", Int};
    Attribute ps_supplycost = {"ps_supplycost", Double};
    Attribute joinAttr[] = {IA, SA, SA, s_nationkey, SA, DA, SA, IA, IA, IA, ps_supplycost, SA};
    Schema joinSchema("joinSchema", numAttrsOut, joinAttr);

    GroupBy groupBy;
    Pipe out(1);
    Function func;
    char *cnfInput4 = "(ps_supplycost)";
    GetCnf(cnfInput4, joinSchema, func);
    OrderMaker groupByOrder(joinSchema);
    groupBy.Use_n_Pages(1);

    selFilePartSupp.Run(dbFilePartSupp, pipePartSupp, cnfPartSupp, litPartSupp); // 161 recs qualified
    join.Run(pipeSupp, pipePartSupp, pipe_S_PS, cnf_S_PS, lit_S_PS);
    groupBy.Run(pipe_S_PS, out, groupByOrder, func);

    selFilePartSupp.WaitUntilDone();
    join.WaitUntilDone();
    groupBy.WaitUntilDone();

    Schema outSchema("outSchema", 1, &DA);
    int count = ClearPipe(out, outSchema, true);
    cout << endl << ">>>> Query 6 returned sum for " << count << " groups (expected 25 groups)." << endl;

    dbFileSupp.Close();
    dbFilePartSupp.Close();
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Error: Please provide an argument. Example usage: \"./sqlike-test 1\" where 1 means run query 1." << endl;
        exit(0);
    }

    // Making array of all query functions above and calling appropriate function based on
    // what user has selected.
    void (*query) ();
    void (*queryPtr[]) () = {&Query1, &Query2, &Query3, &Query4, &Query5, &Query6};

    int selectedQueryIndex = atoi(argv[1]);
    if (selectedQueryIndex > 0 && selectedQueryIndex < 7) {
        Setup();

        query = queryPtr[selectedQueryIndex - 1];
        query();

        Cleanup();

        cout << endl << endl;
    }
    else {
        cout << "Error: Please select valid query number between 1 to 6." << endl;
    }
}