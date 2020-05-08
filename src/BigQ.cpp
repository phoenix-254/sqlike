#include "BigQ.h"

BigQ::BigQ(Pipe &input, Pipe &output, OrderMaker &sortOrder, int runLength) {
    // read data from in pipe sort them into runLength pages

    // construct priority queue over sorted runs and dump sorted data
    // into the out pipe

    // Finally, shut down the out pipe
    output.ShutDown();
}

BigQ::~BigQ() {

}
