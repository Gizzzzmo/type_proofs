#pragma once

#include <type_traits>
#include <concepts>

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif
#ifdef FREE_VAR_TAG_TYPE
using fv_tag_t = FREE_VAR_TAG_TYPE;
#else
using fv_tag_t = char;
#endif

struct MakeExpression;
struct GenericVariableMarker;
template<typename T> struct MakeVariable;

template<typename T>
concept Expression = std::is_base_of_v<MakeExpression, T> && sizeof(T) == 1;

template<typename V>
concept GenericVar = std::is_base_of_v<GenericVariableMarker, V> && requires {
    typename V::type;
};

template<typename V, typename T>
concept Variable = std::is_base_of_v<MakeVariable<T>, V> && std::same_as<typename V::type, T>;

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

struct Zero;
template<Variable<int>> struct Succ;
template<Variable<int>, Variable<int>> struct Plus;
template<Variable<int>, Variable<int>> struct Times;

#ifndef NOT_NAMESPACED
} // namespace logic
#endif