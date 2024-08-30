#include "../algo.hpp"
#include "../empty_types.hpp"
#include "../logic.hpp"
#include "../tagging.hpp"

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
        template<typename, typename, typename, typename>
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
            return Bool<And<E, E2>>(x && static_cast<bool>(other));
        }

        template<Expression E2>
        constexpr Bool<Or<E, E2>> operator||(Bool<E2> other) {
            return Bool<Or<E, E2>>(x && static_cast<bool>(other));
        }

    private:
        bool x;
};

template<>
class Bool<True> {
    private:
        constexpr Bool(bool) {}
        template<Expression>
        friend class Bool;
        template<typename, typename, typename, typename>
        friend class FnDispatcher;
    public:
        constexpr Bool(True) {}
        constexpr operator bool() {
            return true;
        }
        constexpr Bool<Not<True>> operator!() {
            return Bool<Not<True>>(false);
        }

        template<Expression E>
        constexpr Bool<And<True, E>> operator&&(Bool<E> other) {
            return Bool<And<True, E>>(static_cast<bool>(other));
        }

        template<Expression E>
        constexpr Bool<Or<True, E>> operator||(Bool<E> other) {
            return Bool<Or<True, E>>(true);
        }
};

template<>
class Bool<False> {
    private:
        constexpr Bool(bool) {}
        template<Expression>
        friend class Bool;
        template<typename, typename, typename, typename>
        friend class FnDispatcher;
    public:
        constexpr Bool(False) {}
        constexpr operator bool() {
            return false;
        }
        constexpr Bool<Not<False>> operator!() {
            return Bool<Not<False>>(true);
        }

        template<Expression E>
        constexpr Bool<And<False, E>> operator&&(Bool<E> other) {
            return Bool<And<False, E>>(false);
        }

        template<Expression E>
        constexpr Bool<Or<False, E>> operator||(Bool<E> other) {
            return Bool<Or<False, E>>(static_cast<bool>(other));
        }
};


#ifndef NOT_NAMESPACED
}
#endif