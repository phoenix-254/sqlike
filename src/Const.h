#ifndef SQLIKE_CONST_H
#define SQLIKE_CONST_H

#define MAX_ANDS 20
#define MAX_ORS 20

#define PAGE_SIZE 131072

#define CATALOG_PATH "../../files/catalog"
#define TBL_FILES_PATH "../../files/10mb/"
#define BIN_FILES_PATH "../../bin/10mb/"

#define TBL_REGION "region"
#define TBL_NATION "nation"
#define TBL_PART "part"
#define TBL_SUPPLIER "supplier"
#define TBL_PARTSUPP "partsupp"
#define TBL_CUSTOMER "customer"
#define TBL_ORDERS "orders"
#define TBL_LINEITEM "lineitem"

enum Target {
    Left,
    Right,
    Literal
};

enum CompOperator {
    LessThan,
    GreaterThan,
    Equals
};

enum Type {
    Int,
    Double,
    String
};

unsigned int Random_Generate();

#endif //SQLIKE_CONST_H