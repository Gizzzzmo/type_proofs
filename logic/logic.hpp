#pragma once

#include <concepts>
#include <functional>
#include <iostream>
#include <limits>
#include <type_traits>
#include <cassert>

#include "empty_types.hpp"
#include "algo.hpp"

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif

struct MakeExpression {
    protected:
        template<Expression E>
        static constexpr E produce() __attribute__((const));
        friend class Axioms;
};

struct Axioms;

#define MAKE_EXPRESSION(T)\
friend class Axioms;\
friend class MakeExpression;\
template<typename, typename, typename, typename>\
friend class FnDispatcher;



struct GenericVariableMarker {};

template<typename T>
struct MakeVariable : GenericVariableMarker {};

#define MAKE_VARIABLE(T) using type = T

struct FVMarker {};

template<typename T, fv_tag_t _tag>
struct FV : MakeVariable<T>, FVMarker {
    MAKE_VARIABLE(T);
    static constexpr fv_tag_t tag = _tag;
};

struct True : MakeExpression {
    MAKE_EXPRESSION(True);
    constexpr True() __attribute__((const));
    constexpr True(False) __attribute__((const));
};

struct False : MakeExpression {
    MAKE_EXPRESSION(False);
    template<Expression E>
    constexpr False(E, Not<E>) __attribute__((const));
    template<Expression E>
    constexpr False(Not<E>, E) __attribute__((const));
    template<Expression E>
    constexpr False(And<E, Not<E>>) __attribute__((const));
    template<Expression E>
    constexpr False(And<Not<E>, E>) __attribute__((const));
    private:
        constexpr False() __attribute__((const));
};

template<Expression E>
struct Not : MakeExpression {
    MAKE_EXPRESSION(Not);
    constexpr Not(False) __attribute__((const));
    constexpr Not(Implies<E, False>) __attribute__((const));
    template<Expression NegE> requires (std::same_as<E, Not<NegE>>)
    constexpr NegE elim() const __attribute__((const));
    private:
        constexpr Not() __attribute__((const));
};

template<Expression E1, Expression E2>
struct And : MakeExpression {
    MAKE_EXPRESSION(And);
    constexpr And(False) __attribute__((const));
    constexpr And(E1, E2) __attribute__((const));
    constexpr E1 left_elim() const __attribute__((const));
    constexpr E2 right_elim() const __attribute__((const));
    private:
        constexpr And() __attribute__((const));
};

template<Expression E1, Expression E2>
struct Or : MakeExpression {
    MAKE_EXPRESSION(Or);
    constexpr Or(False) __attribute__((const));
    constexpr Or(E1) requires(!std::same_as<E1, False>) __attribute__((const));
    constexpr Or(E2) requires (!std::same_as<E1, E2> && ! std::same_as<E2, False>) __attribute__((const));
    template<Expression Consequence>
    constexpr Consequence elim(Implies<E1, Consequence>, Implies<E2, Consequence>) const __attribute__((const));
    private:
        constexpr Or() __attribute__((const));
};

template<Expression Consequence, Expression... Assumptions>
using deriv = std::function<Consequence(Assumptions...)>;

template<Expression Assumption, Expression Consequence>
struct Implies : MakeExpression {
    MAKE_EXPRESSION(Implies);
    constexpr Implies(False) __attribute__((const));
    template<Expression... Context>
    constexpr Implies(deriv<Consequence, Assumption, Context...>, Context...) __attribute__((const));
    constexpr Consequence elim(Assumption) const __attribute__((const));
    private:
        constexpr Implies() __attribute__((const));
};

template<Expression E1, Expression E2>
using Equiv = And<Implies<E1, E2>, Implies<E2, E1>>;


template<typename T, fv_tag_t x, Expression E>
struct ForAll : MakeExpression {
    MAKE_EXPRESSION(ForAll);
    constexpr ForAll(False) __attribute__((const));
    template<fv_tag_t y> requires (x != y)
    constexpr ForAll(ForAll<T, y, Subst<E, T, x, FV<T, y>>>);
    template<Expression... Context> requires(!FreeIn<FV<T, x>, Context> && ...)
    constexpr ForAll(deriv<E, Context...>, Context...) __attribute__ ((const));
    template<Variable<T> X>
    constexpr Subst<E, T, x, X> elim() const __attribute__ ((const));
    template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
    constexpr ForAll<T, y, Subst<E, T, x, FV<T, y>>> rename() const __attribute__ ((const));
    private:
        constexpr ForAll() __attribute__((const));
};

template<typename T, fv_tag_t x, Expression E>
struct Exists : MakeExpression {
    MAKE_EXPRESSION(Exists);
    constexpr Exists(False) __attribute__((const));
    template<Variable<T> X>
    constexpr Exists(Subst<E, T, x, X>, X) __attribute__ ((const));
    template<Expression Consequence, Expression... Context>
        requires(!FreeIn<FV<T, x>, Consequence> && (!FreeIn<FV<T, x>, Context> && ...))
    constexpr Consequence elim(deriv<Consequence, E, Context...>, Context...) const __attribute__((const));
    template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
    constexpr Exists<T, y, Subst<E, T, x, FV<T, y>>> rename() const __attribute__((const));
    private:
        constexpr Exists() __attribute__((const));
};


template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
struct Equals : MakeExpression {
    MAKE_EXPRESSION(Equals);
    constexpr Equals(False) __attribute__((const));
    constexpr Equals() requires (std::same_as<X, Y>) __attribute__ ((const));
    constexpr Equals(Equals<Y, X>) requires(!std::same_as<X, Y>)__attribute__ ((const));
    template<Variable<typename X::type> Z>
    constexpr Equals(Equals<X, Z>, Equals<Z, Y>) __attribute__ ((const));

    template<typename, fv_tag_t, typename...>
    friend class Tagger;
    private:
        constexpr Equals() __attribute__((const));
};


template<Expression E>
constexpr E MakeExpression::produce() {
    return E();
}

// True Definitions
constexpr True::True() {}

constexpr True::True(False) {}


// False Definitions
template<Expression E>
constexpr False::False(E, Not<E>) {}

template<Expression E>
constexpr False::False(Not<E>, E) {}

template<Expression E>
constexpr False::False(And<E, Not<E>>) {}

template<Expression E>
constexpr False::False(And<Not<E>, E>) {}

constexpr False::False() {}


// Not Definitions
template<Expression E>
constexpr Not<E>::Not(False) {}

template<Expression E>
constexpr Not<E>::Not(Implies<E, False>) {}

template<Expression E>
template<Expression NegE> requires (std::same_as<E, Not<NegE>>)
constexpr NegE Not<E>::elim() const {
    return MakeExpression::produce<NegE>();
}

template<Expression E>
constexpr Not<E>::Not() {}


// And Definitions
template<Expression E1, Expression E2>
constexpr And<E1, E2>::And(False) {}

template<Expression E1, Expression E2>
constexpr And<E1, E2>::And(E1, E2) {}

template<Expression E1, Expression E2>
constexpr E1 And<E1, E2>::left_elim() const {
    return MakeExpression::produce<E1>();
}

template<Expression E1, Expression E2>
constexpr E2 And<E1, E2>::right_elim() const {
    return MakeExpression::produce<E2>();
}

template<Expression E1, Expression E2>
constexpr And<E1, E2>::And() {}


// Or Definitions
template<Expression E1, Expression E2>
constexpr Or<E1, E2>::Or(False) {}

template<Expression E1, Expression E2>
constexpr Or<E1, E2>::Or(E1) requires(!std::same_as<E1, False>) {}

template<Expression E1, Expression E2>
constexpr Or<E1, E2>::Or(E2) requires (!std::same_as<E1, E2> && !std::same_as<E2, False>) {}

template<Expression E1, Expression E2>
template<Expression Consequence>
constexpr Consequence Or<E1, E2>::elim(Implies<E1, Consequence>, Implies<E2, Consequence>) const {
    return MakeExpression::produce<Consequence>();
}

template<Expression E1, Expression E2>
constexpr Or<E1, E2>::Or() {}


// Implies Definitions
template<Expression Assumption, Expression Consequence>
constexpr Implies<Assumption, Consequence>::Implies(False) {}

template<Expression Assumption, Expression Consequence>
template<Expression... Context>
constexpr Implies<Assumption, Consequence>::Implies(deriv<Consequence, Assumption, Context...>, Context...) {}

template<Expression Assumption, Expression Consequence>
constexpr Consequence Implies<Assumption, Consequence>::elim(Assumption) const {
    return MakeExpression::produce<Consequence>();
}

template<Expression Assumption, Expression Consequence>
constexpr Implies<Assumption, Consequence>::Implies() {}


// ForAll Definitions
template<typename T, fv_tag_t x, Expression E>
constexpr ForAll<T, x, E>::ForAll(False) {};

template<typename T, fv_tag_t x, Expression E>
template<fv_tag_t y> requires (x != y)
constexpr ForAll<T, x, E>::ForAll(ForAll<T, y, Subst<E, T, x, FV<T, y>>>) {}

template<typename T, fv_tag_t x, Expression E>
template<Expression... Context> requires(!FreeIn<FV<T, x>, Context> && ...)
constexpr ForAll<T, x, E>::ForAll(deriv<E, Context...>, Context...) {}

template<typename T, fv_tag_t x, Expression E>
template<Variable<T> X>
constexpr Subst<E, T, x, X> ForAll<T, x, E>::elim() const {
    return MakeExpression::produce<Subst<E, T, x, X>>();
}

template<typename T, fv_tag_t x, Expression E>
template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
constexpr ForAll<T, y, Subst<E, T, x, FV<T, y>>> ForAll<T, x, E>::rename() const {
    return MakeExpression::produce<ForAll<T, y, Subst<E, T, x, FV<T, y>>>>();
}

template<typename T, fv_tag_t x, Expression E>
constexpr ForAll<T, x, E>::ForAll() {};


// Exists Definitions
template<typename T, fv_tag_t x, Expression E>
constexpr Exists<T, x, E>::Exists(False) {}

template<typename T, fv_tag_t x, Expression E>
template<Variable<T> X>
constexpr Exists<T, x, E>::Exists(Subst<E, T, x, X>, X) {}

template<typename T, fv_tag_t x, Expression E>
template<Expression Consequence, Expression... Context>
    requires(!FreeIn<FV<T, x>, Consequence> && (!FreeIn<FV<T, x>, Context> && ...))
constexpr Consequence Exists<T, x, E>::elim(deriv<Consequence, E, Context...>, Context...) const {
    return MakeExpression::produce<Consequence>();
}

template<typename T, fv_tag_t x, Expression E>
template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
constexpr Exists<T, y, Subst<E, T, x, FV<T, y>>> Exists<T, x, E>::rename() const {
    return MakeExpression::produce<Exists<T, y, Subst<E, T, x, FV<T, y>>>>();
}

template<typename T, fv_tag_t x, Expression E>
constexpr Exists<T, x, E>::Exists() {};


// Equals Definitions
template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
constexpr Equals<X, Y>::Equals(False) {}

template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
constexpr Equals<X, Y>::Equals() requires(std::same_as<X, Y>) {}

template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
constexpr Equals<X, Y>::Equals(Equals<Y, X>) requires(!std::same_as<X, Y>) {}

template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
template<Variable<typename X::type> Z>
constexpr Equals<X, Y>::Equals(Equals<X, Z>, Equals<Z, Y>) {}

template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
constexpr Equals<X, Y>::Equals() {}

// peano_int arithmetic:

struct MakeCustomIntegral {};

struct peano_int : MakeCustomIntegral {
    private:
        constexpr peano_int(int x, True) : x(x) {  }
    public:
        consteval peano_int(int x) : x(x) { assert(x >= 0); }
        constexpr operator int() const { return x; }
        const int x;
        constexpr peano_int operator+(peano_int other) const { return peano_int(x + other.x, True()); }
};

// signature:
// constant
template<CustomIntegral T>
struct Zero : MakeVariable<T> { MAKE_VARIABLE(T); };

// functions
template<GenericVar X> requires (CustomIntegral<typename X::type>)
struct Succ : MakeVariable<typename X::type> { MAKE_VARIABLE(typename X::type); };

template<GenericVar X> requires (CustomIntegral<typename X::type> && std::is_signed_v<typename X::type>)
struct Pred : MakeVariable<typename X::type> { MAKE_VARIABLE(typename X::type); };

template<GenericVar X, GenericVar Y>
    requires (CustomIntegral<typename X::type> && std::same_as<typename X::type, typename Y::type>)
struct Plus : MakeVariable<typename X::type> { MAKE_VARIABLE(typename X::type); };

template<GenericVar X, GenericVar Y>
    requires (CustomIntegral<typename X::type> && std::same_as<typename X::type, typename Y::type>)
struct Times : MakeVariable<typename X::type> { MAKE_VARIABLE(typename X::type); };

// mapping to primitive type
template<auto _x, CustomIntegral T = decltype(_x)> requires (std::convertible_to<decltype(_x), T>)
struct N : MakeVariable<T> {
    MAKE_VARIABLE(T);
    static constexpr T x = _x; 
};

template<auto _x> requires (_x >= 0)
struct N<_x, peano_int> : MakeVariable<peano_int> {
    MAKE_VARIABLE(peano_int);
    static constexpr peano_int x = _x;
};

// predicates
// axioms

struct Axioms {
    template<Expression E1, Expression E2, Expression... Equalities>
        requires(std::same_as<EqSubst<E1, Equalities...>, EqSubst<E2, Equalities...>>)
    static constexpr E1 from_subst(E2, Equalities...) {
        return MakeExpression::produce<E1>();
    }

    template<CustomIntegral T>
    static constexpr inline Equals<N<0, T>, Zero<T>>
        primitive_base;

    template<auto _x, CustomIntegral T = decltype(_x)> requires (std::convertible_to<decltype(_x), T>)
    static constexpr inline Equals<Succ<N<_x, T>>, N<_x + 1, T>>
        primitive_succ;

    template<auto _x, CustomIntegral T = decltype(_x)> requires (std::convertible_to<decltype(_x), T> && std::is_signed_v<T>)
    static constexpr inline Equals<Pred<N<_x, T>>, N<_x - 1, T>>
        primitive_pred;

    template<auto _x, auto _y, CustomIntegral T = std::common_type_t<decltype(_x), decltype(_y)>>
        requires (std::convertible_to<decltype(_x), T> && std::convertible_to<decltype(_y), T>)
    static constexpr inline Equals<Plus<N<_x, T>, N<_y, T>>, N<_x + _y, T>>
        primitive_plus;

    template<auto _x, auto _y, CustomIntegral T = std::common_type_t<decltype(_x), decltype(_y)>>
        requires (std::convertible_to<decltype(_x), T> && std::convertible_to<decltype(_y), T>)
    static constexpr inline Equals<Times<N<_x, T>, N<_y, T>>, N<_x * _y, T>>
        primitive_times;

    template<CustomIntegral T> requires (std::is_unsigned_v<T>)
    static constexpr inline Equals<Succ<N<std::numeric_limits<T>::max(), T>>, Zero<T>>
        wraparound;

    template<fv_tag_t x = 'x'>
    static constexpr inline Not<Exists<peano_int, x, Equals<Succ<FV<peano_int, x>>, Zero<peano_int>>>>
        zero_is_no_successor;

    template<CustomIntegral T, fv_tag_t x = 'x', fv_tag_t y = 'y'> requires (x != y)
    static constexpr inline 
        ForAll<T, x, ForAll<T, y, 
            Implies<
                Equals<Succ<FV<T, x>>, Succ<FV<T, y>>>,
                Equals<FV<T, x>, FV<T, y>>
            >
        >>
        successor_injective;

    template<CustomIntegral T, fv_tag_t x = 'x', fv_tag_t y = 'y'> requires (x != y && std::is_signed_v<T>)
    static constexpr inline
        ForAll<T, x, ForAll<T, y,
            Implies<
                Equals<Pred<FV<T, x>>, Pred<FV<T, y>>>,
                Equals<FV<T, x>, FV<T, y>>
            >
        >>
        predecessor_injective;

    template<CustomIntegral T, fv_tag_t x = 'x'> requires (std::is_signed_v<T>)
    static constexpr inline
        ForAll<T, x,
            Equals<
                Pred<Succ<FV<T, x>>>,
                FV<T, x>
            >
        >
        successor_predecessor_inverse;

    template<CustomIntegral T, fv_tag_t x = 'x'>
    static constexpr inline
        ForAll<T, x,
            Equals<
                Plus<FV<T, x>, Zero<T>>, 
                FV<T, x>
            >
        >
        addition_base;

    template<CustomIntegral T, fv_tag_t x = 'x', fv_tag_t y = 'y'> requires (x != y)
    static constexpr inline
        ForAll<T, x, ForAll<T, y,
            Equals<
                Plus<FV<T, x>, Succ<FV<T, y>>>,
                Succ<Plus<FV<T, x>, FV<T, y>>>
            >
        >>
        addition_recursion;
    
    template<CustomIntegral T, fv_tag_t x = 'x'>
    static constexpr inline
        ForAll<T, x,
            Equals<
                Times<FV<T, x>, Zero<T>>, 
                Zero<T>
            >
        >
        multiplication_base;

    template<CustomIntegral T, fv_tag_t x = 'x', fv_tag_t y = 'y'> requires (x != y)
    static constexpr inline
        ForAll<T, x, ForAll<T, y,
            Equals<
                Times<FV<T, x>, Succ<FV<T, y>>>,
                Plus<
                    Times<FV<T, x>, FV<T, y>>,
                    FV<T, x>
                >
            >
        >>
        multiplication_recursion;

    template<Expression E, fv_tag_t x = 'x'>
        requires(FreeIn<FV<peano_int, x>, E>)
    static constexpr inline
        Implies<
            And<
                Subst<E, peano_int, x, Zero<peano_int>>,
                ForAll<peano_int, x,
                    Implies<E, Subst<E, peano_int, x, Succ<FV<peano_int, x>>>>
                >
            >,
            ForAll<peano_int, x, E>
        >
        induction;

};

#ifndef NOT_NAMESPACED
} // namespace logic
#endif