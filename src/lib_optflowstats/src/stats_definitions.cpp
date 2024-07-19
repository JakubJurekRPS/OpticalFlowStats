#include "stats_definitions.hpp"
#include <math.h>

#include <iostream>
#include <chrono>
namespace ofs_detail
{
    using cv::Point2f;
    using std::floor;

namespace
{
    inline float calc_angle(const float x, const float y)
    {
        float angle = atan2(y, x);
        if (angle < 0)
            angle += 2 * CV_PI;
        return angle;
    }

    auto calc_divergence(const Mat& flow)
    {
        Mat divMat(flow.rows-1, flow.cols-1, CV_32F);
            
        for(int i=0; i < flow.rows-1; i++) //rows - y; cols - x
        {
            for(int j=0; j < flow.cols-1; j++)
            {
                const Point2f& currentVect = flow.at<Point2f>(i, j);
                float xAtCurrent = currentVect.x;
                float yAtCurrent = currentVect.y;
                // asumming that x-axis points to increasing row number and y-axis points to increasing column number
                float xAtNext = flow.at<Point2f>(i, j+1).x;
                float yAtNext = flow.at<Point2f>(i+1, j).y;
                divMat.at<float>(i, j) = (xAtNext - xAtCurrent) + (yAtNext - yAtCurrent);
            }
        }

        return divMat;
    }

    auto calc_mean_square(Mat& inp)
    {
        float meanSquare = 0;
        for(int i = 0; i < inp.rows; i++)
        {
            for(int j = 0; j < inp.cols; j++)
            {
                meanSquare += pow((double)inp.at<float>(i,j),2) / (inp.rows * inp.cols);
            } 
        }
        return meanSquare;
    }
} // namespace

    HoA calc_hist_of_angles(const Mat & flow,const unsigned binsNum)
    {
        HoA hist(binsNum);
        const double dAngle = 2 * CV_PI/binsNum;
        float angle;

        for(int i=0; i<flow.rows; i++)
        {
            for(int j=0; j<flow.cols; j++)
            {
                const Point2f& fxy = flow.at<Point2f>(i, j);
                angle = calc_angle(fxy.x, fxy.y);
                hist[static_cast<unsigned int>(floor(angle/dAngle))] += 1;
            }
        }
        return hist;
    }

    MADiv calc_mean_abs_divergence(const Mat & flow)
    {
        auto start = std::chrono::high_resolution_clock::now(); 
        float div = 0.0;
        for(int i=0; i < flow.rows-1; i++) //rows - y; cols - x
        {  
            for(int j=0; j < flow.cols-1; j++)
            {
                const Point2f& currentVect = flow.at<Point2f>(i, j);
                float xAtCurrent = currentVect.x;
                float yAtCurrent = currentVect.y;
                // asumming that x-axis points to increasing row number and y-axis points to increasing column number
                float xAtNext = flow.at<Point2f>(i, j+1).x;
                float yAtNext = flow.at<Point2f>(i+1, j).y;
                div += abs((xAtNext - xAtCurrent) + (yAtNext - yAtCurrent));
            }
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Time taken by function: "
                << duration.count() << " microseconds" << std::endl;
        return MADiv(div/ ((flow.rows-1) * (flow.cols-1)));
    }
} // namespace ofs_detail