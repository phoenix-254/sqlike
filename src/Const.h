#ifndef SQLIKE_CONST_H
#define SQLIKE_CONST_H

#define MAX_ANDS 20
#define MAX_ORS 20

#define PAGE_SIZE 131072

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