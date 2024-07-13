#pragma once

#include "../logic.hpp"

using namespace logic;

using test1 = And<
    Equals<FV<int, 'y'>, Zero>,
    ForAll<int, 'x',
        Or<
            Equals<FV<int, 'y'>, FV<int, 'x'>>,
            Exists<int, 'y',
                And<
                    Equals<Succ<FV<int, 'y'>>, FV<int, 'x'>>,
                    Equals<FV<int, 'z'>, FV<int, 'z'>>
                >
            >
        >
    >
>;