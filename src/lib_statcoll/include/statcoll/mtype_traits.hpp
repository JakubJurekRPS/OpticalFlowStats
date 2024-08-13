#ifndef MTYPE_TRAITS_HPP
#define MTYPE_TRAITS_HPP

#include <tuple>
#include <variant>

namespace sc
{

using std::tuple;
using std::tuple_element;

template <int N, typename... Ts>
using nth_type = typename tuple_element<N, tuple<Ts...>>::type;

template<typename Sig>
struct free_fn_traits;

template<typename R, typename... Args>
struct free_fn_traits<R(*)(Args...)>
{
    template <int N>
    using arg_type = typename tuple_element<N, tuple<Args...>>::type;

    static consteval int num_args() { return sizeof...(Args);}
};

template<typename Tfnc>
struct mbr_fn_traits;

template <typename TObj, typename R, typename... Args>
struct mbr_fn_traits<R (TObj::*)(Args...)>
{
    typedef R result_type;
    template <int N>
    using arg_type = typename tuple_element<N, tuple<Args...>>::type;
    static consteval int num_args() { return sizeof...(Args);}
};


template <int N, typename Tfnc>
using mbr_fn_arg_t = typename mbr_fn_traits<Tfnc>::template arg_type<N>;

template <typename Tfnc>
using mbr_fn_res_t = typename mbr_fn_traits<Tfnc>::result_type;

template<int N, typename Tfnc>
using free_fn_arg_t = typename free_fn_traits<Tfnc>::template arg_type<N>;

template<typename Tfnc>
using free_fn_res_t = typename free_fn_traits<Tfnc>::result_type;


template <typename T, typename... Ts>
class unique_types
{
private:
    template <typename... Tuniques>
    struct Output
    {
        using variant_t = std::variant<Tuniques...>;
        using tuple_t = std::tuple<Tuniques...>;
        static consteval unsigned short number() {return sizeof...(Tuniques);}
    };
    template<typename T0>
    static consteval bool is_unique()
    {
        return true;
    }

    template<typename T0,typename T1, typename... Tuniques>
    static consteval bool is_unique()
    {
        if constexpr (sizeof...(Tuniques) == 0)
        {
            return !std::is_same<T0,T1>::value;
        }
        else
        {
            return (!std::is_same<T0,T1>::value) && is_unique<T0,Tuniques...>();
        }
    }
    
    template <int N, typename... Tuniques>
    static consteval auto get_uniques()
    {
        if constexpr (sizeof...(Ts) == 0)
        {
            return Output<T>{};
        }
        else
        {
            if constexpr (N == 0)
            {
                return Output<Tuniques...>{};
            }
            else
            {
                if constexpr (is_unique<nth_type<N-1, T, Ts...>,Tuniques...>())
                    return get_uniques<N-1, nth_type<N-1, T, Ts...>,Tuniques...>();
                else
                    return get_uniques<N-1, Tuniques...>();
            }
        }
    }
    
public:
    static consteval bool is_unique()
    {
        return is_unique<T,Ts...>();
    }

    using variant_t = decltype(get_uniques<1+sizeof...(Ts)>())::variant_t;
    using tuple_t = decltype(get_uniques<1+sizeof...(Ts)>())::tuple_t;
    static consteval auto number() 
    { 
        return decltype(get_uniques<1+sizeof...(Ts)>())::number();
    }
};


}; // namespace sc

#endif // TYPE_TRAITS_HPP