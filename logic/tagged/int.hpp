#include "../algo.hpp"
#include "../empty_types.hpp"
#include "../logic.hpp"
#include "../tagging.hpp"

#include "bool.hpp"


namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif

template<Variable<peano_int> X, Variable<peano_int> Y, char z = UnusedFV<peano_int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<peano_int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using GreaterThanEq = Exists<peano_int, z, Equals<Plus<FV<peano_int, z>, Y>, X>>;

template<Variable<peano_int> X, Variable<peano_int> Y, char z = UnusedFV<peano_int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<peano_int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using GreaterThan = GreaterThanEq<X, Succ<Y>, z>;


template<Variable<peano_int> X, Variable<peano_int> Y, char z = UnusedFV<peano_int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<peano_int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using LessThanEq = Not<GreaterThan<X, Y, z>>;

template<Variable<peano_int> X, Variable<peano_int> Y, char z = UnusedFV<peano_int, Concat<FVsIn<X>, FVsIn<Y>>>::tag>
    requires (!IsIn<FV<peano_int, z>, Concat<FVsIn<X>, FVsIn<Y>>>)
using LessThan = Not<GreaterThanEq<X, Y, z>>;



template<Variable<peano_int> X>
class Peano {
    private:
        constexpr Peano(peano_int x) : x(x) {}
        template<typename T, fv_tag_t x, typename... _FVs>
        friend class Tagger;
        template<Variable<peano_int> Y>
        friend class Peano;
        template<typename, typename, typename, typename>
        friend class FnDispatcher;
    public:
        using TagVar = X;
        using Wrapped = peano_int;
        consteval Peano() : x(X::x) {}

        constexpr operator peano_int() {
            return x;
        }

        template<Variable<peano_int> Y>
        constexpr Bool<Equals<X, Y>> operator==(Peano<Y> other) {
            return Bool<Equals<X, Y>>(x == other.x);
        }

        template<Variable<peano_int> Y>
        constexpr Peano<Plus<X, Y>> operator+(Peano<Y> other) {
            return Peano<Plus<X, Y>>(x + other.x);
        }

        template<Variable<peano_int> Y>
        constexpr Bool<GreaterThanEq<X, Y>> operator>(Peano<Y> other) {
            return Bool<GreaterThanEq<X, Y>>(x > other.x);
        }
    private:
        peano_int x;
};


#ifndef NOT_NAMESPACED
}
#endif