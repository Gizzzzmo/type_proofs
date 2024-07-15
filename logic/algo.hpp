#pragma once

#include "empty_types.hpp"
#include <concepts>

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

template<GenericVar V1, GenericVar V2>
    requires(std::same_as<typename V1::type, typename V2::type>)
struct EqSubstImpl<V1, Equals<V1, V2>> {
    using type = V2; 
};



#ifndef NOT_NAMESPACED
} //namespace logic
#endif
