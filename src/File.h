#ifndef SQLIKE_FILE_H
#define SQLIKE_FILE_H

#include "Page.h"
#include "Record.h"

class File {
private:
    int fileDescriptor;

    // Length of the file, in terms if number of pages.
    off_t currLength;

public:
    File() = default;

    ~File() = default;

    off_t GetLength() const;

    /*
     * Opens and existing file or Creates a new file.
     * mode -> Specifies if new file needs to be created to an existing file needs to be opened.
     * mode equal 0 means create new file, otherwise simply open the file.
     */
    void Open(int mode, const char *fileName);

    // Closes the file and returns the file length.
    int Close();

    // Gets a specified page from a file and update the page param given to it.
    void GetPage(Page *page, off_t whichPage) const;

    /*
     * Writes a specified page to the file.
     * If the write is past the end of the file, all of the new pages which are before the page
     * to be written are emptied out.
     */
    void AddPage(Page *page, off_t whichPage);
};

#endif //SQLIKE_FILE_H
