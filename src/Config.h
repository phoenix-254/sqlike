#ifndef SQLIKE_CONFIG_H
#define SQLIKE_CONFIG_H

#include <map>
#include <string>

using namespace std;

/*
 * All the paths are relative to the build directory from where our executable resides.
 * This is from where the commands are executed.
 */

// Path to the catalog file.
const char *CATALOG_FILE_PATH = "../files/catalog";

/*
 * This directory contains the *.tbl files generated through tpch.
 * Change to "files/1gb" when you need to work with 1gb tbl files.
 */
const char *TBL_FILES_PATH = "../files/1gb/";

/*
 * This is the directory where you generated bin files should go to.
 * Change to "bin/1gb" when you need to work with 1gb bin files.
 */
const char *BIN_FILES_PATH = "../bin/1gb/";

// Tables indexes and names.
const int TBL_REGION_INDEX = 1, TBL_NATION_INDEX = 2, TBL_PART_INDEX = 3,
        TBL_SUPPLIER_INDEX = 4, TBL_PARTSUPP_INDEX = 5, TBL_CUSTOMER_INDEX = 6,
        TBL_ORDERS_INDEX = 7, TBL_LINEITEM_INDEX = 8;

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
        {TBL_REGION_INDEX, TBL_REGION},
        {TBL_NATION_INDEX, TBL_NATION},
        {TBL_PART_INDEX, TBL_PART},
        {TBL_SUPPLIER_INDEX, TBL_SUPPLIER},
        {TBL_PARTSUPP_INDEX, TBL_PARTSUPP},
        {TBL_CUSTOMER_INDEX, TBL_CUSTOMER},
        {TBL_ORDERS_INDEX, TBL_ORDERS},
        {TBL_LINEITEM_INDEX, TBL_LINEITEM}
    };

#endif //SQLIKE_CONFIG_H
