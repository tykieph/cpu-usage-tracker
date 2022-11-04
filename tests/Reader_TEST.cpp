#include <gtest/gtest.h>

extern "C" 
{ 
    #include "Reader.h" 
}

#include <thread>

/********************************************************************************/
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
/********************************************************************************/
TEST_F(ReaderTest, Open)
{
    ASSERT_EQ(1, open_proc_stat());
    ASSERT_EQ(0, proc_stat_closed());
}
/********************************************************************************/
TEST_F(ReaderTest, Read)
{
    size_t rows;
    get_buffer(&data, &rows);

    EXPECT_NE(nullptr, data);
    EXPECT_LT(0, rows);

    size_t hwThreads = std::thread::hardware_concurrency();
    EXPECT_EQ(hwThreads + 1, rows);

    get_buffer_without_header(&data, &rows);
    EXPECT_EQ(hwThreads, rows);
}
/********************************************************************************/
TEST_F(ReaderTest, Close)
{
    destroy_reader();
    EXPECT_EQ(1, proc_stat_closed());
}
/********************************************************************************/
