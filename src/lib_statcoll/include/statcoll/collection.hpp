#ifndef OFS_COLLECTION2_HPP
#define OFS_COLLECTION2_HPP

#include <vector>
#include <string>
#include "base_classes.hpp"

namespace sc
{
    using std::string;
    using std::vector;

    // Base class for all collections too enable homogeneous storage

    template <typename ElemT>
    class Collection: public CollectionBase, public vector<ElemT> 
    {
        string mName_;
        string mBatch_;
        string mDescription_;
        int mId_;
    public:
        Collection(const string &name, const string &batch, const string &description, const int id)
            : mName_{name}, mBatch_{batch}, mDescription_{description}, mId_{id} {}

        auto getName() const { return mName_; }
        auto getBatch() const { return mBatch_; }
        auto getDescription() const { return mDescription_; }
        auto getId() const { return mId_; }
    };

// TODO: batch, name and stat info
} // namespace sc

#endif // OFS_COLLECTION2_HPP