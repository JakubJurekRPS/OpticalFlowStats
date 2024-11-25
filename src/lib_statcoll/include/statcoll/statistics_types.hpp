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
    hsize_t checkSize(const int size) const
    {
        if (size <= 1)
        {
            throw std::invalid_argument("Histogram size is too small. -> " 
                + std::to_string(size) + " <= 1"); 
        }
        return size;
    }
public:
    explicit Histogram(const hsize_t binsNum): mBinsNum_{checkSize(binsNum)}, mData_{new T[binsNum]} 
    {
        std::fill(mData_, mData_ + binsNum, 0);
    }
    
    //copy constructor
    Histogram(const Histogram & other) : mBinsNum_{other.mBinsNum_}, mData_{new T[mBinsNum_]}
    {
        std::copy(other.mData_, other.mData_ + mBinsNum_, mData_);
        std::cout << "copy constructor" << std::endl;
    }

    friend void swap(Histogram & first, Histogram & second) noexcept
    {
        using std::swap;
        swap(first.mBinsNum_, second.mBinsNum_);
        swap(first.mData_, second.mData_);
    }
    //move constructor
    Histogram(Histogram&& other) noexcept
        : mBinsNum_(std::exchange(other.mBinsNum_, 0)),
          mData_(std::exchange(other.mData_, nullptr)) { std::cout << "move constructor" << std::endl; }

    //use copy swap idiom to implement copy and move assignment operators
    Histogram & operator=(Histogram other)
    {
        // TODO: remove the comment below
        std::cout << "move or copy assignment operator" << std::endl;
        using std::swap;
        swap(mBinsNum_, other.mBinsNum_);
        swap(mData_, other.mData_);
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
    hsize_t mBinsNum_ = 0;
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
