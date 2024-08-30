#include "../logic/logic.hpp"
#include "test_types.hpp"


using test1_exp_subst_zero_for_y = And<
    Equals<Zero<int>, Zero<int>>,
    ForAll<int, 'x',
        Or<
            Equals<Zero<int>, FV<int, 'x'>>,
            Exists<int, 'y',
                And<
                    Equals<Succ<FV<int, 'y'>>, FV<int, 'x'>>,
                    Equals<FV<int, 'z'>, FV<int, 'z'>>
                >
            >
        >
    >
>;

using test1_subst_zero_for_y = Subst<test1, int, 'y', Zero<int>>;

static_assert(std::is_same_v<test1_subst_zero_for_y, test1_exp_subst_zero_for_y>);