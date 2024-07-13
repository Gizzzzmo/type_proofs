#pragma once

#include <concepts>
#include <functional>
#include <sys/cdefs.h>
#include <type_traits>

namespace logic {

} // namespace logic
struct MakeExpression;

template<typename T>
concept Expression = std::is_base_of_v<MakeExpression, T> && sizeof(T) == 1;

struct True;
struct False;

template<Expression E>
struct Not;

template<Expression Assumption, Expression Consequence>
struct Implies;

template<Expression E1, Expression E2>
struct And;

template<Expression E1, Expression E2>
struct Or;

struct MakeExpression {
    constexpr MakeExpression(False) __attribute__((const));
    protected:
        constexpr MakeExpression() {};
        template<Expression E>
        constexpr E produce() const __attribute__((const));
};

struct Axioms;

#define MAKE_EXPRESSION()\
using MakeExpression::MakeExpression;\
friend class Axioms;\
friend class MakeExpression


struct True : MakeExpression {
    MAKE_EXPRESSION();
    constexpr True() __attribute__((const));
};

struct False : MakeExpression {
    MAKE_EXPRESSION();
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
    MAKE_EXPRESSION();
    constexpr Not(Implies<E, False>) __attribute__((const));
    template<Expression NegE> requires std::same_as<E, Not<NegE>>
    constexpr NegE elim() const __attribute__((const));
};

template<Expression E1, Expression E2>
struct And : MakeExpression {
    MAKE_EXPRESSION();
    constexpr And(E1, E2) __attribute__((const));
    constexpr E1 left_elim() const __attribute__((const));
    constexpr E2 right_elim() const __attribute__((const));
};

template<Expression E1, Expression E2>
struct Or : MakeExpression {
    MAKE_EXPRESSION();
    constexpr Or(E1) __attribute__((const));
    constexpr Or(E2) requires (!std::same_as<E1, E2>) __attribute__((const));
    template<Expression Consequence>
    constexpr Consequence elim(Implies<E1, Consequence>, Implies<E2, Consequence>) const __attribute__((const));
};

template<Expression Consequence, Expression... Assumptions>
using deriv = std::function<Consequence(Assumptions...)>;

template<Expression Assumption, Expression Consequence>
struct Implies : MakeExpression {
    MAKE_EXPRESSION();
    template<Expression... Context>
    constexpr Implies(deriv<Consequence, Assumption, Context...>, Context...) __attribute__((const));
    constexpr Consequence elim(Assumption) const __attribute__((const));
};

template<Expression E1, Expression E2>
using Equiv = And<Implies<E1, E2>, Implies<E2, E1>>;

struct GenericVariableMarker {};

template<typename T>
struct MakeVariable : GenericVariableMarker {};

template<typename V>
concept GenericVar = std::is_base_of_v<GenericVariableMarker, V> && requires {
    typename V::type;
};

#define MAKE_VARIABLE(T) using type = T

template<typename T, char _x>
struct FV : MakeVariable<T> {
    MAKE_VARIABLE(T);
    static constexpr char x = _x;
};


template<typename V, typename T>
concept Variable = std::is_base_of_v<MakeVariable<T>, V> && std::same_as<typename V::type, T>;

template<typename T>
concept ExpOrVar = GenericVar<T> || Expression<T>;

template<typename T, char x, Expression E>
struct ForAll;

template<typename T, char x, Expression E>
struct Exists;

template<GenericVar, ExpOrVar>
struct FreeInImpl {
    static constexpr bool value = false;
};

template<typename T, char x>
struct FreeInImpl<FV<T, x>, FV<T, x>> {
    static constexpr bool value = true;
};

template<typename T, char x, template<typename...> class Comb, ExpOrVar... Es>
    requires(Expression<Comb<Es...>>)
struct FreeInImpl<FV<T, x>, Comb<Es...>> {
    static constexpr bool value = (FreeInImpl<FV<T, x>, Es>::value || ...);
};

template<typename T, char x, template<typename...> class Func, GenericVar... Vs>
    requires(GenericVar<Func<Vs...>>)
struct FreeInImpl<FV<T, x>, Func<Vs...>> {
    static constexpr bool value = (FreeInImpl<FV<T, x>, Vs>::value || ...);
};

template<typename T, char x, template<typename, char, typename> class Quant, typename TT, char xx, Expression E>
    requires((!std::same_as<T, TT> || x != xx) && Expression<Quant<TT, xx, E>>)
struct FreeInImpl<FV<T, x>, Quant<TT, xx, E>> {
    static constexpr bool value = FreeInImpl<FV<T, x>, E>::value;
};

template<GenericVar V, ExpOrVar E>
inline constexpr bool FreeIn = FreeInImpl<V, E>::value;

template<GenericVar, ExpOrVar>
struct BoundInImpl {
    static constexpr bool value = false;
};

template<typename T, char x, template<typename, char, typename> class Quant, Expression E>
    requires(Expression<Quant<T, x, E>>)
struct BoundInImpl<FV<T, x>, Quant<T, x, E>> {
    static constexpr bool value = FreeIn<FV<T, x>, E>;
};

template<typename T, char x, template<typename...> class Comb, ExpOrVar... Es>
    requires(Expression<Comb<Es...>>)
struct BoundInImpl<FV<T, x>, Comb<Es...>> {
    static constexpr bool value = (BoundInImpl<FV<T, x>, Es>::value || ...);
};

template<typename T, char x, template<typename...> class Func, GenericVar... Vs>
    requires(GenericVar<Func<Vs...>>)
struct BoundInImpl<FV<T, x>, Func<Vs...>> {
    static constexpr bool value = (BoundInImpl<FV<T, x>, Vs>::value || ...);
};

template<typename T, char x, template<typename, char, typename> class Quant, typename TT, char xx, Expression E>
    requires((!std::same_as<T, TT> || x != xx) && Expression<Quant<TT, xx, E>>)
struct BoundInImpl<FV<T, x>, Quant<TT, xx, E>> {
    static constexpr bool value = BoundInImpl<FV<T, x>, E>::value;
};

template<GenericVar V,  ExpOrVar E>
inline constexpr bool BoundIn = BoundInImpl<V, E>::value;

template<typename T, char x, ExpOrVar E, Variable<T> X>
    requires(!BoundIn<X, E>)
struct SubstImpl {
    using type = E;
};

template<typename T, char x, Variable<T> X>
struct SubstImpl<T, x, FV<T, x>, X> {
    using type = X;
};

template<typename T, char x, template<typename...> class Comb, ExpOrVar... Es, Variable<T> X>
    requires(Expression<Comb<Es...>>)
struct SubstImpl<T, x, Comb<Es...>, X> {
    using type = Comb<typename SubstImpl<T, x, Es, X>::type...>;
};

template<typename T, char x, template<typename...> class Func, GenericVar... Vs, Variable<T> X>
    requires(GenericVar<Func<Vs...>>)
struct SubstImpl<T, x, Func<Vs...>, X> {
    using type = Func<typename SubstImpl<T, x, Vs, X>::type...>;
};

template<typename T, char x, typename TT, char xx, Expression E, Variable<T> X>
    requires((!std::same_as<T, TT> || x != xx) && !std::same_as<X, FV<TT, xx>>)
struct SubstImpl<T, x, ForAll<TT, xx, E>, X> {
    using type = ForAll<TT, xx, typename SubstImpl<T, x, E, X>::type>;
};

template<typename T, char x, typename TT, char xx, Expression E, Variable<T> X>
    requires((!std::same_as<T, TT> || x != xx)  && !std::same_as<X, FV<TT, xx>>)
struct SubstImpl<T, x, Exists<TT, xx, E>, X> {
    using type = Exists<TT, xx, typename SubstImpl<T, x, E, X>::type>;
};

template<Expression E, typename T, char x, Variable<T> X>
    requires(!BoundIn<X, E>)
using Subst = SubstImpl<T, x, E, X>::type;

template<typename T, char x, Expression E>
struct ForAll : MakeExpression {
    MAKE_EXPRESSION();
    template<char y> requires (x != y)
    constexpr ForAll(ForAll<T, y, Subst<E, T, x, FV<T, y>>>);
    template<Expression... Context> requires(!FreeIn<FV<T, x>, Context> && ...)
    constexpr ForAll(deriv<E, Context...>, Context...) __attribute__ ((const));
    template<Variable<T> X>
    constexpr Subst<E, T, x, X> elim() const __attribute__ ((const));
    template<char y>
    constexpr ForAll<T, y, Subst<E, T, x, FV<T, y>>> rename() const __attribute__ ((const));
};

template<typename T, char x, Expression E>
struct Exists : MakeExpression {
    MAKE_EXPRESSION();
    template<Variable<T> X>
    constexpr Exists(Subst<E, T, x, X>, X) __attribute__ ((const));
    template<Expression Consequence, Expression... Context>
        requires(!FreeIn<FV<T, x>, Consequence> && (!FreeIn<FV<T, x>, Context> && ...))
    constexpr Consequence elim(deriv<Consequence, E, Context...>, Context...) const __attribute__((const));
    template<char y>
    constexpr Exists<T, y, Subst<E, T, x, FV<T, y>>> rename() const __attribute__((const));
};

template<GenericVar X, GenericVar Y>
struct Equals : MakeExpression {
    MAKE_EXPRESSION();
    Equals() = delete;
};

template<GenericVar X, GenericVar Y> requires(std::same_as<typename X::type, typename Y::type>)
struct Equals<X, Y> : MakeExpression {
    MAKE_EXPRESSION();
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

    template<char x, Expression E>
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
