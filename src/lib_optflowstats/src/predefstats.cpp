#include "optflowstats/predefstats.hpp"
#include "stats_definitions.hpp"

namespace ofs
{

hsize_t CheckHistParams::checkNumOfBins(const hsize_t numOfBins) const
{
    if (numOfBins <= 1)
        throw std::invalid_argument("Number of bins is too small : "
            + std::to_string(numOfBins) + string{" (or >= "} 
            + std::to_string(std::numeric_limits<hsize_t>::max()) 
            + string{", so that it's overflowed)"}); 
    return numOfBins;
}

CalcHoA::CalcHoA(hsize_t numOfBins): mNumOfBins_{checkNumOfBins(numOfBins)} {}

HoA CalcHoA::operator()(const Mat & src)
{
    return ofs_detail::calc_hist_of_angles(src, mNumOfBins_);
}

unique_ptr<FunctorBase> CalcHoA::create(hsize_t numOfBins)
{
    return make_unique<CalcHoA>(numOfBins);
}

// TODO: Implement CalcHoV operator()
CalcHoV::CalcHoV(hsize_t numOfBins): mNumOfBins_{checkNumOfBins(numOfBins)} {}
HoV CalcHoV::operator()(const Mat & src)
{
    return HoV(mNumOfBins_);
}

unique_ptr<FunctorBase> CalcHoV::create(hsize_t numOfBins)
{
    return make_unique<CalcHoV>(numOfBins);
}

MADiv CalcMADiv::operator()(const Mat & src)
{
    return ofs_detail::calc_mean_abs_divergence(src);
}

} // namespace ofs
