#pragma once

#include <concepts>
#include <functional>

#include "empty_types.hpp"
#include "algo.hpp"

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif

struct MakeExpression {
    constexpr MakeExpression(False) __attribute__((const));
    protected:
        constexpr MakeExpression() {};
        template<Expression E>
        constexpr E produce() const __attribute__((const));
};

struct Axioms;

#define MAKE_EXPRESSION(T)\
using MakeExpression::MakeExpression;\
friend class Axioms;\
friend class MakeExpression;


struct GenericVariableMarker {};

template<typename T>
struct MakeVariable : GenericVariableMarker {};

#define MAKE_VARIABLE(T) using type = T

template<typename T, fv_tag_t _x>
struct FV : MakeVariable<T> {
    MAKE_VARIABLE(T);
    static constexpr fv_tag_t x = _x;
};

struct True : MakeExpression {
    MAKE_EXPRESSION(True);
    constexpr True() __attribute__((const));
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
};

template<Expression E>
struct Not : MakeExpression {
    MAKE_EXPRESSION(Not);
    constexpr Not(Implies<E, False>) __attribute__((const));
    template<Expression NegE> requires std::same_as<E, Not<NegE>>
    constexpr NegE elim() const __attribute__((const));
};

template<Expression E1, Expression E2>
struct And : MakeExpression {
    using Self = And<E1, E2>;
    MAKE_EXPRESSION(And);
    constexpr And(E1, E2) __attribute__((const));
    constexpr E1 left_elim() const __attribute__((const));
    constexpr E2 right_elim() const __attribute__((const));
};

template<Expression E1, Expression E2>
struct Or : MakeExpression {
    using Self = Or<E1, E2>;
    MAKE_EXPRESSION(Or);
    constexpr Or(E1) __attribute__((const));
    constexpr Or(E2) requires (!std::same_as<E1, E2>) __attribute__((const));
    template<Expression Consequence>
    constexpr Consequence elim(Implies<E1, Consequence>, Implies<E2, Consequence>) const __attribute__((const));
};

template<Expression Consequence, Expression... Assumptions>
using deriv = std::function<Consequence(Assumptions...)>;

template<Expression Assumption, Expression Consequence>
struct Implies : MakeExpression {
    using Self = Implies<Assumption, Consequence>;
    MAKE_EXPRESSION(Implies);
    template<Expression... Context>
    constexpr Implies(deriv<Consequence, Assumption, Context...>, Context...) __attribute__((const));
    constexpr Consequence elim(Assumption) const __attribute__((const));
};

template<Expression E1, Expression E2>
using Equiv = And<Implies<E1, E2>, Implies<E2, E1>>;


template<typename T, fv_tag_t x, Expression E>
struct ForAll : MakeExpression {
    using Self = ForAll<T, x, E>;
    MAKE_EXPRESSION(ForAll);
    template<fv_tag_t y> requires (x != y)
    constexpr ForAll(ForAll<T, y, Subst<E, T, x, FV<T, y>>>);
    template<Expression... Context> requires(!FreeIn<FV<T, x>, Context> && ...)
    constexpr ForAll(deriv<E, Context...>, Context...) __attribute__ ((const));
    template<Variable<T> X>
    constexpr Subst<E, T, x, X> elim() const __attribute__ ((const));
    template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
    constexpr ForAll<T, y, Subst<E, T, x, FV<T, y>>> rename() const __attribute__ ((const));
};

template<typename T, fv_tag_t x, Expression E>
struct Exists : MakeExpression {
    using Self = Exists<T, x, E>;
    MAKE_EXPRESSION(Exists);
    template<Variable<T> X>
    constexpr Exists(Subst<E, T, x, X>, X) __attribute__ ((const));
    template<Expression Consequence, Expression... Context>
        requires(!FreeIn<FV<T, x>, Consequence> && (!FreeIn<FV<T, x>, Context> && ...))
    constexpr Consequence elim(deriv<Consequence, E, Context...>, Context...) const __attribute__((const));
    template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
    constexpr Exists<T, y, Subst<E, T, x, FV<T, y>>> rename() const __attribute__((const));
};

template<GenericVar X, GenericVar Y>
struct Equals : MakeExpression {
    using Self = Equals<X, Y>;
    MAKE_EXPRESSION(Equals);
    Equals() = delete;
};

template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
struct Equals<X, Y> : MakeExpression {
    using Self = Equals<X, Y>;
    MAKE_EXPRESSION(Equals);
    Equals() requires std::same_as<X, Y> __attribute__ ((const));
    Equals(const Equals<Y, X>&) __attribute__ ((const));
    template<Variable<int> Z>
    Equals(Equals<X, Z>, Equals<Z, Y>) __attribute__ ((const));
};

constexpr True::True() {};

constexpr MakeExpression::MakeExpression(False) {}
template<Expression E>
constexpr E MakeExpression::produce() const {
    return E();
}

template<Expression E1, Expression E2>
template<Expression Consequence>
constexpr Consequence Or<E1, E2>::elim(Implies<E1, Consequence>, Implies<E2, Consequence>) const {
    return produce<Consequence>(); 
}
// todo: add further definitions to allow compilation and linking without optimization
// (and importantly to make that process be a reliable check,
// as long as reinterpret casts are forbidden)

// peano arithmetic:

// signature:
// constant
struct Zero : MakeVariable<int> { MAKE_VARIABLE(int); };

// functions
template<Variable<int> X>
struct Succ : MakeVariable<int> { MAKE_VARIABLE(int); };

template<Variable<int> X, Variable<int> Y>
struct Plus : MakeVariable<int> { MAKE_VARIABLE(int); };

template<Variable<int> X, Variable<int> Y>
struct Times : MakeVariable<int> { MAKE_VARIABLE(int); };
// mapping to primitive type
template<int _x> requires (_x >= 0)
struct N : MakeVariable<int> {
    MAKE_VARIABLE(int);
    static constexpr int x = _x; 
};

// predicates

// axioms

struct Axioms {
    template<Expression E1, Expression E2, Expression... Equalities>
        requires(std::same_as<EqSubst<E1, Equalities...>, EqSubst<E2, Equalities...>>)
    static constexpr E1 from_subst(E2, Equalities...);

    using x = FV<int, 'x'>;
    using y = FV<int, 'y'>;
    static const inline Equals<N<0>, Zero>
        primitive_mapping_base;

    template<int _x>
    static const inline Equals<Succ<N<_x>>, N<_x + 1>>
        primitive_mapping_succ;

    template<int _x, int _y>
    static const inline Equals<Plus<N<_x>, N<_y>>, N<_x + _y>>
        primitive_mapping_plus;

    template<int _x, int _y>
    static const inline Equals<Times<N<_x>, N<_y>>, N<_x * _y>>
        primitive_mapping_times;

    static const inline Not<Exists<int, 'x', Equals<Succ<x>, Zero>>>
        zero_is_no_successor;

    static const inline 
        ForAll<int, 'x', ForAll<int, 'y', 
            Implies<
                Equals<Succ<x>, Succ<y>>,
                Equals<x, y>
            >
        >>
        successor_injective;
    static const inline
        ForAll<int, 'x',
            Equals<
                Plus<x, Zero>, 
                x
            >
        >
        addition_base;

    static const inline
        ForAll<int, 'x', ForAll<int, 'y',
            Equals<
                Plus<x, Succ<y>>,
                Succ<Plus<x, y>>
            >
        >>
        addition_recursion;
    
    static const inline
        ForAll<int, 'x',
            Equals<
                Times<x, Zero>, 
                Zero
            >
        >
        multiplication_base;
    static const inline
        ForAll<int, 'x', ForAll<int, 'y',
            Equals<
                Times<x, Succ<y>>,
                Plus<
                    Times<x, y>,
                    x
                >
            >
        >>
        multiplication_recursion;

    template<fv_tag_t x, Expression E>
        requires(FreeIn<FV<int, x>, E>)
    static const inline
        Implies<
            And<
                Subst<E, int, x, Zero>,
                ForAll<int, x,
                    Implies<E, Subst<E, int, x, Succ<FV<int, x>>>>
                >
            >,
            ForAll<int, 'x', E>
        >
        induction;

};

#ifndef NOT_NAMESPACED
} // namespace logic
#endif