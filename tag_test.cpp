#define NOT_NAMESPACED

#include "logic/logic.hpp"
#include "logic/tagging.hpp"
#include "logic/int.hpp"


using _x = FV<int, 'x'>;
using _y = FV<int, '\x00'>;
auto testt(Context<_x> c0, Int<_x> x, Not<Equals<_x, Zero>> x_not_zero) {

    auto pre_y = x + Int<N<1>>();
    auto [c1, y, eq] = create_tagged_split(std::move(c0), pre_y);

    return PackagedContext<Context<_x, _y>, TypeList<_y>, decltype(eq)>(std::move(c1), eq, y);
}

using out = PackagedContext<Context<_x, _y>, TypeList<_y>, Equals<_y, Plus<_x, N<1>>>>; 
FnDispatcher<Context<_x>, TypeList<Not<Equals<_x, Zero>>>, out> run_testt(testt);


using _z = FV<int, 1>;
void blub(Context<> c0) {
    auto [c1, z] = create_tagged<int>(std::move(c0), 2);

    auto z1 = z + Int<N<1>>();

    auto [c2, z2, eq] = create_tagged_split(std::move(c1), z1);

    // proof that _z (tag of z2) is not zero
    Equals<N<1>, Succ<N<0>>> p_eq(Axioms::primitive_succ<0>);
    auto blub = Axioms::addition_recursion.elim<_y>().elim<Zero>();
    auto foo = Axioms::addition_base.elim<_y>();
    auto z_is_succ_of_y =
        Axioms::from_subst<Equals<_z, Succ<_y>>>(eq, p_eq, Axioms::primitive_base, blub, foo);

    deriv<False, Equals<_z, Zero>, Equals<_z, Succ<_y>>> proof =
    [](Equals<_z, Zero> z_zero, Equals<Succ<_y>, _z> z_is_succ_of_y){
        Equals<Succ<_y>, Zero> zero_is_succ_of_y(z_is_succ_of_y, z_zero);
        Exists<int, 'x', Equals<Succ<_x>, Zero>> zero_is_succesor(zero_is_succ_of_y, _y());
        False f(Axioms::zero_is_no_successor, zero_is_succesor);

        return f;
    };

    Implies<Equals<_z, Zero>, False> contr(proof, z_is_succ_of_y);
    Not<Equals<_z, Zero>> req(contr);
    
    // end proof that _z (tag of z2) is not zero

    run_testt(std::move(c2), req, z2);

}


