#pragma once

#include "empty_types.hpp"
#include <algorithm>
#include <concepts>
#include <limits>
#include <utility>

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif

template<typename... Ts>
struct TypeList { };

template<typename T, typename List>
struct IsInImpl {
    static constexpr bool value = false;
};

template<template<typename...> class tmpl, typename T, typename... Us>
struct IsInImpl<T, tmpl<Us...>> {
    static constexpr bool value = (std::same_as<T, Us> || ... );
};

template<typename T, typename List>
concept IsIn = IsInImpl<T, List>::value;


template<typename List1, typename List2>
struct IsSubsetImpl {
    static constexpr bool value = false;
};

template<
    template<typename...> class tmpl1,
    typename... Ts, typename List2
> struct IsSubsetImpl<tmpl1<Ts...>, List2> {
    static constexpr bool value = (IsIn<Ts, List2> && ...); 
};

template<typename List1, typename List2>
concept IsSubset = IsSubsetImpl<List1, List2>::value;

template<typename... Ts>
struct ConcatImpl {};

template<template<typename...> class tmpl, typename... Ts>
struct ConcatImpl<tmpl<Ts...>> {
    using type = tmpl<Ts...>;
};

template<template<typename...> class tmpl, typename... Ts, typename... Us>
struct ConcatImpl<tmpl<Ts...>, tmpl<Us...>> {
    using type = tmpl<Ts..., Us...>;
};

template<typename T, typename... Ts>
struct ConcatImpl<T, Ts...> {
    using type = typename ConcatImpl<T, typename ConcatImpl<Ts...>::type>::type;
};

template<typename... Ts>
using Concat = typename ConcatImpl<Ts...>::type;



template<typename T, typename List>
struct RemoveImpl {};

template<template<typename...> class tmpl, typename T>
struct RemoveImpl<T, tmpl<>> {
    using type = tmpl<>;
};

template<template<typename...> class tmpl, typename T, typename... Ts>
struct RemoveImpl<T, tmpl<T, Ts...>> {
    using type = typename RemoveImpl<T, tmpl<Ts...>>::type;
};

template<template<typename...> class tmpl, typename U, typename T, typename... Ts>
struct RemoveImpl<U, tmpl<T, Ts...>> {
    using type = Concat<tmpl<T>, typename RemoveImpl<U, tmpl<Ts...>>::type>;
};

template<typename T, typename List>
using Remove = typename RemoveImpl<T, List>::type;


template<ExpOrVar>
struct FVsInImpl {
    using type = TypeList<>;
};

template<typename T, fv_tag_t x>
struct FVsInImpl<FV<T, x>> {
    using type = TypeList<FV<T, x>>;
};

template<template<typename...> class Comb, ExpOrVar... Es> requires(Expression<Comb<Es...>>)
struct FVsInImpl<Comb<Es...>> {
    using type = Concat<typename FVsInImpl<Es>::type...>;
};

template<template<typename...> class Func, GenericVar... Vs> requires(GenericVar<Func<Vs...>>)
struct FVsInImpl<Func<Vs...>> {
    using type = Concat<typename FVsInImpl<Vs>::type...>;
};

template<
    template<typename, fv_tag_t, typename> class Quant, Expression E,
    typename T, fv_tag_t x
> requires(Expression<Quant<T, x, E>>)
struct FVsInImpl<Quant<T, x, E>> {
    using type = Remove<FV<T, x>, typename FVsInImpl<E>::type>;
};

template<ExpOrVar E>
using FVsIn = typename FVsInImpl<E>::type;

template<typename V, typename E>
concept FreeIn = GenericVar<V> && ExpOrVar<E> && IsIn<V, FVsIn<E>>;


template<GenericVar, ExpOrVar>
struct BoundInImpl {
    static constexpr bool value = false;
};

template<
    typename T, fv_tag_t x, template<typename, fv_tag_t, typename> class Quant,
    Expression E
> requires(Expression<Quant<T, x, E>>)
struct BoundInImpl<FV<T, x>, Quant<T, x, E>> {
    static constexpr bool value = true;
};

template<typename T, fv_tag_t x, template<typename...> class Comb, ExpOrVar... Es>
    requires(Expression<Comb<Es...>>)
struct BoundInImpl<FV<T, x>, Comb<Es...>> {
    static constexpr bool value = (BoundInImpl<FV<T, x>, Es>::value || ...);
};

template<typename T, fv_tag_t x, template<typename...> class Func, GenericVar... Vs>
    requires(GenericVar<Func<Vs...>>)
struct BoundInImpl<FV<T, x>, Func<Vs...>> {
    static constexpr bool value = (BoundInImpl<FV<T, x>, Vs>::value || ...);
};

template<
    typename T, fv_tag_t x, template<typename, fv_tag_t, typename> class Quant,
    typename TT, fv_tag_t xx, Expression E
> requires((!std::same_as<T, TT> || x != xx) && Expression<Quant<TT, xx, E>>)
struct BoundInImpl<FV<T, x>, Quant<TT, xx, E>> {
    static constexpr bool value = BoundInImpl<FV<T, x>, E>::value;
};

template<typename V, typename E>
concept BoundIn = GenericVar<V> && ExpOrVar<E> && BoundInImpl<V, E>::value;

template<typename List, ExpOrVar E>
struct NoneBoundInImpl {};

template<template<typename...> class tmpl, GenericVar... Vs, ExpOrVar E>
struct NoneBoundInImpl<tmpl<Vs...>, E> {
    static constexpr bool value = (!BoundIn<Vs, E> && ...);
};

template<typename List, typename E>
concept NoneBoundIn = ExpOrVar<E> && NoneBoundInImpl<List, E>::value; 

template<typename T, fv_tag_t x, ExpOrVar E, Variable<T> X>
    requires(NoneBoundIn<FVsIn<X>, E>)
struct SubstImpl {
    using type = E;
};

template<typename T, fv_tag_t x, Variable<T> X>
struct SubstImpl<T, x, FV<T, x>, X> {
    using type = X;
};

template<typename T, fv_tag_t x, template<typename...> class Comb, ExpOrVar... Es, Variable<T> X>
    requires(Expression<Comb<Es...>>)
struct SubstImpl<T, x, Comb<Es...>, X> {
    using type = Comb<typename SubstImpl<T, x, Es, X>::type...>;
};

template<typename T, fv_tag_t x, template<typename...> class Func, GenericVar... Vs, Variable<T> X>
    requires(GenericVar<Func<Vs...>>)
struct SubstImpl<T, x, Func<Vs...>, X> {
    using type = Func<typename SubstImpl<T, x, Vs, X>::type...>;
};

template<typename T, fv_tag_t x, typename TT, fv_tag_t xx, Expression E, Variable<T> X>
    requires((!std::same_as<T, TT> || x != xx) && !std::same_as<X, FV<TT, xx>>)
struct SubstImpl<T, x, ForAll<TT, xx, E>, X> {
    using type = ForAll<TT, xx, typename SubstImpl<T, x, E, X>::type>;
};

template<typename T, fv_tag_t x, typename TT, fv_tag_t xx, Expression E, Variable<T> X>
    requires((!std::same_as<T, TT> || x != xx)  && !std::same_as<X, FV<TT, xx>>)
struct SubstImpl<T, x, Exists<TT, xx, E>, X> {
    using type = Exists<TT, xx, typename SubstImpl<T, x, E, X>::type>;
};

template<Expression E, typename T, fv_tag_t x, Variable<T> X>
    requires(NoneBoundIn<FVsIn<X>, E>)
using Subst = SubstImpl<T, x, E, X>::type;

template<ExpOrVar E, Expression... Equalities>
struct EqSubstImpl {

};

template<ExpOrVar E, Expression Eq, Expression... Eqs>
struct EqSubstImpl<E, Eq, Eqs...> {
    using type = typename EqSubstImpl<typename EqSubstImpl<E, Eq>::type, Eqs...>::type;
};

template<GenericVar X, GenericVar Y, GenericVar Z>
    requires(std::same_as<typename X::type, typename Y::type>)
struct EqSubstImpl<Z, Equals<X, Y>> {
    using type = Z;
};

template<GenericVar V1, GenericVar V2>
    requires(std::same_as<typename V1::type, typename V2::type>)
struct EqSubstImpl<V1, Equals<V1, V2>> {
    using type = V2; 
};

template<template<typename...> class Comb, ExpOrVar... Es, GenericVar X, GenericVar Y>
    requires(
        Expression<Comb<Es...>> &&
        std::same_as<typename X::type, typename Y::type>
    )
struct EqSubstImpl<Comb<Es...>, Equals<X, Y>> {
    using type = Comb<typename EqSubstImpl<Es, Equals<X, Y>>::type...>;
};

template<template<typename...> class Func, GenericVar... Vs, GenericVar X, GenericVar Y>
    requires(
        GenericVar<Func<Vs...>> &&
        std::same_as<typename X::type, typename Y::type> &&
        !std::same_as<Func<Vs...>, X>
    )
struct EqSubstImpl<Func<Vs...>, Equals<X, Y>> {
    using type = Func<typename EqSubstImpl<Vs, Equals<X, Y>>::type...>;
};

template<
    typename T, fv_tag_t x, template<typename, fv_tag_t, typename> class Quant, 
    Expression E, GenericVar X, GenericVar Y
> requires(
    Expression<Quant<T, x, E>> &&
    !std::same_as<Quant<T, x, E>, X> &&
    !IsIn<FV<T, x>, Concat<FVsIn<Y>, FVsIn<X>>>
)
struct EqSubstImpl<Quant<T, x, E>, Equals<X, Y>> {
    using type = Quant<T, x, typename EqSubstImpl<E, Equals<X, Y>>::type>;
};

template<ExpOrVar E, Expression... Equalities>
using EqSubst = typename EqSubstImpl<E, Equalities...>::type;

template<ExpOrVar E, typename EqualityList>
struct EqSubstListImpl {

};

template<ExpOrVar E, Expression... Equalities>
struct EqSubstListImpl<E, TypeList<Equalities...>> {
    using type = EqSubst<E, Equalities...>;
};

template<ExpOrVar E, typename EqualityList>
using EqSubstList = typename EqSubstListImpl<E, EqualityList>::type;


template<typename List, typename Predicate>
struct FilterImpl {};

template<template<typename...> class tmpl, typename Predicate>
struct FilterImpl<tmpl<>, Predicate> {
    using type = tmpl<>;
};

template<template<typename...> class tmpl, typename Predicate, typename Head, typename... Tail>
    requires (Predicate::template value<Head>)
struct FilterImpl<tmpl<Head, Tail...>, Predicate> {
    using type = Concat<tmpl<Head>, typename FilterImpl<tmpl<Tail...>, Predicate>::type>;
};

template<template<typename...> class tmpl, typename Predicate, typename Head, typename... Tail>
    requires (!Predicate::template value<Head>)
struct FilterImpl<tmpl<Head, Tail...>, Predicate> {
    using type = typename FilterImpl<tmpl<Tail...>, Predicate>::type;
};

template<typename List, typename Predicate>
using Filter = typename FilterImpl<List, Predicate>::type;

template<std::integral T, T... xs>
    requires (sizeof...(xs) < std::numeric_limits<T>::max() - std::numeric_limits<T>::min())
    // could be relaxed to sizeof unique elements in xs
constexpr T unused_num() {
    int arr[] = {xs...};

    std::sort(arr, &arr[sizeof...(xs)]);

    T n = std::numeric_limits<T>::min();
    for (size_t i = 0; i < sizeof...(xs); i++) {
        if (arr[i] > n)
            break;
        n++;
    }
    
    return n;
}

template<typename T>
struct FreeVarP {
    template<typename V>
    static constexpr bool value = FreeVar<V> && std::same_as<typename V::type, T>;
};

template<typename List>
struct UnusedFVTagImpl {};

template<template<typename...> class tmpl, FreeVar... FVs>
struct UnusedFVTagImpl<tmpl<FVs...>> {
    static constexpr fv_tag_t value = unused_num<fv_tag_t, FVs::tag...>();
};

template<typename List>
inline static constexpr fv_tag_t UnusedFVTag = UnusedFVTagImpl<List>::value;

template<typename T, typename List>
struct UnusedFVImpl {
    using type = FV<T, UnusedFVTag<Filter<List, FreeVarP<T>>>>;
};

template<typename T, typename List>
using UnusedFV = typename UnusedFVImpl<T, List>::type;

template<
    typename List1, typename List2,
    template<typename, typename> class inner = std::pair,
    template<typename...> class outer = TypeList
> struct ZipImpl {};


template<
    template<typename...> class tmpl1, template<typename...> class tmpl2,
    template<typename, typename> class inner, template<typename...> class outer,
    typename... Leftover
> struct ZipImpl<tmpl1<Leftover...>, tmpl2<>, inner, outer> {
    using type = outer<>;  
};

template<
    template<typename...> class tmpl1, template<typename...> class tmpl2,
    template<typename, typename> class inner, template<typename...> class outer,
    typename... Leftover
> requires(sizeof...(Leftover) != 0)
struct ZipImpl<tmpl1<>, tmpl2<Leftover...>, inner, outer> {
    using type = outer<>; 
};

template<
    template<typename...> class tmpl1, typename Head1, typename... Tail1,
    template<typename...> class tmpl2, typename Head2, typename... Tail2,
    template<typename, typename> class inner, template<typename...> class outer 
> struct ZipImpl<tmpl1<Head1, Tail1...>, tmpl2<Head2, Tail2...>, inner, outer> {
    using type = Concat<outer<inner<Head1, Head2>>, typename ZipImpl<tmpl1<Tail1...>, tmpl2<Tail2...>>::type>;
};


template<
    typename List1, typename List2,
    template<typename, typename> class inner = std::pair,
    template<typename...> class outer = TypeList
> using Zip = typename ZipImpl<List1, List2, inner, outer>::type;




#ifndef NOT_NAMESPACED
} //namespace logic
#endif
