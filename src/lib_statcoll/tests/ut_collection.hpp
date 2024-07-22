#include "gtest/gtest.h"
#include "statcoll/collection.hpp"


TEST(Collection, Emplace_back)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    coll.emplace_back(1);
    coll.emplace_back(2);
    coll.emplace_back(3);
    coll.emplace_back(4);
    coll.emplace_back(5);
    
    EXPECT_EQ(coll.size(), 5);
}

TEST(Collection, Clear)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    coll.emplace_back(1);
    coll.emplace_back(2);
    coll.emplace_back(3);
    coll.emplace_back(4);
    coll.emplace_back(5);
    coll.clear();
    
    EXPECT_EQ(coll.size(), 0);
}

TEST(Collection, GetName)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    EXPECT_EQ(coll.getName(), "TestName");
}

TEST(Collection, GetBatch)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    EXPECT_EQ(coll.getBatch(), "TestBatch");
}

TEST(Collection, GetDescription)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    EXPECT_EQ(coll.getDescription(), "TestDescription");
}

TEST(Collection, GetId)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    EXPECT_EQ(coll.getId(), 1);
}

TEST(Collection, GetElement)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    coll.emplace_back(1);
    coll.emplace_back(2);
    coll.emplace_back(3);
    coll.emplace_back(4);
    coll.emplace_back(5);
    
    EXPECT_EQ(coll[0], 1);
    EXPECT_EQ(coll[1], 2);
    EXPECT_EQ(coll[2], 3);
    EXPECT_EQ(coll[3], 4);
    EXPECT_EQ(coll[4], 5);
}

TEST(Collection, Move)
{
    sc::Collection<int> coll("TestName", "TestBatch", "TestDescription", 1);
    coll.emplace_back(1);
    coll.emplace_back(2);
    coll.emplace_back(3);
    coll.emplace_back(4);
    coll.emplace_back(5);
    
    sc::Collection<int> coll2 = std::move(coll);
    
    EXPECT_EQ(coll2.size(), 5);
    EXPECT_EQ(coll.size(), 0);
}




