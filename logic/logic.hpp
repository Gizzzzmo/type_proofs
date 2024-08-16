#pragma once

#include <concepts>
#include <functional>
#include <iostream>

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
    template<Variable<int> Z>
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
    return produce<NegE>();
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
    return produce<E1>();
}

template<Expression E1, Expression E2>
constexpr E2 And<E1, E2>::right_elim() const {
    return produce<E2>();
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
    return produce<Consequence>();
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
    return produce<Consequence>();
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
    return produce<Subst<E, T, x, X>>();
}

template<typename T, fv_tag_t x, Expression E>
template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
constexpr ForAll<T, y, Subst<E, T, x, FV<T, y>>> ForAll<T, x, E>::rename() const {
    return produce<ForAll<T, y, Subst<E, T, x, FV<T, y>>>>();
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
    return produce<Consequence>();
}

template<typename T, fv_tag_t x, Expression E>
template<fv_tag_t y> requires(!FreeIn<FV<T, y>, E>)
constexpr Exists<T, y, Subst<E, T, x, FV<T, y>>> Exists<T, x, E>::rename() const {
    return produce<Exists<T, y, Subst<E, T, x, FV<T, y>>>>();
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
template<Variable<int> Z>
constexpr Equals<X, Y>::Equals(Equals<X, Z>, Equals<Z, Y>) {}

template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
constexpr Equals<X, Y>::Equals() {}

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
    static constexpr E1 from_subst(E2, Equalities...) {
        return MakeExpression::produce<E1>();
    }

    using x = FV<int, 'x'>;
    using y = FV<int, 'y'>;
    static const inline Equals<N<0>, Zero>
        primitive_base;

    template<int _x> requires (_x >= 0)
    static const inline Equals<Succ<N<_x>>, N<_x + 1>>
        primitive_succ;

    template<int _x, int _y> requires (_x >= 0 && _y >= 0)
    static const inline Equals<Plus<N<_x>, N<_y>>, N<_x + _y>>
        primitive_plus;

    template<int _x, int _y> requires (_x >= 0 && _y >= 0)
    static const inline Equals<Times<N<_x>, N<_y>>, N<_x * _y>>
        primitive_times;

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