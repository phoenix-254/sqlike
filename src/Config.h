#ifndef SQLIKE_CONFIG_H
#define SQLIKE_CONFIG_H

#include <map>
#include <string>

using namespace std;

/*
 * Path information relative to the root directory "sqlike"
 * from where the commands are executed.
 */

// Path to the catalog file.
const char *CATALOG_FILE_PATH = "files/catalog";

/*
 * This directory contains the *.tbl files generated through tpch.
 * Change to "files/1gb" when you need to work with 1gb tbl files.
 */
const char *TBL_FILES_PATH = "files/10mb/";

/*
 * This is the directory where you generated bin files should go to.
 * Change to "bin/1gb" when you need to work with 1gb bin files.
 */
const char *BIN_FILES_PATH = "bin/10mb/";

// Tables.
const char *TBL_REGION = "region";
const char *TBL_NATION = "nation";
const char *TBL_PART = "part";
const char *TBL_SUPPLIER = "supplier";
const char *TBL_PARTSUPP = "partsupp";
const char *TBL_CUSTOMER = "customer";
const char *TBL_ORDERS = "orders";
const char *TBL_LINEITEM = "lineitem";

/*
 * Mapping of table name with the index.
 * Helps up in displaying the tables in order and parsing the table name from the user input.
 * e.g Gives table name as "part" directly when the user selects 3 from options menu.
 */
const map<int, const char*> TABLE_MAP = {
        {1, TBL_REGION},
        {2, TBL_NATION},
        {3, TBL_PART},
        {4, TBL_SUPPLIER},
        {5, TBL_PARTSUPP},
        {6, TBL_CUSTOMER},
        {7, TBL_ORDERS},
        {8, TBL_LINEITEM}
    };

#endif //SQLIKE_CONFIG_H
