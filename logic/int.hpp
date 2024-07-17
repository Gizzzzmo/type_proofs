#include "algo.hpp"
#include "empty_types.hpp"
#include "logic.hpp"
#include "tagging.hpp"

#include <cassert>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif

template<Expression E>
class Bool {
    private:
        constexpr Bool(bool x) : x(x) {}
        template<Variable<int>>
        friend class Int;
        template<Expression>
        friend class Bool;
        template<typename, typename>
        friend class FnDispatcher;
    public:
        constexpr Bool(E) : x(true) {}
        constexpr Bool(Not<E>) : x(false) {}
        constexpr operator bool() {
            return x;
        }

        constexpr Bool<Not<E>> operator!() {
            return Bool<Not<E>>(!x);
        }

        template<Expression E2>
        constexpr Bool<And<E, E2>> operator&&(Bool<E2> other) {
            return Bool<And<E, E2>>(x && other.x);
        }

        template<Expression E2>
        constexpr Bool<Or<E, E2>> operator||(Bool<E2> other) {
            return Bool<Or<E, E2>>(x && other.x);
        }

    private:
        bool x;
};


template<Variable<int> X, Variable<int> Y, char z = UnusedFV<int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using GreaterThanEq = Exists<int, z, Equals<Plus<FV<int, z>, Y>, X>>;

template<Variable<int> X, Variable<int> Y, char z = UnusedFV<int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using GreaterThan = GreaterThanEq<X, Succ<Y>, z>;


template<Variable<int> X, Variable<int> Y, char z = UnusedFV<int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using LessThanEq = Not<GreaterThan<X, Y, z>>;

template<Variable<int> X, Variable<int> Y, char z = UnusedFV<int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using LessThan = Not<GreaterThanEq<X, Y, z>>;



template<Variable<int> X>
class Int {
    private:
        constexpr Int(int x) : x(x) {}
        template<typename T, fv_tag_t x, typename... _FVs>
        friend class Tagger;
        template<Variable<int> Y>
        friend class Int;
        template<typename, typename, typename>
        friend class FnDispatcher;
    public:
        using TagVar = X;
        using Wrapped = int;
        consteval Int() : x(X::x) {}

        constexpr operator int() {
            return x;
        }

        template<Variable<int> Y>
        constexpr Bool<Equals<X, Y>> operator==(Int<Y> other) {
            return Bool<Equals<X, Y>>(x == other.x);
        }

        template<Variable<int> Y>
        constexpr Int<Plus<X, Y>> operator+(Int<Y> other) {
            return Int<Plus<X, Y>>(x + other.x);
        }

        template<Variable<int> Y>
        constexpr Bool<GreaterThanEq<X, Y>> operator>(Int<Y> other) {
            return Bool<GreaterThanEq<X, Y>>(x > other.x);
        }
    private:
        int x;
};


#ifndef NOT_NAMESPACED
}
#endif