#include <iostream>

#include "Pipe.h"

Pipe::Pipe(int bufferSize) {
    // Set up the mutex associated with the pipe
    pthread_mutex_init(&pipeMutex, nullptr);

    // Set up the condition variables associated with the pipe
    pthread_cond_init(&producerVar, nullptr);
    pthread_cond_init(&consumerVar, nullptr);

    // Set up the pipe's buffer
    buffered = new (std::nothrow) Record[bufferSize];
    if (buffered == nullptr) {
        cout << "ERROR : Not enough memory. EXIT !!!\n";
        exit(1);
    }

    firstSlot = lastSlot = 0;
    totalSpace = bufferSize;

    // Note: the pipe has not yet been turned off
    isDone = false;
}

Pipe::~Pipe() {
    delete [] buffered;

    pthread_mutex_destroy(&pipeMutex);
    pthread_cond_destroy(&producerVar);
    pthread_cond_destroy(&consumerVar);
}

void Pipe::Insert(Record *rec) {
    // First, get a mutex on the pipeline
    pthread_mutex_lock(&pipeMutex);

    // Next, check if there's enough space in the pipe for more data; if there is, then perform the insertion,
    // and if not wait until the consumer frees up some space in the pipeline.
    if (lastSlot - firstSlot < totalSpace) {
        buffered[lastSlot % totalSpace].Consume(rec);
    }
    else {
        pthread_cond_wait(&producerVar, &pipeMutex);
        buffered[lastSlot % totalSpace].Consume(rec);
    }

    // Note that we have added a new record
    lastSlot++;

    // Signal the consumer who might now want to suck up the new record that has been added to the pipeline.
    pthread_cond_signal(&consumerVar);

    // Done!
    pthread_mutex_unlock(&pipeMutex);
}

int Pipe::Remove(Record *rec) {
    // First, get a mutex on the pipeline
    pthread_mutex_lock(&pipeMutex);

    // Next, check if there is anything in the pipeline; if there is, then do the removal, and if not, then
    // we need to wait until the producer puts some data into the pipeline.
    if (lastSlot != firstSlot) {
        rec->Consume(&buffered[firstSlot % totalSpace]);
    }
    else {
        // The pipeline is empty so we first see if this is because it was turned off
        if (isDone) {
            pthread_mutex_unlock(&pipeMutex);
            return 0;
        }

        // Wait until there is something there
        pthread_cond_wait(&consumerVar, &pipeMutex);

        // Since the producer may have decided to turn off the pipe, we need to check if it is still open
        if (isDone && lastSlot == firstSlot) {
            pthread_mutex_unlock(&pipeMutex);
            return 0;
        }

        rec->Consume(&buffered[firstSlot % totalSpace]);
    }

    // Note that we have deleted a record
    firstSlot++;

    // Signal the producer who might now want to take the slot that has been freed up by the deletion.
    pthread_cond_signal(&producerVar);

    // Done!
    pthread_mutex_unlock(&pipeMutex);

    return 1;
}

void Pipe::ShutDown() {
    // First, get a mutex on the pipeline
    pthread_mutex_lock(&pipeMutex);

    // Note that we are now done with the pipeline
    isDone = true;

    // Signal the consumer who may be waiting
    pthread_cond_signal(&consumerVar);

    // Unlock the mutex
    pthread_mutex_unlock(&pipeMutex);
}
