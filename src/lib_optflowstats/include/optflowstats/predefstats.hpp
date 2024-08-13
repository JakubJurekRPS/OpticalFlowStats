#ifndef OFS_PREDEFSTATS_H
#define OFS_PREDEFSTATS_H

#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

#include "statcoll/statistics_types.hpp"

namespace ofs
{

using std::string;
using std::unique_ptr;
using std::make_unique;
using sc::Histogram;
using sc::hsize_t;

#ifdef USE_GPU
    using Mat = cv::cuda::GpuMat;
#else 
    using Mat = cv::Mat;
#endif

class CheckHistParams
{
protected:
    hsize_t checkNumOfBins(const hsize_t numOfBins) const;
};

// Histogram of Angles
class HoA: public Histogram<unsigned>
{
public:
    using Histogram<unsigned>::Histogram;
};

class HoV: public Histogram<unsigned>
{
public:
    using Histogram<unsigned>::Histogram;
};

class MADiv: public sc::DescriptiveStat<float>
{
public:
    using DescriptiveStat<float>::DescriptiveStat;
};

class CalcHistConfig
{
protected:
    hsize_t mNumOfBins_;
public:
    CalcHistConfig() : mNumOfBins_{10} {};
    void config(hsize_t numOfBins) { mNumOfBins_ = numOfBins; }
};

class CalcHoA: public CalcHistConfig, public CheckHistParams
{
public:
    using CalcHistConfig::CalcHistConfig;
    HoA operator()(const Mat & src);
    static string getName() { return "HoA"; }
    static string getDescription() { return "Histogram of Angles"; }
};


class CalcHoV: protected CalcHistConfig, public CheckHistParams
{
public:
    using CalcHistConfig::CalcHistConfig;
    HoV operator()(const Mat & src);
    static string getName() { return "HoV"; }
    static string getDescription() { return "Histogram of Velocities"; }
};

class CalcMADiv
{
public:
    MADiv operator()(const Mat & src);
    static string getName() { return "MADiv"; }
    static string getDescription() { return "Mean absolute divergence"; }
};

} // namespace ofs
#endif // OFS_PREDEFSTATS_H