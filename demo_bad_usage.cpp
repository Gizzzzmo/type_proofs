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


