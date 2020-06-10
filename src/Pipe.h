#ifndef SQLIKE_PIPE_H
#define SQLIKE_PIPE_H

#include <pthread.h>

#include "Record.h"

class Pipe {
private:
    // Used for storing data in the pipeline.
    Record *buffered;

    int firstSlot, lastSlot, totalSpace;

    bool isDone;

    // mutex for the pipe
    pthread_mutex_t pipeMutex;

    // condition variables that the producer and consumer wait on
    pthread_cond_t producerVar;
    pthread_cond_t consumerVar;

public:
    Pipe() = default;

    /*
     * Sets up the pipeline
     * bufferSize = number of records to buffer
     */
    Pipe(int bufferSize);

    ~Pipe();

    /*
     * Inserts a new record into the pipeline. This may block if the buffer size is exceeded.
     * Note: The parameter passed is consumed by this method, and is of no use after this method.
     */
    void Insert(Record *rec);

    /*
     * Removes a record from the pipeline and puts it into the argument. This may block if there are
     * no records in the pipeline to be removed. Returns 1 on success, and 0 if there are no more
     * records in the pipeline.
     * Note: Whatever the previous value of the parameter, it will be lost.
     */
    int Remove(Record *rec);

    /*
     * Shut down the pipeline.
     * Used by the consumer to indicate that there's no more data left to be added into the pipe.
     */
    void ShutDown();
};

#endif //SQLIKE_PIPE_H
