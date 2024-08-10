#ifndef OFS_COLLECTOR_HPP
#define OFS_COLLECTOR_HPP

#include <string>
#include <map>
#include <array>
#include <vector>
#include <memory>
#include <concepts>
#include <variant>
#include "mtype_traits.hpp"
#include <type_traits>
#include <nlohmann/json.hpp>
#include <fstream>
#include <thread>
#include <future>
#include "base_classes.hpp"
#include <chrono>
#include <ctime> 

namespace sc
{

using std::string;
using std::map;
using std::unique_ptr;
using std::make_unique;
using std::array;
using std::vector;
using std::variant;
using nlohmann::json;
using std::thread;
using std::future;
using std::async;
using std::movable;
using std::chrono::system_clock;

class CollectorException : public std::exception
{
public:
    explicit CollectorException(const std::string& message) : mMessage(message) {}

    const char* what() const noexcept override
    {
        return mMessage.c_str();
    }

private:
    std::string mMessage;
};

template <template<typename> typename T>
// It should be a Tcollection property - shouldn't depend on element type 
concept MovableCollection = movable<T<int>> && requires(T<int> coll)
{
    { coll.emplace_back(1) };
    { coll.clear() };
    { coll.size() };
    { coll[0] };
    { coll[0] = 1 };
};

template <typename T>
    concept HasStaticGetName = requires(T) { T::getName(); };


// TODO: Default constructible functors?
// TODO: Other concepts for Collector template types
template <template<typename>  typename Tcollection, typename... Ts>
    requires (unique_types<Ts...>::number() == sizeof...(Ts))
    && MovableCollection<Tcollection> // have to check Tcollection here, because the language doesn't support concepts as templated arguments
    && (unique_types<mbr_fn_arg_t<0, decltype(&Ts::operator())>...>::number() == 1)  // functors call operator should accept the first (input data) argument of the same type
    && (HasStaticGetName<Ts> && ...)
class Collector
{
    template <typename T>
    using call_op_res_t = mbr_fn_res_t<decltype(&T::operator())>;

    using Tinp = mbr_fn_arg_t<0, decltype(&nth_type<0, Ts...>::operator())>;
// TODO: Use variant and get() instead of unique_ptrs. It's not worth to use pointers as accelaration is tiny
    using CollVariant = unique_types<Tcollection<call_op_res_t<Ts>>...>::variant_t;
    using FunctorArray = array<vector<unique_ptr<FunctorBase>>, sizeof...(Ts)>;
    using CollectionArray = array<vector<CollVariant>, sizeof...(Ts)>;
    using FunctorFactFn = void(*)(FunctorArray & ,const json &);
    using CollectionFactoryFn = void(*)(CollectionArray &, const string &, const int);

    struct FactoryBindings{
        unsigned mIndex = 0; 
        FunctorFactFn mMakeFunctor = nullptr;
        CollectionFactoryFn mMakeCollection = nullptr;
    };

    template <int N, int ARG_NUM>
        requires (N < sizeof...(Ts)) && (N >= 0) && (ARG_NUM >= 0) 
    static unique_ptr<FunctorBase> make_functor(const json & conf, auto&&... params)
    {
        using ObjT = nth_type<N, Ts...>;
        if constexpr (ARG_NUM == 0)
        {
            return ObjT::create(std::forward<decltype(params)>(params)...);
        }
        else
        {
            using ParamT = free_fn_arg_t<0, decltype(&ObjT::create)>;

            const auto & [k,v] = conf.at(ARG_NUM-1).items().begin();
            ParamT par;
            try { par = v.get<ParamT>(); }
            catch(const std::exception& e)
            {
                throw CollectorException("Invalid argument type in config file -> Stat name: "
                + ObjT::getName() + string{", param: "} + conf.at(ARG_NUM-1).dump());
            }
            std::cout << ":" << par;
            return make_functor<N,ARG_NUM-1>(conf,par,std::forward<decltype(params)>(params)...);
        }
    }

    template<int N>
        requires (N < sizeof...(Ts)) && (N >= 0)
    static void func_factory(FunctorArray & functors, const json & config)
    {
        using ObjT = nth_type<N, Ts...>;
        constexpr auto ObjArgNum = free_fn_traits<decltype(&ObjT::create)>::num_args();
        if ( config.size() != ObjArgNum)
        {
            throw CollectorException("Invalid number of arguments in config file");
        }
        std::cout << "Creating functor for  " << ObjT::getName();
        if(0 < config.size()) 
        {
            std::cout << " with params ";
        }
        functors[N].emplace_back(make_functor<N, ObjArgNum>(config));
        std::cout << std::endl;
    }

    template<int N>
        requires (N < sizeof...(Ts)) && (N >= 0)
    static void coll_factory(CollectionArray & collections, const string & batch, const  int id)
    {
        using ObjT = nth_type<N, Ts...>;
        using ElemT = call_op_res_t<ObjT>;
        std::cout << "Creating collection for " << ObjT::getName() << std::endl;
        collections[N].emplace_back( 
            Tcollection<ElemT>(
                ObjT::getName(),
                batch,
                ObjT::getDescription(),
                id)
            );
    }

    template<int N>
        requires (N < sizeof...(Ts)) && (N >= 0)
    static void create_factory_bindings(map<string, FactoryBindings> & factoryBindings)
    {
        string name = nth_type<N, Ts...>::getName();
        factoryBindings[name] = FactoryBindings{N, &(func_factory<N>), &(coll_factory<N>)};
        if constexpr (N>0)
        {
            create_factory_bindings<N-1>(factoryBindings);
        }
    }  

    void configure(const string configFile, bool confFunctors = true, bool confCollections = true)
    {
        std::ifstream file(configFile);
        if (!file.is_open())
        {
            std::cerr << "Failed to open config file: " << configFile << std::endl;
            return;
        }

        json configs;
        file >> configs;    
        file.close();

        int collId = 0;
        std::time_t timestamp = time(NULL);
        struct std::tm datetime = *localtime(&timestamp);
        char output[50];
        strftime(output, 50, "%y-%m-%d_%H-%M-%S", &datetime);
        string batch = string{output};
        for (const auto & el : configs)
        {
            json config;
            try {
                config = el.at("config");
            }
            catch (const std::exception& e) { 
                /*Should be empty*/ 
            }

            try{
                auto & fact = mFactoryBindings_.at(el.at("StatName").get<string>());
                if(confFunctors) 
                    fact.mMakeFunctor(mFunctors_, config);
                if(confCollections) 
                    fact.mMakeCollection(mCollections_, batch, collId);
            }
            catch(const std::out_of_range& e)
            {
                std::cerr << "[WARNING] Invalid StatName in config file: " << el.at("StatName").get<string>() << std::endl;
            }
            ++collId;
        }
    }

    template <int N>
        requires (N < sizeof...(Ts)) && (N >= 0)
    static void run_nth_type(const Tinp & input, CollectionArray & collections, FunctorArray & functors)
    //kept static to make threads creation easier
    {
        using ObjT = nth_type<N, Ts...>;
        using ElemT = call_op_res_t<ObjT>;

        vector<future<ElemT>> results;
        for (int i = 0; i < collections[N].size(); i++)
        {
            results.emplace_back(async([&functors](const Tinp & input, int i)
            {
                return static_cast<ObjT*>(functors[N][i].get())->operator()(input);
            }, 
            std::ref(input), i));
        }
        for (int i = 0; i < results.size(); i++)
        {
            std::get<Tcollection<ElemT>>(collections[N][i]).emplace_back(results[i].get());
        }
    }

    template <int N> 
        requires (N < sizeof...(Ts)) && (N >= 0)
    void runEachType(const Tinp & input, vector<thread> & threads)
    {
        threads.emplace_back(thread(run_nth_type<N>, std::ref(input), std::ref(mCollections_), std::ref(mFunctors_)));
        if constexpr (N>0)
        {
            runEachType<N-1>(input, threads);
        }
    }

    map<string, FactoryBindings> mFactoryBindings_;
    FunctorArray mFunctors_;
    CollectionArray mCollections_;
    string mConfigFile_;
public:
    Collector(const string & configFile): mConfigFile_{configFile}
    {
        create_factory_bindings<sizeof...(Ts)-1>(mFactoryBindings_);
        configure(configFile);
    }
    
    void run(const Tinp & input)
    {
        vector<thread> threads;
        runEachType<sizeof...(Ts)-1>(input, threads);
        for (auto & t : threads)
        {
            t.join();
        }
    }

    void reconfigure(const string configFile = "", 
                        bool confFunctors = true, bool confCollections = true)
    {
        string config = configFile;
        if(configFile.empty())
        {
            config = mConfigFile_;
        }
        if(confCollections)
        {
            for (auto & coll : mCollections_)
            {
                coll.clear();
            }
        }
        if(confFunctors)
        {
            for (auto & funct: mFunctors_)
            {
                funct.clear();
            }
        }
        configure(config, confFunctors, confCollections);
    }

    auto getCollections()
    {
        vector<CollVariant> colls;
        colls.reserve(sizeof...(Ts));
        for( auto & collvec : mCollections_)
        {
            for (auto & coll : collvec)
            {
                colls.emplace_back(std::move(coll));
            }
        }
        reconfigure(mConfigFile_, false, true);
        return colls;
    }
};

} // namespace sc

#endif // OFS_COLLECTOR_HPP

