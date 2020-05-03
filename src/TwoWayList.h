#ifndef SQLIKE_TWOWAYLIST_H
#define SQLIKE_TWOWAYLIST_H

template <class Type>
class TwoWayList {
private:
    struct Node {
        // data
        Type *data, *next, *previous;

        // constructor
        Node() : data(0), next(0), previous(0) {}

        // destructor
        ~Node() {
            delete data;
        }
    };

    struct Header {
        // data
        Node *first, *last, *current;

        int leftSize, rightSize;
    };

    // the list itself is pointed to by this pointer
    Header *list;

public:
    TwoWayList();

    TwoWayList(TwoWayList &List);

    ~TwoWayList();

    // swap operator
    void operator &= (TwoWayList &List);

    // add to current pointer position
    void Insert(Type *Item);

    // remove from current pointer position
    void Remove(Type *Item);

    // get a reference to the current item, plus the offset given
    Type* Current(int offset);

    // move the current pointer position backward through the list
    void Retreat();

    // move the current pointer position forward through the list
    void Advance();

    // operations to check the size of both sides
    int LeftLength();
    int RightLength();

    // operations to swap the left and right sides of two lists
    void SwapLefts(TwoWayList &List);
    void SwapRights(TwoWayList &List);

    // operations to move the the start of end of a list
    void MoveToStart();
    void MoveToFinish();
};

#endif //SQLIKE_TWOWAYLIST_H
