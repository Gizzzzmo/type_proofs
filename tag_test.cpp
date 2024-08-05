#define NOT_NAMESPACED

#include "logic/logic.hpp"
#include "logic/tagging.hpp"
#include "logic/int.hpp"

using _x = FV<int, 'x'>;

template<Variable<int> V1, Variable<int> V2>
auto no_successor_is_zero(Equals<V1, Succ<V2>> v1_succ_of_v2) {
    deriv<False, Equals<V1, Zero>, Equals<V1, Succ<V2>>> proof =
    [](Equals<V1, Zero> z_zero, Equals<Succ<V2>, V1> z_is_succ_of_y){
        Equals<Succ<V2>, Zero> zero_is_succ_of_y(z_is_succ_of_y, z_zero);
        Exists<int, 'x', Equals<Succ<_x>, Zero>> zero_is_succesor(zero_is_succ_of_y, V2());
        False f(Axioms::zero_is_no_successor, zero_is_succesor);

        return f;
    };

    Implies<Equals<V1, Zero>, False> contr(proof, v1_succ_of_v2);
    Not<Equals<V1, Zero>> req(contr);

    return req;
}

template<Variable<int> V, Variable<int> X, int n> requires(n > 0)
auto x_plus_positive_is_not_zero(Equals<V, Plus<X, N<n>>> v_is_x_plus_positive) {
    Equals<N<n>, Succ<N<n-1>>> p_eq(Axioms::primitive_succ<n-1>);
    auto x_plus_succ_of_n_minus_1_is_succ_of_x_plus_n_minus_1 =
        Axioms::addition_recursion.elim<X>().template elim<N<n-1>>();

    auto v_is_succ_of_x_plus_n_minus_1 =
        Axioms::from_subst<
            Equals<V, Succ<
                Plus<X, N<n-1>>
            >>
        >
    (
        v_is_x_plus_positive, p_eq,
        x_plus_succ_of_n_minus_1_is_succ_of_x_plus_n_minus_1
    );

    return no_successor_is_zero(v_is_succ_of_x_plus_n_minus_1);
}



using _y = FV<int, '\x00'>;
auto _testt(Context<_x> c0, Int<_x> x, Not<Equals<_x, Zero>> x_not_zero) {

    auto pre_y = x + Int<N<1>>();
    auto [c1, y, eq] = create_tagged_split(std::move(c0), pre_y);

    return PackagedContext<Context<_x, _y>, TypeList<_y>, decltype(eq)>(std::move(c1), eq, y);
}

using out = PackagedContext<Context<_x, _y>, TypeList<_y>, Equals<_y, Plus<_x, N<1>>>>; 
constexpr FnDispatcher<Context<_x>, TypeList<Not<Equals<_x, Zero>>>, out, decltype(_testt)*>
    testt(_testt);


using _z = FV<int, 1>;
void blub(Context<> c0) {
    auto [c1, y] = create_tagged<int>(std::move(c0), 0);

    auto pre_z = y + Int<N<1>>();

    auto [c2, z, eq] = create_tagged_split(std::move(c1), pre_z);

    Not<Equals<_z, Zero>> z_is_not_zero = x_plus_positive_is_not_zero(eq);

    testt(std::move(c2), z_is_not_zero, z);

}


