#ifndef OFS_COLLECTION2_HPP
#define OFS_COLLECTION2_HPP

#include <vector>
#include <string>

namespace sc
{
    using std::string;
    using std::vector;

    template <typename ElemT>
    class Collection: public vector<ElemT> 
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
} // namespace sc

#endif // OFS_COLLECTION2_HPP