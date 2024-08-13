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

HoA CalcHoA::operator()(const Mat & src)
{
    return ofs_detail::calc_hist_of_angles(src, mNumOfBins_);
}

// TODO: Implement CalcHoV operator()
HoV CalcHoV::operator()(const Mat & src)
{
    return HoV(mNumOfBins_);
}

MADiv CalcMADiv::operator()(const Mat & src)
{
    return ofs_detail::calc_mean_abs_divergence(src);
}

} // namespace ofs
