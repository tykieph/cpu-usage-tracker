#include <gtest/gtest.h>

extern "C" 
{ 
    #include "SafeQueue.h" 
}

#include <thread>

/********************************************************************************/
class SafeQueueTest : public testing::TestWithParam<size_t>
{
public:
    virtual void SetUp()
    {     
        q = nullptr;
        q = queue_init();

        size_t thdNum = GetParam();
        threads.resize(thdNum);
    }

    virtual void TearDown()
    {
    }

protected:
    Queue *q;
    std::vector<pthread_t> threads;
};
/********************************************************************************/
TEST_P(SafeQueueTest, Push)
{
    EXPECT_EQ(0, q->size);
    EXPECT_EQ(1, queue_empty(q));

    for (auto &thd : threads)
    {
        pthread_create(&thd, NULL, [](void *arg)
        { 
            Queue *q = (Queue *)arg;
            char tmp[32] = "testing...testing..."; 

            for (size_t i = 0; i < 50; i++)
                queue_push(q, tmp, LOG_ENTRY_MAX);

            return (void *)NULL; 
        }, q);
    }

    for (const auto &thd : threads)
    {
        pthread_join(thd, NULL);
    }

    EXPECT_EQ(threads.size() * 50, q->size);
}
/********************************************************************************/
TEST_P(SafeQueueTest, Pop)
{
    EXPECT_EQ(0, q->size);
    EXPECT_EQ(1, queue_empty(q));

    for (auto &thd : threads)
    {
        pthread_create(&thd, NULL, [](void *arg)
        { 
            Queue *q = (Queue *)arg;
            char tmp[32] = "testing...testing..."; 

            for (size_t i = 0; i < 50; i++)
                queue_push(q, tmp, LOG_ENTRY_MAX);

            return (void *)NULL; 
        }, q);
    }

    for (const auto &thd : threads)
    { 
        pthread_join(thd, NULL);
    }

    char buff[LOG_ENTRY_MAX];
    for (size_t i = 0; i < 30; i++)
    {
        queue_wait_pop(q, buff, LOG_ENTRY_MAX);
    }

    EXPECT_EQ(threads.size() * 50 - 30, q->size);
    
    while (!queue_empty(q))
    {
        queue_timedwait_pop(q, buff, LOG_ENTRY_MAX, 1);
    }


    EXPECT_EQ(0, q->size);
    EXPECT_EQ(1, queue_empty(q));
}
/********************************************************************************/
INSTANTIATE_TEST_SUITE_P(PushPopTest, SafeQueueTest, testing::Range((size_t)1, (size_t)std::thread::hardware_concurrency()));
