#ifndef UTESTS_H
#define UTESTS_H

#include <gtest/gtest.h>
#include "optflowstats/predefstats.hpp"

// TODO: other tests
auto getTestFlow()
{
    cv::Mat flow(600, 600, CV_32FC2);
    for(int i = 0; i < flow.rows; i++)
    {
        for(int j = 0; j < flow.cols; j++)
        {
            flow.at<cv::Point2f>(i, j) = cv::Point2f(j, i);
        }
    }
#ifdef USE_GPU
    ofs::Mat testFlow;
    testFlow.upload(flow);
#else
    auto & testFlow = flow;
#endif
    return testFlow;
}

TEST(MADiv, sum)
{
    ofs::CalcMADiv calcMADiv;
    auto madiv = calcMADiv(getTestFlow());
    EXPECT_EQ(madiv.val, 2.0);
}

TEST(HoA, sum)
{
    ofs::CalcHoA calcHoA;
    auto testFlow = getTestFlow();
    auto hoa = calcHoA(testFlow);
    EXPECT_EQ(hoa.size(), 10);
    int sum = 0;
    for(auto & val : hoa)
    {
        sum += val;
    }
    EXPECT_EQ(sum,testFlow.rows * testFlow.cols);
}
#endif // UTESTS_H