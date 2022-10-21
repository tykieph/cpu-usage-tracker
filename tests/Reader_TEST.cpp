#include <gtest/gtest.h>

extern "C" 
{ 
    #include "Reader.h" 
}

class ReaderTest : public testing::Test
{
public:
    virtual void SetUp()
    {     
    }

    virtual void TearDown()
    {
    }

protected:
    char **data;
};

TEST_F(ReaderTest, Open)
{
    ASSERT_EQ(0, open_proc_stat());
}

TEST_F(ReaderTest, Read)
{
    size_t rows;
    get_buffer(&data, &rows);

    EXPECT_NE(nullptr, data);
    EXPECT_LT(0, rows);
}

TEST_F(ReaderTest, Close)
{
    destroy_reader();
    EXPECT_EQ(1, proc_stat_closed());
}