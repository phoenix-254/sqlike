#ifndef SQLIKE_BIGQ_H
#define SQLIKE_BIGQ_H

#include "Comparison.h"
#include "Pipe.h"

class BigQ {
public:
    BigQ(Pipe &input, Pipe &output, OrderMaker &sortOrder, int runLength);
    ~BigQ();
};

#endif //SQLIKE_BIGQ_H
