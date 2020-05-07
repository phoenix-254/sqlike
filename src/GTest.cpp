#include <gtest/gtest.h>

#include "DBFile.h"

TEST(DBFile, CreateNegative) {
    DBFile dbFile;
    ASSERT_EQ(dbFile.Create(nullptr, HEAP, nullptr), 0);
}

TEST(DBFile, CreatePositive) {
    DBFile dbFile;
    ASSERT_EQ(dbFile.Create("test.bin", HEAP, nullptr), 1);

    FILE* f = fopen("test.bin", "r");
    ASSERT_TRUE(f != nullptr);
}

TEST(DBFile, OpenNegative) {
    DBFile dbFile;
    ASSERT_EQ(dbFile.Open(nullptr), 0);
}

TEST(DBFile, OpenPositive) {
    DBFile dbFile;
    ASSERT_EQ(dbFile.Open("test.bin"), 1);

    FILE* f = fopen("test.bin", "r");
    ASSERT_TRUE(f != nullptr);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}