#ifndef OFS_BASICTYPES_HPP
#define OFS_BASICTYPES_HPP

// #include "rawbuf.hpp"
#include <limits>
#include <stdexcept>

namespace sc
{

using std::invalid_argument;
using hsize_t = unsigned short;

template <typename T = unsigned>
class Histogram
{
    hsize_t checkSize(const hsize_t size) const
    {
        if(size > std::numeric_limits<hsize_t>::max())
            throw std::invalid_argument("Histogram size is too large.");
        else if (size <= 1)
            throw std::invalid_argument("Histogram size is too small. -> " 
                + std::to_string(size) + " <= 1"); 
        return size;
    }
public:
    Histogram(const hsize_t binsNum): mBinsNum_{checkSize(binsNum)}, mData_{new T[binsNum]} 
    {
        std::fill(mData_, mData_ + binsNum, 0);
    }
    
    Histogram(const Histogram & other) : mBinsNum_{other.mBinsNum_}, mData_{new T[mBinsNum_]}
    {
        std::copy(other.mData_, other.mData_ + mBinsNum_, mData_);
    }

    Histogram & operator=(const Histogram & other)
    {
        if(this == &other)
            return *this;
        delete[] mData_;
        mBinsNum_ = other.mBinsNum_;
        mData_ = new T[mBinsNum_];
        std::copy(other.mData_, other.mData_ + mBinsNum_, mData_);
        return *this;
    }

    Histogram(Histogram && other) noexcept 
    {
        mBinsNum_ = other.mBinsNum_;
        mData_ = other.mData_;
        other.mData_ = nullptr;
    }

    Histogram & operator=(Histogram && other) noexcept
    {
        if(this == &other)
            return *this;
        delete[] mData_;
        mBinsNum_ = other.mBinsNum_;
        mData_ = other.mData_;
        other.mData_ = nullptr;
        return *this;
    }
    auto begin() const 
    { 
        return mData_; 
    }
    auto end() const 
    { 
        return mData_ + mBinsNum_; 
    }
    auto & operator[](const hsize_t index) 
    { 
        return mData_[index]; 
    } 
    auto & operator[] (const hsize_t index) const 
    { 
        return mData_[index]; 
    }
    auto size() const 
    { 
        return mBinsNum_; 
    }
    auto & at(const hsize_t index) const 
    {
        if(index >= mBinsNum_)
            throw std::out_of_range("Histogram index out of range.");
        return mData_[index]; 
    }
    ~Histogram() { delete[] mData_; }
private:
    hsize_t mBinsNum_;
    T* mData_;
};

template <typename T = double>
class DescriptiveStat
{
public:
    DescriptiveStat(const T val): val{val} {}
    T val;
};

} // namespace sc

#endif // OFS_BASICTYPES_HPP
