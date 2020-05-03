#include <iostream>

#include "Const.h"
#include "Record.h"
#include "Schema.h"

using namespace std;

int main() {
    Schema schema(CATALOG_PATH, TBL_NATION);
    schema.Print();

    FILE *tblFile = fopen("../../files/10mb/nation.tbl", "r");

    Record temp;
    while (temp.SuckNextRecord(schema, tblFile)) {
        temp.Print(schema);
    }

    return 0;
}