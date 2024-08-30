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

using x = FV<peano_int, 'x'>;
using ZeroP = Zero<peano_int>;
using One = Succ<ZeroP>;

False lambda_capture_1() {
    deriv<Implies<Equals<x, ZeroP>, False>> bad_proof =
    []() {
        deriv<False, Equals<x, ZeroP>> false_from_x_is_zero =
        [](Equals<x, ZeroP> x_is_zero) {
            deriv<Equals<x, ZeroP>> problematic_derivation =
            [x_is_zero]() {
                return x_is_zero;
            };
            ForAll<peano_int, 'x', Equals<x, ZeroP>> bad_universal_quantification(problematic_derivation);

            Equals<One, ZeroP> one_is_zero = bad_universal_quantification.elim<One>();
            Exists<peano_int, 'x', Equals<Succ<x>, ZeroP>> zero_is_successor(one_is_zero, ZeroP());
            False f(zero_is_successor, Axioms::zero_is_no_successor<>);
            return f;
        };
        Implies<Equals<x, ZeroP>, False> bad_implication(false_from_x_is_zero);
        return bad_implication;
    };

    using Quantified = ForAll<peano_int, 'x', Implies<Equals<x, ZeroP>, False>>; 
    Quantified any_x_zero_implies_false(bad_proof);

    Equals<ZeroP, ZeroP> refl;
    auto f = any_x_zero_implies_false.elim<ZeroP>().elim(refl);
    return f;
}

False lambda_capture_2() {
    deriv<Implies<Equals<x, ZeroP>, False>> bad_proof =
    []() {
        deriv<False, Equals<x, ZeroP>> false_from_x_is_zero =
        [](Equals<x, ZeroP> x_is_zero) {
            Equals<One, One> refl;
            Exists<peano_int, 'x', Equals<x, One>> one_exists(refl, One());

            deriv<False, Equals<x, One>> problematic_derivation =
            [x_is_zero](Equals<x, One> x_is_one) {
                Equals<One, x> one_is_x(x_is_one);
                Equals<One, ZeroP> one_is_zero(one_is_x, x_is_zero);
                Exists<peano_int, 'x', Equals<Succ<x>, ZeroP>> zero_is_successor(one_is_zero, ZeroP());
                False f(zero_is_successor, Axioms::zero_is_no_successor<>);
                return f;
            };
            
            False f = one_exists.elim(problematic_derivation);
            return f;
        };
        Implies<Equals<x, ZeroP>, False> bad_implication(false_from_x_is_zero);
        return bad_implication;
    };

    using Quantified = ForAll<peano_int, 'x', Implies<Equals<x, ZeroP>, False>>; 
    Quantified any_x_zero_implies_false(bad_proof);

    Equals<ZeroP, ZeroP> refl;
    auto f = any_x_zero_implies_false.elim<ZeroP>().elim(refl);
    return f;
}

