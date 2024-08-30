#define NOT_NAMESPACED

#include "logic/logic.hpp"

#include "logic/tagged/int.hpp"

using _x = FV<peano_int, 'x'>;
using ZeroP = Zero<peano_int>;

template<Variable<peano_int> V1, Variable<peano_int> V2>
auto no_successor_is_zero(Equals<V1, Succ<V2>> v1_succ_of_v2) -> Not<Equals<V1, ZeroP>>{
    deriv<False, Equals<V1, ZeroP>, Equals<V1, Succ<V2>>> proof =
    [](Equals<V1, ZeroP> z_zero, Equals<Succ<V2>, V1> z_is_succ_of_y){
        Equals<Succ<V2>, ZeroP> zero_is_succ_of_y(z_is_succ_of_y, z_zero);
        Exists<peano_int, 'x', Equals<Succ<_x>, ZeroP>> zero_is_succesor(zero_is_succ_of_y, V2());
        False f(Axioms::zero_is_no_successor<>, zero_is_succesor);

        return f;
    };

    Implies<Equals<V1, ZeroP>, False> contr(proof, v1_succ_of_v2);
    Not<Equals<V1, ZeroP>> req(contr);

    return req;
}

template<Variable<peano_int> V, Variable<peano_int> X, auto n> requires(n > 0)
auto x_plus_positive_is_not_zero(Equals<V, Plus<X, N<n, peano_int>>> v_is_x_plus_positive) -> Not<Equals<V, ZeroP>> {
    Equals<N<n, peano_int>, Succ<N<n-1, peano_int>>> p_eq(Axioms::primitive_succ<n-1, peano_int>);
    auto x_plus_succ_of_n_minus_1_is_succ_of_x_plus_n_minus_1 =
        Axioms::addition_recursion<peano_int, 0, 1>.elim<X>().template elim<N<n-1, peano_int>>();

    auto v_is_succ_of_x_plus_n_minus_1 =
        Axioms::from_subst<
            Equals<V, Succ<
                Plus<X, N<n-1, peano_int>>
            >>
        >
    (
        v_is_x_plus_positive, p_eq,
        x_plus_succ_of_n_minus_1_is_succ_of_x_plus_n_minus_1
    );

    return no_successor_is_zero(v_is_succ_of_x_plus_n_minus_1);
}



using _y = FV<peano_int, 'y'>;
auto _testt(Context<_x> c0, Peano<_x> x, Not<Equals<_x, ZeroP>> x_not_zero) {
    auto test = Peano<N<1, peano_int>>();
    auto pre_y = x + Peano<N<1, peano_int>>();
    auto [c1, y, eq] = create_tagged_split<'y'>(std::move(c0), pre_y);

    return PackagedContext<Context<_x, _y>, TypeList<_y>, decltype(eq)>(std::move(c1), eq, y);
}

using out = PackagedContext<Context<_x, _y>, TypeList<_y>, Equals<_y, Plus<_x, N<1, peano_int>>>>; 
constexpr FnDispatcher<Context<_x>, TypeList<Not<Equals<_x, ZeroP>>>, out, decltype(_testt)*>
    testt(_testt);


using _z = FV<peano_int, 'z'>;
void blub(Context<> c0) {
    auto [c1, y] = create_tagged<'y'>(std::move(c0), peano_int(0));

    auto pre_z = y + Peano<N<1, peano_int>>();

    auto [c2, z, eq] = create_tagged_split<'z'>(std::move(c1), pre_z);

    Not<Equals<_z, ZeroP>> z_is_not_zero = x_plus_positive_is_not_zero(eq);

    testt(std::move(c2), z_is_not_zero, z);

}


