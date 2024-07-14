#include "../../logic/logic.hpp"
#include "../test_types.hpp"

using test1_subst_x_for_y = Subst<test1, int, 'y', Succ<FV<int, 'x'>>>;