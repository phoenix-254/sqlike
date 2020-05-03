#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "File.h"

using namespace std;

off_t File::GetLength() const {
    return currLength;
}

void File::Open(int mode, const char *fileName) {
    int openMode;
    if (mode == 0) openMode = O_TRUNC | O_RDWR | O_CREAT;
    else openMode = O_RDWR;

    fileDescriptor = open(fileName, mode, S_IRUSR | S_IWUSR);
    if (fileDescriptor < 0) {
        cerr << "BAD!  Open did not work for " << fileName << endl;
        exit(1);
    }

    // Read the buffer if opening an existing file.
    if (mode != 0) {
        // Read in the first page
        lseek(fileDescriptor, 0, SEEK_SET);
        read(fileDescriptor, &currLength, sizeof(off_t));
    }
    else {
        currLength = 0;
    }
}

int File::Close() {
    // Write out the current length in pages
    lseek(fileDescriptor, 0, SEEK_SET);
    write(fileDescriptor, &currLength, sizeof(off_t));

    close(fileDescriptor);

    return GetLength();
}

void File::GetPage(Page *page, off_t whichPage) const {
    // Increment as the first page has no data.
    whichPage++;

    if (whichPage >= currLength) {
        cerr << "whichPage " << whichPage << " length " << currLength << endl;
        cerr << "BAD: you tried to read past the end of the file\n";
        exit(1);
    }

    // Read in the specified page
    char *bits = new (std::nothrow) char[PAGE_SIZE];
    if (bits == nullptr){
        cout << "ERROR : Not enough memory. EXIT !!!\n";
        exit(1);
    }

    lseek(fileDescriptor, PAGE_SIZE * whichPage, SEEK_SET);
    read(fileDescriptor, bits, PAGE_SIZE);
    page->FromBinary(bits);
    delete [] bits;
}

void File::AddPage(Page *page, off_t whichPage) {
    // Increment as the first page has no data.
    whichPage++;

    // If adding past the end of the file, then zero out add of the pages.
    if (whichPage >= currLength) {
        // Zeroing
        for (off_t index = currLength; index < whichPage; index++) {
            int foo = 0;
            lseek(fileDescriptor, PAGE_SIZE * index, SEEK_SET);
            write(fileDescriptor, &foo, sizeof(int));
        }

        // Update size
        currLength = whichPage + 1;
    }

    // now write the page
    char *bits = new (std::nothrow) char[PAGE_SIZE];
    if (bits == nullptr) {
        cout << "ERROR : Not enough memory. EXIT !!!\n";
        exit(1);
    }

    page->ToBinary(bits);
    lseek(fileDescriptor, PAGE_SIZE * whichPage, SEEK_SET);
    write(fileDescriptor, bits, PAGE_SIZE);
    delete [] bits;
}
