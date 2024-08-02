#ifndef COLLECTIONPRINTER_HPP
#define COLLECTIONPRINTER_HPP

#include <concepts>
#include <ostream>
#include <nlohmann/json.hpp>
#include "memory"
#include <fstream>
#include <filesystem>


namespace sc
{

using std::ostream;
using nlohmann::json;
using std::unique_ptr;
using std::make_unique;
using std::endl;
using std::ofstream;
using std::filesystem::create_directories;

template <typename T>
concept Iterable = requires(T t) 
{
    t.begin();
    t.end();
};

template <typename T>
concept WithValMember = requires(T t, ostream &os)
{
    {t.val};
    requires std::is_scalar<decltype(t.val)>::value;
};

template <WithValMember ElemT>
void to_json(json &j, const ElemT &elem)
{
    j = elem.val;
}

template <typename T>
concept PushableToJsonArray = requires(T t, json::array_t j) 
{
    { j.push_back(t) };
};

class JsonPrinter
{
    void printInfo(const auto & collection)
    {
        mOutputStream_ << "  " << R"("info":)" << endl;
        mOutputStream_ << "  " << R"({)" << endl;
        mOutputStream_ << "    " <<R"("name": )"  << "\"," << collection.getName()  << "\"," << endl;
        string batch = collection.getBatch();
        mOutputStream_ << "    " <<R"("batch": )"  << "\"" << batch.substr(0, batch.size()-1)  << "\"," << endl;
        mOutputStream_ << "    " <<R"("description": )"  << "\"" << collection.getDescription()  << "\"," << endl;
        mOutputStream_ << "    " <<R"("id": )"  << collection.getId()  << endl;
        mOutputStream_ << "  " << R"(},)" << endl;
    }

    ostream &mOutputStream_;
public:
    JsonPrinter(ostream &outputStream) : mOutputStream_(outputStream) {}

    template <typename ElemT, template<typename> typename Tcollection> 
        requires Iterable<Tcollection<ElemT>>
            && Iterable<ElemT>
            && PushableToJsonArray<ElemT>
    void operator()(const Tcollection<ElemT> &collection) 
    {
        mOutputStream_  << R"({)"<< endl;
        printInfo(collection);
        mOutputStream_ << "  " << R"("data":)" << endl << "  " <<  R"([)" << endl;
        for (const auto &elem : collection)
        {
            json::array_t valArray;
            valArray.reserve(elem.size());
            for(const auto &val : elem)
            {
                valArray.push_back(val);
            }
            mOutputStream_ << "    " << valArray << ((&elem == &collection.back()) ? "" : ",") << endl;
        }
        mOutputStream_ << "  " << R"(])" << endl << R"(})" << endl;
    }

    template <typename ElemT, template<typename> typename Tcollection>
        requires Iterable<Tcollection<ElemT>>
            && PushableToJsonArray<ElemT>
            && (!Iterable<ElemT>)
    void operator()(const Tcollection<ElemT> &collection) 
    {
        mOutputStream_  << R"({)"<< endl;
        printInfo(collection);
        mOutputStream_ << "  " << R"("data":)" << endl << "  ";
        json::array_t valArray;
        valArray.reserve(collection.size());
        for ( auto &elem : collection)
        {
            valArray.push_back(elem);
        }
        mOutputStream_ << valArray << endl << R"(})" << endl;
    }
    static const string getFileNameExtension() { return "json"; }
};

template <typename Tprinter>
class FilePrinter
{
public:
    void operator()(const auto &collection)
    {
        string resDir = "statResults/" + collection.getBatch() + "/";
        create_directories(resDir);
        string fileName = collection.getName()  + "." + Tprinter::getFileNameExtension();
        ofstream file(resDir + fileName);
        Tprinter printer(file);
        printer(collection);
    }
    
};

} // namespace sc

#endif // COLLECTIONPRINTER_HPP