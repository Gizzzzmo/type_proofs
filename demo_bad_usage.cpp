#define NOT_NAMESPACED // helps make clangd deduced types more readable
#include "logic/logic.hpp"

using namespace logic;

extern False imported_false;

False using_extern() {
    return imported_false;
}

False illegal_casts() {
    return *reinterpret_cast<False*>(1);
}

using x = FV<int, 'x'>;
using One = Succ<Zero>;
False lambda_capture_1() {
    deriv<Implies<Equals<x, Zero>, False>> bad_proof =
    []() {
        deriv<False, Equals<x, Zero>> false_from_x_is_zero =
        [](Equals<x, Zero> x_is_zero) {
            deriv<Equals<x, Zero>> problematic_derivation =
            [x_is_zero]() {
                return x_is_zero;
            };
            ForAll<int, 'x', Equals<x, Zero>> bad_universal_quantification(problematic_derivation);

            Equals<One, Zero> one_is_zero = bad_universal_quantification.elim<One>();
            Exists<int, 'x', Equals<Succ<x>, Zero>> zero_is_successor(one_is_zero, Zero());
            False f(zero_is_successor, Axioms::zero_is_no_successor);
            return f;
        };
        Implies<Equals<x, Zero>, False> bad_implication(false_from_x_is_zero);
        return bad_implication;
    };

    using Quantified = ForAll<int, 'x', Implies<Equals<x, Zero>, False>>; 
    Quantified any_x_zero_implies_false(bad_proof);

    Equals<Zero, Zero> refl;
    auto f = any_x_zero_implies_false.elim<Zero>().elim(refl);
    return f;
}

False lambda_capture_2() {
    deriv<Implies<Equals<x, Zero>, False>> bad_proof =
    []() {
        deriv<False, Equals<x, Zero>> false_from_x_is_zero =
        [](Equals<x, Zero> x_is_zero) {
            Equals<One, One> refl;
            Exists<int, 'x', Equals<x, One>> one_exists(refl, One());

            deriv<False, Equals<x, One>> problematic_derivation =
            [x_is_zero](Equals<x, One> x_is_one) {
                Equals<One, x> one_is_x(x_is_one);
                Equals<One, Zero> one_is_zero(one_is_x, x_is_zero);
                Exists<int, 'x', Equals<Succ<x>, Zero>> zero_is_successor(one_is_zero, Zero());
                False f(zero_is_successor, Axioms::zero_is_no_successor);
                return f;
            };
            
            False f = one_exists.elim(problematic_derivation);
            return f;
        };
        Implies<Equals<x, Zero>, False> bad_implication(false_from_x_is_zero);
        return bad_implication;
    };

    using Quantified = ForAll<int, 'x', Implies<Equals<x, Zero>, False>>; 
    Quantified any_x_zero_implies_false(bad_proof);

    Equals<Zero, Zero> refl;
    auto f = any_x_zero_implies_false.elim<Zero>().elim(refl);
    return f;
}

