#include <iostream>
#include <cstring>

#include "Record.h"

using namespace std;

Record::Record() {
    bits = nullptr;
}

Record::~Record() {
    Clear();
}

char * Record::GetBits() {
    return bits;
}

void Record::SetBits(char *src) {
    delete [] bits;
    bits = src;
}

void Record::CopyBits(char *src, int len) {
    delete [] bits;
    bits = new (std::nothrow) char[len];
    MemoryValidation(bits);
    memcpy(bits, src, len);
}

void Record::Clear() {
    delete [] bits;
    bits = nullptr;
}

void Record::MemoryValidation(const char *arr) {
    if (arr == nullptr) {
        cout << "ERROR : Not enough memory. EXIT !!!\n";
        exit(1);
    }
}

void Record::Consume(Record *consumeMe) {
    delete [] bits;
    bits = consumeMe->bits;
    consumeMe->bits = nullptr;
}

void Record::Copy(Record *copyMe) {
    delete [] bits;
    bits = new (std::nothrow) char[((int *) copyMe->bits)[0]];
    MemoryValidation(bits);
    memcpy(bits, copyMe->bits, ((int *) copyMe->bits)[0]);
}

int Record::SuckNextRecord(Schema &schema, FILE *file) {
    char *attrVal = new (std::nothrow) char[PAGE_SIZE];
    MemoryValidation(attrVal);

    char *rec = new (std::nothrow) char[PAGE_SIZE];
    MemoryValidation(rec);

    Clear();

    vector<Attribute> attrs = schema.GetAttributes();
    int size = attrs.size();

    // current position (int bytes) in the binary representation of the record that we are dealing with
    int currentPosInRec = sizeof(int) * (size + 1);

    // loop through all of the attributes
    for (int index = 0; index < size; index++) {
        int len = 0;

        // Read character by character, the record is pipe ('|') separated.
        // e.g "0|ALGERIA|0| haggle. carefully final deposits detect slyly agai|"
        while (true) {
            int nextChar = getc(file);
            if (nextChar == '|') break;
            else if (nextChar == EOF) {
                delete [] attrVal;
                delete [] rec;
                return 0;
            }

            attrVal[len] = (char) nextChar;
            len++;
        }

        ((int*) rec)[index + 1] = currentPosInRec;

        // Null terminate the string
        attrVal[len] = 0;
        len++;

        // Convert the data to correct binary representation
        if (attrs[index].type == Int) {
            *((int *) &(rec[currentPosInRec])) = atoi(attrVal);
            currentPosInRec += sizeof(int);
        }
        else if (attrs[index].type == Double) {
            // align things to the size of an integer if needed
            while (currentPosInRec % sizeof(double) != 0) {
                currentPosInRec += sizeof(int);
                ((int *) rec)[index + 1] = currentPosInRec;
            }

            *((double *) &(rec[currentPosInRec])) = atof(attrVal);
            currentPosInRec += sizeof(double);
        }
        else if (attrs[index].type == String) {
            // align things to the size of an integer if needed
            if (len % sizeof(int) != 0) {
                len += sizeof(int) - (len % sizeof(int));
            }

            strcpy(&(rec[currentPosInRec]), attrVal);
            currentPosInRec += len;
        }
    }

    // Set up the pointer to just past the end of the record
    ((int *) rec)[0] = currentPosInRec;

    // Copy over the bits
    bits = new (std::nothrow) char[currentPosInRec];
    MemoryValidation(bits);

    memcpy(bits, rec, currentPosInRec);

    delete [] attrVal;
    delete [] rec;

    return 1;
}

void Record::Project(const int *attrsToKeep, int numAttrsToKeep, int numAttrsNow) {
    // Figure out the required size for the new record.
    int requiredSize = sizeof(int) * (numAttrsToKeep + 1);
    for (int index = 0; index < numAttrsToKeep; index++) {
        // If we're keeping the last record, be careful!
        if (attrsToKeep[index] == numAttrsNow - 1) {
            // in this case, take the length of the record and subtract the start pos
            requiredSize += ((int *) bits)[0] - ((int *) bits)[attrsToKeep[index] + 1];
        } else {
            // in this case, subtract the start of the next field from the start of this field
            requiredSize += ((int *) bits)[attrsToKeep[index] + 2] - ((int *) bits)[attrsToKeep[index] + 1];
        }
    }

    char *projectionBits = new (std::nothrow) char[requiredSize];
    MemoryValidation(projectionBits);

    // Record the required size.
    *((int*) projectionBits) = requiredSize;

    // Copy over all of the fields.
    int currPosition = sizeof(int) * (numAttrsToKeep + 1);
    for (int index = 0; index < numAttrsToKeep; index++) {
        // Length of the current attribute (in bytes)
        int attrLen;

        // If we're keeping the last record, be careful!
        if (attrsToKeep[index] == numAttrsNow - 1) {
            // in this case, take the length of the record and subtract the start pos
            attrLen = ((int *) bits)[0] - ((int *) bits)[attrsToKeep[index] + 1];
        } else {
            // in this case, subtract the start of the next field from the start of this field
            attrLen = ((int *) bits)[attrsToKeep[index] + 2] - ((int *) bits)[attrsToKeep[index] + 1];
        }

        // set the start position of this field, and copy over the bits
        ((int *) projectionBits)[index + 1] = currPosition;
        memcpy(&(projectionBits[currPosition]),
                &(bits[((int *) bits)[attrsToKeep[index] + 1]]), attrLen);

        // note that we are moving along in the record
        currPosition += attrLen;
    }

    // Reset bits
    delete [] bits;
    bits = projectionBits;
}

void Record::MergeRecords(Record *left, Record *right, int numAttrsLeft, int numAttrsRight,
        const int *attrsToKeep, int numAttrsToKeep, int startOfRight) {
    Clear();

    // If one of the records is empty, new record is simply the other non-empty record.
    if (numAttrsLeft == 0) {
        Copy(right);
        return;
    } else if (numAttrsRight == 0) {
        Copy(left);
        return;
    }

    // Figure out the required size for the new record.
    int requiredSize = sizeof(int) * (numAttrsToKeep + 1);

    int numAttrsNow = numAttrsLeft;
    char *mergedBits = left->bits;

    for (int index = 0; index < numAttrsToKeep; index++) {
        if (index == startOfRight) {
            numAttrsNow = numAttrsRight;
            mergedBits = right->bits;
        }

        // If we're keeping the last record, be careful!
        if (attrsToKeep[index] == numAttrsNow - 1) {
            // in this case, take the length of the record and subtract the start pos
            requiredSize += ((int *) mergedBits)[0] - ((int *) mergedBits)[attrsToKeep[index] + 1];
        } else {
            // in this case, subtract the start of the next field from the start of this field
            requiredSize += ((int *) mergedBits)[attrsToKeep[index] + 2] - ((int *) mergedBits)[attrsToKeep[index] + 1];
        }
    }

    bits = new (std::nothrow) char[requiredSize + 1];
    MemoryValidation(bits);

    // Record the required size.
    *((int *) bits) = requiredSize;

    numAttrsNow = numAttrsLeft;
    mergedBits = left->bits;

    // Copy over all of the fields.
    int currPosition = sizeof(int) * (numAttrsToKeep + 1);
    for (int index = 0; index < numAttrsToKeep; index++) {
        if (index == startOfRight) {
            numAttrsNow = numAttrsRight;
            mergedBits = right->bits;
        }

        // Length of the current attribute (in bytes)
        int attrLen;

        // If we're keeping the last record, be careful!
        if (attrsToKeep[index] == numAttrsNow - 1) {
            // in this case, take the length of the record and subtract the start pos
            attrLen = ((int *) mergedBits)[0] - ((int *) mergedBits)[attrsToKeep[index] + 1];
        } else {
            // in this case, subtract the start of the next field from the start of this field
            attrLen = ((int *) mergedBits)[attrsToKeep[index] + 2] - ((int *) mergedBits)[attrsToKeep[index] + 1];
        }

        // set the start position of this field, and copy over the bits
        ((int *) bits)[index + 1] = currPosition;
        memcpy(&(bits[currPosition]),
               &(mergedBits[((int *) mergedBits)[attrsToKeep[index] + 1]]), attrLen);

        // note that we are moving along in the record
        currPosition += attrLen;
    }
}

void Record::Print(Schema &schema) {
    vector<Attribute> attrs = schema.GetAttributes();
    int index = 0;
    for  (auto & attr : attrs) {
        cout << attr.name << ": ";
        try {
            int ptr = ((int*) bits)[index + 1];
            cout << "[";
            if (attr.type == Int) {
                int *myInt = (int *) &(bits[ptr]);
                cout << *myInt;
            }
            else if (attr.type == Double) {
                auto *myDouble = (double *) &(bits[ptr]);
                cout << *myDouble;
            }
            else if (attr.type == String) {
                char *myString = (char *) &(bits[ptr]);
                cout << myString;
            }
            cout << "]";

            // Separate by comma if not the last value
            if (index != attrs.size() - 1) cout << ", ";
        } catch (char *exception) {
            cout << "Exception : " << exception << endl;
        }
        index++;
    }

    cout << endl;
}