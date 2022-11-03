#include <gtest/gtest.h>

extern "C" 
{ 
    #include "Reader.h"
    #include "Analyzer.h" 
}

/********************************************************************************/
class AnalyzerTest : public testing::Test
{
public:
    virtual void SetUp()
    {     
        open_proc_stat();
        get_buffer_without_header(&data, &rows); 
        usage = nullptr;       
    }

    virtual void TearDown()
    {
        destroy_analyzer();
        destroy_reader();
    }

protected:
    char **data;
    float *usage;
    size_t rows;
};
/********************************************************************************/
TEST_F(AnalyzerTest, ProcessData)
{
    process_data(&data, rows);
    
    EXPECT_EQ(nullptr, usage);    
    get_cpus_usage(&usage, rows);
    EXPECT_NE(nullptr, usage);
}
/********************************************************************************/