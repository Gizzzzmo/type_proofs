
#define NOT_NAMESPACED
#include "logic/algo.hpp"
#include "logic/logic.hpp"

using namespace logic;

deriv<Not<True>, False> false_implies_not_true = 
[](False f) {
    return Not<True>(f);
};

deriv<False, Not<True>> not_true_implies_false = 
[](Not<True> not_t) {
    True t;
    return False(t, not_t);
};

deriv<Not<False>, True> true_implies_not_false = 
[](True t) {
    std::function<False(False)> contr = [](False f) {
        return f;
    };

    return Not<False>(Implies<False, False>(contr));
};

deriv<True, Not<False>> not_false_implies_true = 
[](Not<False>) {
    return True();
};

And<Equiv<True, Not<False>>, Equiv<False, Not<True>>> true_false_duality() {
    std::function<Not<False>(True)> tinf = true_implies_not_false;
    Implies<True, Not<False>> a(true_implies_not_false);
    Implies<Not<False>, True> b(not_false_implies_true);
    Equiv<True, Not<False>> equiv1(a, b);

    Implies<False, Not<True>> c(false_implies_not_true);
    Implies<Not<True>, False> d(not_true_implies_false);
    Equiv<False, Not<True>> equiv2(c, d);

    return And<Equiv<True, Not<False>>, Equiv<False, Not<True>>>(equiv1, equiv2);
}

template<Expression E1, Expression E2>
E2 modus_ponens(E1 ass, Implies<E1, E2> impl) {
    return impl.elim(ass);
}

template True modus_ponens<True, True>(True, Implies<True, True>);
template True modus_ponens<False, True>(False, Implies<False, True>);
template False modus_ponens<True, False>(True, Implies<True, False>);
template False modus_ponens<False, False>(False, Implies<False, False>);

template<Expression E1, Expression E2>
Not<E1> modus_tollens(Not<E2> not_cons, Implies<E1, E2> impl) {
    std::function<False(E1)> contr = [impl, not_cons](E1 ass){
        E2 cons = impl.elim(ass);
        return False(not_cons, cons);
    };

    return Not<E1>(Implies<E1, False>(contr));
}

template Not<True> modus_tollens<True, True>(Not<True>, Implies<True, True>);
template Not<True> modus_tollens<True, False>(Not<False>, Implies<True, False>);
template Not<False> modus_tollens<False, True>(Not<True>, Implies<False, True>);
template Not<False> modus_tollens<False, False>(Not<False>, Implies<False, False>);

template<Expression E1, Expression E2, Expression E3>
Implies<E1, E3> implication_transitivity(Implies<E1, E2> impl1, Implies<E2, E3> impl2) {
    std::function<E3(E1)> deriv = [impl1, impl2](E1 ass) {
        return impl2.elim(impl1.elim(ass));
    };
    return Implies<E1, E3>(deriv);
}

template Implies<True, False> implication_transitivity(Implies<True, False>, Implies<False, False>);
template Implies<True, False> implication_transitivity(Implies<True, True>, Implies<True, False>);

template<Expression E1, Expression E2>
E1 disjunctive_syllogism(Or<E1, E2> disj, Not<E2> not_e2) {
    
    std::function<E1(E1)> deriv1 = [](E1 e1) {
        return e1;
    };

    Implies<E1, E1> impl1(deriv1);
    
    std::function<E1(E2, Not<E2>)> deriv2 = [](E2 e2, Not<E2> not_e2) {
        False f(e2, not_e2);
        return E1(f);
    };

    Implies<E2, E1> impl2(deriv2, not_e2);

    return disj.elim(impl1, impl2);
}

template True disjunctive_syllogism(Or<True, False>, Not<False>);
template True disjunctive_syllogism(Or<True, True>, Not<True>);

using ZeroP = Zero<peano_int>;
using blub = Subst<
    And<Equals<FV<peano_int, 'y'>, FV<peano_int, 'y'>>, Equals<FV<peano_int, 'x'>, ZeroP>>, 
    peano_int, 'x',
    Succ<ZeroP>
>;

using y = FV<peano_int, 'y'>;
deriv<And<True, Equals<y, y>>> test_proof = 
[]() {
    using y = FV<peano_int, 'y'>;
    Equals<y, y> refl;
    True t;
    And<True, Equals<y, y>> both(t, refl);
    return both;
};


auto test_for_all() {
    ForAll<peano_int, 'y', And<True, Equals<FV<peano_int, 'y'>, FV<peano_int, 'y'>>>> fa(test_proof);

    auto inst = fa.elim<Succ<ZeroP>>();
    return inst;
}


using x = FV<peano_int, 'x'>;
using y = FV<peano_int, 'y'>;

template<Variable<peano_int> X>
using is_successor = Exists<peano_int, 'y', Equals<Succ<y>, X>>;

using to_prove = Or<Equals<x, ZeroP>, is_successor<x>>;

using base_to_prove = Subst<to_prove, peano_int, 'x', ZeroP>;

using induction_hyp = to_prove;
using induction_next = Subst<to_prove, peano_int, 'x', Succ<x>>;
using step_to_prove = Implies<
    induction_hyp,
    induction_next
>;

using step_generalization = ForAll<peano_int, 'x', step_to_prove>;

deriv<ForAll<peano_int, 'x', to_prove>>
test_induction = [] () {
    Equals<ZeroP, ZeroP> refl;
    base_to_prove base(refl);

    deriv<step_to_prove> step_proof = []() {
        deriv<induction_next, induction_hyp> derivation = [](induction_hyp) {
            Equals<Succ<x>, Succ<x>> refl;
            is_successor<Succ<x>> succ_x_is_succ(refl, x());
            return induction_next(succ_x_is_succ);
        };
        return step_to_prove(derivation);
    };

    step_generalization gen(step_proof);

    And<base_to_prove, step_generalization> combine(base, gen);

    return Axioms::induction<to_prove>.elim(combine);
};

using One = N<1, peano_int>;
using Two = N<2, peano_int>;
using Three = N<3, peano_int>;

deriv<
    Implies<Equals<y, Two>, Equals<Plus<x, y>, Three>>,
    Equals<x, One>
> cp =
[](Equals<x, One> exp) {
    deriv<Equals<Plus<x, y>, Three>, Equals<y, Two>, Equals<x, One>> d =
    [](Equals<y, Two> y_is_two, Equals<x, One> x_is_one) {
        Equals<Plus<x, y>, Plus<x, y>> refl;
        auto add = Axioms::primitive_plus<1, 2, peano_int>;
        auto sub = Axioms::from_subst<Equals<Plus<x, y>, Three>>(refl, y_is_two, x_is_one, add);
        return sub;
    };
    Implies<Equals<y, Two>, Equals<Plus<x, y>, Three>> impl(d, exp);
    return impl;
};