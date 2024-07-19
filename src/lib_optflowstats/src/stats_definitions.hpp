#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include "optflowstats/predefstats.hpp"

namespace ofs_detail
{
    using ofs::HoA;
    using ofs::MADiv;
    using ofs::Mat;

    HoA calc_hist_of_angles(const Mat & flow, const unsigned binsNum);
    MADiv calc_mean_abs_divergence(const Mat & flow);
} // namespace ofs_detail

#endif // STATISTICS_HPP