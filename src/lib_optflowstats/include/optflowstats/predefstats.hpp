#ifndef OFS_PREDEFSTATS_H
#define OFS_PREDEFSTATS_H

#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

#include "statcoll/base_classes.hpp"
#include "statcoll/statistics_types.hpp"

namespace ofs
{

using std::string;
using std::unique_ptr;
using std::make_unique;
using sc::Histogram;
using sc::FunctorBase;
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



class CalcHoA: public FunctorBase, public CheckHistParams
{
    hsize_t mNumOfBins_;
public:
    // CalcHoA(): CalcHoA(10) {};
    CalcHoA(hsize_t numOfBins);
    HoA operator()(const Mat & src);
    static string getName() { return "HoA"; }
    static string getDescription() { return "Histogram of Angles"; }
    static unique_ptr<FunctorBase> create(hsize_t numOfBins);
};


class CalcHoV: public FunctorBase, public CheckHistParams
{
    hsize_t mNumOfBins_;
public:
    // CalcHoV(): CalcHoV(10) {};
    CalcHoV(hsize_t numOfBins);
    HoV operator()(const Mat & src);
    static string getName() { return "HoV"; }
    static string getDescription() { return "Histogram of Velocities"; }
    static unique_ptr<FunctorBase> create(hsize_t numOfBins);
};

class CalcMADiv: public FunctorBase
{
public:
    MADiv operator()(const Mat & src);
    static string getName() { return "MADiv"; }
    static string getDescription() { return "Mean absolute divergence"; }
    static unique_ptr<FunctorBase> create() { return make_unique<CalcMADiv>(); }
};

} // namespace ofs
#endif // OFS_PREDEFSTATS_H