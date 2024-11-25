#include "gtest/gtest.h"
#include "statcoll/statistics_types.hpp"

TEST(Histogram, basic_constructor)
{
    sc::Histogram<int> hist(10);
    EXPECT_EQ(10, hist.size());
}

TEST(Histogram, checkSize)
{
    EXPECT_THROW(sc::Histogram<int>(0), std::invalid_argument);
    EXPECT_THROW(sc::Histogram<int>(1), std::invalid_argument);
    EXPECT_NO_THROW(sc::Histogram<int>(2));
    EXPECT_NO_THROW(sc::Histogram<int>(std::numeric_limits<sc::hsize_t>::max()));
}

TEST(Histogram, subscript_operator)
{
    sc::Histogram<int> hist(10);

    int i = 0;
    for(auto & elem : hist)
    {
        elem = i++;
    }
    i = 0;
    for(const auto & elem : hist)
    {
        EXPECT_EQ(i++, elem);
    }
}
// Check  precisely if the meaning of copy operation is fulfilled
TEST(Histogram, copy_constructor)
{
    sc::Histogram<int> hist(10);
    {
        int i = 0;
        for(auto & elem : hist)
        {
            elem = i++;
        }
    }
    sc::Histogram<int> hist2(hist);

    EXPECT_NE(hist.begin(), hist2.begin());
    EXPECT_NE(hist.begin(), nullptr);
    EXPECT_NE(hist2.begin(), nullptr);
    EXPECT_EQ(hist.size(), hist2.size());

    for(auto ith1 = hist.begin(), ith2 = hist2.begin(); ith1 != hist.end(); ++ith1, ++ith2)
    {
        EXPECT_EQ(*ith1, *ith2);
        if (ith2 == hist2.end())
        {
            FAIL() << "hist2 end reached before hist end";
        }
        
    }
}

// Check  precisely if the meaning of move operation is fulfilled
TEST(Histogram, move_constructor)
{
    sc::Histogram<int> hist(10);
    {
        int i = 0;
        for(auto & elem : hist)
        {
            elem = i++;
        }
    }

    auto h1begin = hist.begin();
    auto h1end = hist.end();
    auto h1size = hist.size();

    sc::Histogram<int> hist2(std::move(hist));

    EXPECT_EQ(nullptr, hist.begin());
    EXPECT_EQ(0, hist.size());
    EXPECT_EQ(h1begin, hist2.begin());
    EXPECT_EQ(h1end, hist2.end());
    EXPECT_EQ(h1size, hist2.size());

    int i = 0;
    for(const auto & elem : hist2)
    {
        EXPECT_EQ(i++, elem);
    }
}

TEST(Histogram, copy_assignment_operator)
{
    sc::Histogram<int> hist(10);
    sc::Histogram<int> hist2(5);
    {
        int i = 0;
        for(auto & elem : hist)
        {
            elem = i++;
        }

        for(auto & elem : hist2)
        {
            elem = i++;
        }
    }

    hist2 = hist;

    EXPECT_NE(hist.begin(), hist2.begin());
    EXPECT_NE(hist.begin(), nullptr);
    EXPECT_NE(hist2.begin(), nullptr);
    EXPECT_EQ(hist.size(), hist2.size());

    for(auto ith1 = hist.begin(), ith2 = hist2.begin(); ith1 != hist.end(); ++ith1, ++ith2)
    {
        EXPECT_EQ(*ith1, *ith2);
        if (ith2 == hist2.end())
        {
            FAIL() << "hist2 end reached before hist end";
        }
        
    }
}

TEST(Histogram, move_assignment_operator)
{
    sc::Histogram<int> hist(10);
    sc::Histogram<int> hist2(5);
    {
        int i = 0;
        for(auto & elem : hist)
        {
            elem = i++;
        }

        for(auto & elem : hist2)
        {
            elem = i++;
        }
    }

    auto h1begin = hist.begin();
    auto h1end = hist.end();
    auto h1size = hist.size();

    hist2 = std::move(hist);

    EXPECT_EQ(nullptr, hist.begin());
    EXPECT_EQ(0, hist.size());
    EXPECT_EQ(h1begin, hist2.begin());
    EXPECT_EQ(h1end, hist2.end());
    EXPECT_EQ(h1size, hist2.size());

    int i = 0;
    for(const auto & elem : hist2)
    {
        EXPECT_EQ(i++, elem);
    }
}