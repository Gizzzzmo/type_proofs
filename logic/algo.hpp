#pragma once

#include "empty_types.hpp"

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif

template<GenericVar, ExpOrVar>
struct FreeInImpl {
    static constexpr bool value = false;
};

template<typename T, fv_tag_t x>
struct FreeInImpl<FV<T, x>, FV<T, x>> {
    static constexpr bool value = true;
};

template<typename T, fv_tag_t x, template<typename...> class Comb, ExpOrVar... Es>
    requires(Expression<Comb<Es...>>)
struct FreeInImpl<FV<T, x>, Comb<Es...>> {
    static constexpr bool value = (FreeInImpl<FV<T, x>, Es>::value || ...);
};

template<typename T, fv_tag_t x, template<typename...> class Func, GenericVar... Vs>
    requires(GenericVar<Func<Vs...>>)
struct FreeInImpl<FV<T, x>, Func<Vs...>> {
    static constexpr bool value = (FreeInImpl<FV<T, x>, Vs>::value || ...);
};

template<typename T, fv_tag_t x, template<typename, fv_tag_t, typename> class Quant, typename TT, fv_tag_t xx, Expression E>
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

template<typename T, fv_tag_t x, template<typename, fv_tag_t, typename> class Quant, Expression E>
    requires(Expression<Quant<T, x, E>>)
struct BoundInImpl<FV<T, x>, Quant<T, x, E>> {
    static constexpr bool value = FreeIn<FV<T, x>, E>;
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

template<typename T, fv_tag_t x, template<typename, fv_tag_t, typename> class Quant, typename TT, fv_tag_t xx, Expression E>
    requires((!std::same_as<T, TT> || x != xx) && Expression<Quant<TT, xx, E>>)
struct BoundInImpl<FV<T, x>, Quant<TT, xx, E>> {
    static constexpr bool value = BoundInImpl<FV<T, x>, E>::value;
};

template<GenericVar V,  ExpOrVar E>
inline constexpr bool BoundIn = BoundInImpl<V, E>::value;



template<typename T, fv_tag_t x, ExpOrVar E, Variable<T> X>
    requires(!BoundIn<X, E>)
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
    requires(!BoundIn<X, E>)
using Subst = SubstImpl<T, x, E, X>::type;


#ifndef NOT_NAMESPACED
} //namespace logic
#endif
