#pragma once

#include <type_traits>
#include <concepts>
#include <functional>

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif
#ifdef FREE_VAR_TAG_TYPE
using fv_tag_t = FREE_VAR_TAG_TYPE;
#else
using fv_tag_t = unsigned char;
#endif

template<typename... Ts>
struct TypeList;


// logic
struct MakeExpression;
struct GenericVariableMarker;
template<typename T> struct MakeVariable;
struct FVMarker;

template<typename T>
concept Expression = std::is_base_of_v<MakeExpression, T>;// && sizeof(T) == 1;

template<typename V>
concept GenericVar = std::is_base_of_v<GenericVariableMarker, V> && requires {
    typename V::type;
};

template<typename V, typename T>
concept Variable = std::is_base_of_v<MakeVariable<T>, V> && std::same_as<typename V::type, T>;

template<fv_tag_t _tag>
struct Checker { static constexpr fv_tag_t tag = _tag; };

template<typename T>
concept FreeVar = std::is_base_of_v<FVMarker, T> && GenericVar<T> && requires() {
    {Checker<T::tag>::tag} -> std::same_as<const fv_tag_t&>;
};

template<typename T>
concept ExpOrVar = GenericVar<T> || Expression<T>;

struct True;
struct False;
template<Expression E> struct Not;
template<Expression Assumption, Expression Consequence> struct Implies;
template<Expression E1, Expression E2> struct And;
template<Expression E1, Expression E2> struct Or;
template<typename T, fv_tag_t x, Expression E> struct ForAll;
template<typename T, fv_tag_t x, Expression E> struct Exists;
template<GenericVar X, GenericVar Y> struct Equals;

template<typename T, fv_tag_t _x> struct FV;


// peano
struct Zero;
template<Variable<int>> struct Succ;
template<Variable<int>, Variable<int>> struct Plus;
template<Variable<int>, Variable<int>> struct Times;


// tagging logic
template<typename Fn, typename Out, typename... Args>
concept Function = requires(Fn fn) {
    {static_cast<std::function<Out(Args...)>>(fn)} -> std::same_as<std::function<Out(Args...)>>;
};


template<typename T, fv_tag_t x, typename... FVs>
struct Tagger;

template<typename Inputs, typename Assumptions, typename Output, typename Fn>
struct FnDispatcher;

template<FreeVar... FVs>
class Context;


// tagged types
template<Variable<int> X>
class Int;

template<Expression E>
class Bool;


#ifndef NOT_NAMESPACED
} // namespace logic
#endif